/*
    Little Smalltalk, version 2
    Written by Tim Budd, Oregon State University, July 1987

    Improved incorporating suggestions by 
        Steve Crawley, Cambridge University, October 1987
        Steven Pemberton, CWI, Amsterdam, Oct 1987

    memory management module

    This is a rather simple, straightforward, reference counting scheme.
    There are no provisions for detecting cycles, nor any attempt made
    at compaction.  Free lists of various sizes are maintained.
    At present only objects up to 255 bytes can be allocated, 
    which mostly only limits the size of method (in text) you can create.

    reference counts are not stored as part of an object image, but
    are instead recreated when the object is read back in.
    This is accomplished using a mark-sweep algorithm, similar
    to those used in garbage collection.

*/

#include <sstream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "env.h"
#include "memory.h"
#include "interp.h"
#include "names.h"


boolean debugging = false;

extern object firstProcess;
object symbols;     /* table of all symbols created */

static object arrayClass = nilobj;  /* the class Array */
static object intClass = nilobj;    /* the class Integer */
static object stringClass = nilobj; /* the class String */
static object symbolClass = nilobj; /* the class Symbol */


/*
    in theory the objectTable should only be accessible to the memory
    manager.  Indeed, given the right macro definitions, this can be
    made so.  Never the less, for efficiency sake some of the macros
    can also be defined to access the object table directly

    Some systems (e.g. the Macintosh) have static limits (e.g. 32K)
    which prevent the object table from being declared.
    In this case the object table must first be allocated via
    calloc during the initialization of the memory manager.
*/

MemoryManager* MemoryManager::m_pInstance = NULL;

//
// ncopy - copy exactly n bytes from place to place 
// 
void ncopy(register char* p, register char* q, register int n)      
{   
    for (; n>0; n--)
        *p++ = *q++;
}


MemoryManager* MemoryManager::Instance()
{
    if(NULL == m_pInstance)
        m_pInstance = new MemoryManager();

    return m_pInstance;
}



MemoryManager::MemoryManager(size_t initialSize) : noGC(false), growAmount(5000)
{
    objectTable.resize(initialSize);

    /* set all the reference counts to zero */
    for(TObjectTableIterator i = objectTable.begin(), iend = objectTable.end(); i != iend; ++i)
    {
        i->referenceCount = 0; 
        i->size = 0; 
    }

    /* make up the initial free lists */
    setFreeLists();

    /* object at location 0 is the nil object, so give it nonzero ref */
    objectTable[0].referenceCount = 1; objectTable[0].size = 0;
}

MemoryManager::~MemoryManager()
{
}

static long numAllocated;

object MemoryManager::allocObject(size_t memorySize)
{
    int i;
    size_t position;
    boolean done;
    TObjectFreeListIterator tpos;

    /* first try the free lists, this is fastest */
    if((tpos = objectFreeList.find(memorySize)) != objectFreeList.end() &&
            tpos->second != nilobj)
    {
        position = tpos->second;
        objectFreeList.erase(tpos);
        objectFreeListInv.erase(position);
    }

    /* if not there, next try making a size zero object and
        making it bigger */
    else if ((tpos = objectFreeList.find(0)) != objectFreeList.end() &&
            tpos->second != nilobj) 
    {
        position = tpos->second;
        objectFreeList.erase(tpos);
        objectFreeListInv.erase(position);
        objectTable[position].size = memorySize;
        objectTable[position].memory = mBlockAlloc(memorySize);
    }

