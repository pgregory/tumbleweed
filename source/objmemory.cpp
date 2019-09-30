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
#include "objmemory.h"
#include "interp.h"
#include "names.h"


bool debugging = false;

extern object firstProcess;
object symbols;     /* table of all symbols created */

ObjectHandle* ObjectHandle::head = NULL;
ObjectHandle* ObjectHandle::tail = NULL;

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


void MemoryManager::Initialise(size_t initialSize, size_t growCount)
{
    if(NULL != m_pInstance)
        delete(m_pInstance);

    m_pInstance = new MemoryManager(initialSize, growCount);
}


MemoryManager::MemoryManager(size_t initialSize, size_t growCount) : noGC(false), growAmount(growCount)
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


object MemoryManager::allocObject(size_t memorySize)
{
    int i;
    size_t position;
    bool done;
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

    /* set class and type */
    objectTable[position].referenceCount = 0;
    objectTable[position]._class = nilobj;
    objectTable[position].size = memorySize;
    return(position << 1);
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
        strcpy(t, str);
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
    register struct ObjectStruct *p;
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

    if (x && (!(x&1))) 
    {
        if (--(objectFromID(x).referenceCount) == -1) 
        {
            /* then it's the first time we've visited it, so: */
            ObjectStruct& obj = objectFromID(x);
            ObjectStruct& thisClass = objectFromID(obj._class);
            visit(objectFromID(x)._class);
            s = objectRef(x).size;
            if (s>0) 
            {
                p = objectFromID(x).memory;
                for (i=s; i; --i)
                {
                    visit(*p++);
                }
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
    ObjectHandle* explicitRef = ObjectHandle::getListHead();
    while(explicitRef)
    {
        visit(*explicitRef);
        explicitRef = explicitRef->next();
    }

    /* add new garbage to objectFreeList 
    * toggle referenceCount 
    * count the objects
    */

    for (j=objectTable.size()-1; j>0; j--) 
    {
        if (objectTable[j].referenceCount == 0) 
        {
            if(destroyObject(j))
                f++;
        } 
        else
        {
            if (0!=(objectTable[j].referenceCount = -objectTable[j].referenceCount))
                c++;
        }
    }

    if (debugging)
    {
        fprintf(stderr," %d references.\n",ObjectHandle::numTotalHandles());
        fprintf(stderr," %d objects - %d freed.\n",c,f);
    }

    return f;
}


size_t MemoryManager::objectCount()
{   
    return storageSize() - freeSlotsCount();
}

size_t MemoryManager::storageSize() const
{
    return objectTable.size();
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

  fr(fp, (char *) &symbols, sizeof(object));
  i = 0;

  while(fr(fp, (char *) &dummyObject, sizeof(dummyObject))) 
  {
    i = dummyObject.di;

    if ((i < 0) || (i >= objectTable.size()))
    {
        // Grow enough, plus a bit.
        growObjectStore(i - objectTable.size() + 500);
    }
    objectTable[i]._class = dummyObject.cl;
    if ((objectTable[i]._class < 0) || 
        ((objectTable[i]._class) >= objectTable.size())) 
    {
        // Grow enough, plus a bit.
        growObjectStore(objectTable[i]._class - objectTable.size() + 500);
    }
    objectTable[i].size = size = dummyObject.ds;
    if (size < 0) size = ((- size) + 1) / 2;
    if (size != 0) 
    {
      objectTable[i].memory = mBlockAlloc((int) size);
      fr(fp, (char *) objectTable[i].memory,
          sizeof(object) * (int) size);
    }
    else
      objectTable[i].memory = (object *) 0;

    objectTable[i].referenceCount = 666;
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

  garbageCollect();

  fw(fp, (char *) &symbols, sizeof(object));

  for (i = 0; i < objectTable.size(); i++) 
  {
    if (objectTable[i].referenceCount > 0) 
    {
      dummyObject.di = i;
      dummyObject.cl = objectTable[i]._class;
      dummyObject.ds = size = objectTable[i].size;
      fw(fp, (char *) &dummyObject, sizeof(dummyObject));
      if (size < 0) size = ((- size) + 1) / 2;
      if (size != 0)
        fw(fp, (char *) objectTable[i].memory,
            sizeof(object) * size);
    }
  }
}

void MemoryManager::disableGC(bool disable)
{
    noGC = disable;
}


size_t MemoryManager::growObjectStore(size_t amount)
{
    // Empty object to use when resizing.
    ObjectStruct empty = {nilobj, 0, 0, NULL};

    size_t currentSize = objectTable.size();
    objectTable.resize(objectTable.size()+amount, empty);

    if(debugging)
        fprintf(stderr, "Growing object store to %d\n", static_cast<int>(objectTable.size()));

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




int ObjectStruct::byteAt(int i)
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

void ObjectStruct::byteAtPut(int i, int x)
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

object ObjectStruct::basicAt(int i)
{
    if(( i <= 0) || (i > size))
    {
        fprintf(stderr, "Indexing: %d\n", i);
        sysError("index out of range", "basicAt");
    }
    else
        return (sysMemPtr()[i-1]);

    return nilobj;
}

double ObjectStruct::floatValue()
{   
    double d;

    memcpy((char *) &d, charPtr(), (int) sizeof(double));
    return d;
}

int ObjectStruct::intValue()
{   
    int d;

    if(this == &objectRef(nilobj))
        return 0;
    memcpy((char *) &d, charPtr(), (int) sizeof(int));
    return d;
}

void* ObjectStruct::cPointerValue()
{   
    if(NULL == charPtr())
        sysError("invalid cPointer","cPointerValue");

    void* l;

    int s = sizeof(void*);
    memcpy((char *) &l, charPtr(), (int) sizeof(void*));
    return l;
}

int ObjectHandle::numTotalHandles()
{
    if(NULL == ObjectHandle::head)
        return 0;
    else
    {
        int total = 1;
        ObjectHandle* head = ObjectHandle::head;
        while(head->m_next)
        {
            ++total;
            head = head->m_next;
        }
        return total;
    }
}

bool ObjectHandle::isReferenced(long handle)
{
    if(NULL == ObjectHandle::head)
        return false;
    else
    {
        ObjectHandle* head = ObjectHandle::head;
        while(head->m_next)
        {
			if(head->m_handle == handle)
				return true;
            head = head->m_next;
        }
		if(head->m_handle == handle)
			return true;
        return false;
    }
}


ObjectHandle* ObjectHandle::getListHead()
{
    return ObjectHandle::head;
}

ObjectHandle* ObjectHandle::getListTail()
{
    return ObjectHandle::tail;
}