    else 
    {      /* not found, must work a bit harder */
        done = false;

        /* first try making a bigger object smaller */
        TObjectFreeListIterator tbigger = objectFreeList.upper_bound(memorySize);
        if(tbigger != objectFreeList.end() &&
                tbigger->second != nilobj)
        {
            position = tbigger->second;
            objectFreeList.erase(tbigger);
            objectFreeListInv.erase(position);
            /* just trim it a bit */
            objectTable[position].size = memorySize;
            done = true;
        }

        /* next try making a smaller object bigger */
        if (! done)
        {
            TObjectFreeListIterator tsmaller = objectFreeList.lower_bound(memorySize);
            if(tsmaller != objectFreeList.begin() &&
                    (--tsmaller != objectFreeList.begin()) &&
                     tsmaller->second != nilobj)
            {
                position = tsmaller->second;
                objectFreeList.erase(tsmaller);
                objectFreeListInv.erase(position);
                objectTable[position].size = memorySize;

                free(objectTable[position].memory);

                objectTable[position].memory = mBlockAlloc(memorySize);
                done = true;
            }
        }

        /* if we STILL don't have it then there is nothing */
        /* more we can do */
        if (! done)
        {
            if(debugging)
                fprintf(stderr, "Failed to find an available object, trying GC\n");
            if(garbageCollect() > 0)
            {
                return allocObject(memorySize);
            }
            else
            {
                if(debugging)
                    fprintf(stderr, "No suitable objects available after GC, growing store.\n");
                growObjectStore(growAmount);
                return allocObject(memorySize);
            }
        }
    }

    numAllocated++;
    /* set class and type */
    objectTable[position].referenceCount = 0;
    objectTable[position]._class = nilobj;
    objectTable[position].size = memorySize;
    return(position);
}

object MemoryManager::allocByte(size_t size)
{
    object newObj;

    newObj = allocObject((size + 1) / 2);
    /* negative size fields indicate bit objects */
    objectRef(newObj).size = -size;
    return newObj;
}

object MemoryManager::allocStr(register const char* str)
{
    register object newSym;
    char* t;

    if(NULL != str)
    {
        newSym = allocByte(1 + strlen(str));
        t = objectRef(newSym).charPtr();
        ignore strcpy(t, str);
    }
    else
    {
        newSym = allocByte(1);
        objectRef(newSym).charPtr()[0] = '\0';
    }
    return(newSym);
}

bool MemoryManager::destroyObject(object z)
{
    register struct objectStruct *p;
    register int i;
    int size;
    bool deleted = false;

    p = &objectTable[z];
    if (p->referenceCount < 0) 
    {
        fprintf(stderr,"object %d\n", static_cast<int>(z));
        sysError("negative reference count","");
    }
    size = p->size;
    if (size < 0) size = ((- size) + 1) /2;

    if(objectFreeListInv.find(z) == objectFreeListInv.end())
    {
        objectFreeList.insert(std::pair<size_t, object>(size, z));
        objectFreeListInv.insert(std::pair<object, size_t>(z, size));
        deleted = true;
    }

    if (size > 0) 
    {
        for (i = size; i > 0; )
            p->memory[--i] = nilobj;
    }
    p->size = size;

    return deleted;
}

void MemoryManager::setFreeLists() 
{
    objectFreeList.clear();
    objectFreeListInv.clear();

    /* add free objects */
    for(int z=objectTable.size()-1; z>0; z--)
    {
        // If really unused, destroyObject will take care
        // of cleaning up and adding to the free list.
        if (objectTable[z].referenceCount == 0)
            destroyObject(z);
    }
}

void MemoryManager::visit(register object x)
{
    int i, s;
    object *p;

    if (x) 
    {
        if (--(objectFromID(x).referenceCount) == -1) 
        {
            /* then it's the first time we've visited it, so: */
            visit(objectFromID(x)._class);
            s = objectRef(x).size;
            if (s>0) 
            {
                p = objectFromID(x).memory;
                for (i=s; i; --i) 
                    visit(*p++);
            }
        }
    }
}


int MemoryManager::garbageCollect()
{
    register int j;
    int c=1,f=0;

    if(noGC)
        return 0;

    if (debugging) 
        fprintf(stderr,"\ngarbage collecting ... \n");

    for(TObjectTableIterator x = objectTable.begin(), xend = objectTable.end(); x != xend; ++x)
        x->referenceCount = 0;
    objectTable[0].referenceCount = 1;
    /* visit symbols and firstProcess to toggle their referenceCount */

    visit(symbols);

    /* Visit any explicitly held references from the use of ObjectHandle */
    for(TObjectRefs::iterator i = objectReferences.begin(), iend = objectReferences.end(); i != iend; ++i)
        visit(i->first);

    /* add new garbage to objectFreeList 
    * toggle referenceCount 
    * count the objects
    */

    for (j=objectTable.size()-1; j>0; j--) 
    {
        if (objectFromID(j).referenceCount == 0) 
        {
            if(destroyObject(j))
                f++;
        } 
        else
        {
            if (0!=(objectFromID(j).referenceCount = -objectFromID(j).referenceCount))
                c++;
        }
    }

    if (debugging)
    {
        fprintf(stderr," %d references.\n",objectReferences.size());
        fprintf(stderr," %d objects - %d freed.\n",c,f);
    }

    numAllocated = 0;
    return f;
}

objectStruct& MemoryManager::objectFromID(object id)
{
    return objectTable[id];
}


size_t MemoryManager::objectCount()
{   
    register int j = 0;
    for(TObjectTableIterator i = objectTable.begin(), iend = objectTable.end(); i != iend; ++i)
        if (i->referenceCount > 0)
            j++;
    return j;
}

size_t MemoryManager::freeSlotsCount()
{   
    return objectFreeListInv.size();
}

std::string MemoryManager::statsString()
{
    std::stringstream str;
    str << "Memory Statistics:" << std::endl;
    str << "\tActive Objects     " << objectCount() << std::endl;
    str << "\tObjectstore size   " << objectTable.size() << std::endl;
    str << "\tFree objects       " << objectFreeListInv.size() << std::endl;

    return str.str();
}


ObjectHandle MemoryManager::newArray(int size)
{   
    ObjectHandle newObj(allocObject(size), this);
    if (arrayClass == nilobj)
        arrayClass = globalSymbol("Array");
    newObj->_class = arrayClass;
    return newObj;
}

ObjectHandle MemoryManager::newBlock()
{   
    ObjectHandle newObj(allocObject(blockSize), this);
    newObj->_class = globalSymbol("Block");
    return newObj;
}

ObjectHandle MemoryManager::newByteArray(int size)
{   
    ObjectHandle newobj(allocByte(size), this);
    newobj->_class = globalSymbol("ByteArray");
    return newobj;
}

ObjectHandle MemoryManager::newChar(int value)
{   
    ObjectHandle newobj(allocObject(1), this);
    newobj->basicAtPut(1, newInteger(value));
    newobj->_class = globalSymbol("Char");
    return newobj;
}

ObjectHandle MemoryManager::newClass(const char* name)
{   
    ObjectHandle newObj, nameObj, methTable;

    newObj = ObjectHandle(allocObject(classSize), this);
    newObj->_class = globalSymbol("Class");

    /* now make name */
    nameObj = newSymbol(name);
    newObj->basicAtPut(nameInClass, nameObj); 
    methTable = newDictionary(39);
    newObj->basicAtPut(methodsInClass, methTable);
    newObj->basicAtPut(sizeInClass, newInteger(classSize));

    /* now put in global symbols table */
    nameTableInsert(symbols, strHash(name), nameObj.handle(), newObj.handle());

    return newObj;
}

ObjectHandle MemoryManager::newContext(int link, const ObjectHandle& method, const ObjectHandle& args, const ObjectHandle& temp)
{   
    ObjectHandle newObj(allocObject(contextSize), this);
    newObj->_class = globalSymbol("Context");
    newObj->basicAtPut(linkPtrInContext, newInteger(link));
    newObj->basicAtPut(methodInContext, method);
    newObj->basicAtPut(argumentsInContext, args);
    newObj->basicAtPut(temporariesInContext, temp);
    return newObj;
}

ObjectHandle MemoryManager::newDictionary(int size)
{   
    ObjectHandle newObj(allocObject(1), this);
    newObj->_class = globalSymbol("Dictionary");
    newObj->basicAtPut(1, newArray(size));
    return newObj;
}

ObjectHandle MemoryManager::newFloat(double d)
{   
    ObjectHandle newObj(allocByte((int) sizeof (double)), this);
    ncopy(newObj->charPtr(), (char *) &d, (int) sizeof (double));
    newObj->_class = globalSymbol("Float");
    return newObj;
}

ObjectHandle MemoryManager::newInteger(int i)
{   
    ObjectHandle newObj(allocByte((int) sizeof (int)), this);
    ncopy(newObj->charPtr(), (char *) &i, (int) sizeof (int));
    newObj->_class = globalSymbol("Integer");
    return newObj;
}

object MemoryManager::newCPointer(void* l)
{   
  object newObj;

  int s = sizeof(void*);
    newObj = allocByte((int) sizeof (void*));
    ncopy(objectRef(newObj).charPtr(), (char *) &l, (int) sizeof (void*));
    objectRef(newObj)._class = globalSymbol("CPointer");
    return newObj;
}

ObjectHandle MemoryManager::newLink(const ObjectHandle& key, const ObjectHandle& value)
{   
    ObjectHandle newObj(allocObject(3), this);
    newObj->_class = globalSymbol("Link");
    newObj->basicAtPut(1, key);
    newObj->basicAtPut(2, value);
    return newObj;
}

object MemoryManager::newMethod()
{   
  object newObj;
  
  newObj = allocObject(methodSize);
  objectRef(newObj)._class = globalSymbol("Method");

  return newObj;
}

ObjectHandle MemoryManager::newStString(const char* value)
{   
  ObjectHandle newObj(allocStr(value), this);
  if (stringClass == nilobj)
    stringClass = globalSymbol("String");
  newObj->_class = stringClass;
  return newObj;
}

ObjectHandle MemoryManager::newSymbol(const char* str)
{    
    /* first see if it is already there */
    ObjectHandle newObj(globalKey(str));
    if (newObj.handle()) 
        return newObj;

    /* not found, must make */
    newObj = ObjectHandle(allocStr(str), this);
    if (symbolClass == nilobj)
        symbolClass = globalSymbol("Symbol");
    newObj->_class = symbolClass;
    nameTableInsert(symbols, strHash(str), newObj.handle(), nilobj);
    return newObj;
}


object MemoryManager::copyFrom(object obj, int start, int size)
{   
    object newObj;
    int i;

    newObj = newArray(size).handle();
    for (i = 1; i <= size; i++) 
    {
        objectRef(newObj).basicAtPut(i, objectRef(obj).basicAt(start));
        start++;
    }
    return newObj;
}


static struct 
{
    int di;
    object cl;
    short ds;
} dummyObject;

/*
   imageRead - read in an object image
   we toss out the free lists built initially,
   reconstruct the linkages, then rebuild the free
   lists around the new objects.
   The only objects with nonzero reference counts
   will be those reachable from either symbols
   */
static int fr(FILE* fp, char* p, int s)
{   
  int r;

  r = fread(p, s, 1, fp);
  if (r && (r != 1))
    sysError("imageRead count error","");
  return r;
}

void MemoryManager::imageRead(FILE* fp)
{   
  long i, size;

  ignore fr(fp, (char *) &symbols, sizeof(object));
  i = 0;

  while(fr(fp, (char *) &dummyObject, sizeof(dummyObject))) 
  {
    i = dummyObject.di;

    if ((i < 0) || (i >= objectTable.size()))
    {
        // Grow enough, plus a bit.
        growObjectStore(i - objectTable.size() + 500);
    }
    objectFromID(i)._class = dummyObject.cl;
    if ((objectFromID(i)._class < 0) || 
        ((objectFromID(i)._class) >= objectTable.size())) 
    {
        // Grow enough, plus a bit.
        growObjectStore(objectFromID(i)._class - objectTable.size() + 500);
    }
    objectFromID(i).size = size = dummyObject.ds;
    if (size < 0) size = ((- size) + 1) / 2;
    if (size != 0) 
    {
      objectFromID(i).memory = mBlockAlloc((int) size);
      ignore fr(fp, (char *) objectFromID(i).memory,
          sizeof(object) * (int) size);
    }
    else
      objectFromID(i).memory = (object *) 0;

    objectFromID(i).referenceCount = 666;
  }
  setFreeLists();
}

/*
   imageWrite - write out an object image
   */

static void fw(FILE* fp, char* p, int s)
{
  if (fwrite(p, s, 1, fp) != 1) {
    sysError("imageWrite size error","");
  }
}

void MemoryManager::imageWrite(FILE* fp)
{   
  long i, size;

  MemoryManager::Instance()->garbageCollect();

  fw(fp, (char *) &symbols, sizeof(object));

  for (i = 0; i < objectTable.size(); i++) 
  {
    if (MemoryManager::Instance()->objectFromID(i).referenceCount > 0) 
    {
      dummyObject.di = i;
      dummyObject.cl = MemoryManager::Instance()->objectFromID(i)._class;
      dummyObject.ds = size = MemoryManager::Instance()->objectFromID(i).size;
      fw(fp, (char *) &dummyObject, sizeof(dummyObject));
      if (size < 0) size = ((- size) + 1) / 2;
      if (size != 0)
        fw(fp, (char *) MemoryManager::Instance()->objectFromID(i).memory,
            sizeof(object) * size);
    }
  }
}

void MemoryManager::disableGC(bool disable)
{
    noGC = disable;
}

void MemoryManager::refObject(long o)
{
    objectReferences[o]++;
}

void MemoryManager::unrefObject(long o)
{
    TObjectRefs::iterator i;
    if((i = objectReferences.find(o)) != objectReferences.end())
    {
        if(--i->second <= 0)
            objectReferences.erase(i);
    }
}


size_t MemoryManager::growObjectStore(size_t amount)
{
    // Empty object to use when resizing.
    objectStruct empty = {nilobj, 0, 0, NULL};

    size_t currentSize = objectTable.size();
    objectTable.resize(objectTable.size()+amount, empty);

    if(debugging)
        fprintf(stderr, "Growing object store to %d\n", objectTable.size());

    // Add all new objects to the free list.
    for(size_t i = currentSize; i < objectTable.size(); ++i)
    {
       objectFreeList.insert(std::pair<size_t, object>(0, i));
       objectFreeListInv.insert(std::pair<object, size_t>(i, 0));
    } 
    return objectTable.size();
}

void MemoryManager::setGrowAmount(size_t amount)
{
    growAmount = amount;
}


object* objectStruct::sysMemPtr()
{
    return memory;
}

byte* objectStruct::bytePtr()
{
    return (byte *)sysMemPtr();
}

char* objectStruct::charPtr()
{
    return (char *)sysMemPtr();
}


void objectStruct::basicAtPut(int i, object v)
{
    if((i <= 0) || (i > size))
        sysError("index out of range", "basicAtPut");
    else
        sysMemPtr()[i-1] = v;
}

void objectStruct::basicAtPut(int i, const ObjectHandle& v)
{
    if((i <= 0) || (i > size))
        sysError("index out of range", "basicAtPut");
    else
        sysMemPtr()[i-1] = v.handle();
}

int objectStruct::byteAt(int i)
{
    byte* bp;
    unsigned char t;

    if((i <= 0) || (i > 2 * - size))
        sysError("index out of range", "byteAt");
    else
    {
        bp = bytePtr();
        t = bp[i-1];
        i = (int) t;
    }
    return i;
}

void objectStruct::byteAtPut(int i, int x)
{
    byte *bp;
    if ((i <= 0) || (i > 2 * - size)) 
    {
        sysError("index out of range", "byteAtPut");
    }
    else 
    {
        bp = bytePtr();
        bp[i-1] = x;
    }
}

object objectStruct::basicAt(int i)
{
    if(( i <= 0) || (i > size))
        sysError("index out of range", "basicAt");
    else
        return (sysMemPtr()[i-1]);

    return nilobj;
}

double objectStruct::floatValue()
{   
    double d;

    ncopy((char *) &d, charPtr(), (int) sizeof(double));
    return d;
}

int objectStruct::intValue()
{   
    int d;

    if(this == &objectRef(nilobj))
        return 0;
    ncopy((char *) &d, charPtr(), (int) sizeof(int));
    return d;
}

void* objectStruct::cPointerValue()
{   
    if(NULL == charPtr())
        sysError("invalid cPointer","cPointerValue");

    void* l;

    int s = sizeof(void*);
    ncopy((char *) &l, charPtr(), (int) sizeof(void*));
    return l;
}
