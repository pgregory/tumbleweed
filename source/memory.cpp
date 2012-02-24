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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "env.h"
#include "memory.h"
#include "interp.h"

boolean debugging = false;
object sysobj;  /* temporary used to avoid rereference in macros */
object intobj;

extern object processStack;
extern object firstProcess;
object symbols;     /* table of all symbols created */

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

MemoryManager* theMemoryManager;

/*
    The following variables are strictly local to the memory
    manager module

    FREELISTMAX defines the maximum size of any object.
*/

/* initialize the memory management module */
noreturn initMemoryManager() 
{
    theMemoryManager = new MemoryManager();
}

/* setFreeLists - initialise the free lists */
void setFreeLists() 
{
    theMemoryManager->setFreeLists();
}

/* allocate a new memory object */
object allocObject(int memorySize)
{   
    return theMemoryManager->allocObject(memorySize);
}

object allocByte(int size)
{  
    object newObj;

    newObj = allocObject((size + 1) / 2);
    /* negative size fields indicate bit objects */
    objectRef(newObj).setSizeField(-size);
    return newObj;
}

object allocStr(register const char* str)
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

/* do the real work in the decr procedure */
void sysDecr(object z, int visit)
{   
    theMemoryManager->sysDecr(z, visit);
}


/*
  Written by Steven Pemberton:
  The following routine assures that objects read in are really referenced,
  eliminating junk that may be in the object file but not referenced.
  It is essentially a marking garbage collector algorithm using the 
  reference counts as the mark
  
  Modified by Paul Gregory based on ideas by Michael Koehne
*/

void visit(register object x)
{
    int i, s;
    object *p;

    if (x) 
    {
        if (--(theMemoryManager->objectFromID(x).referenceCount) == -1) 
        {
            /* then it's the first time we've visited it, so: */
            visit(theMemoryManager->objectFromID(x)._class);
            s = objectRef(x).sizeField();
            if (s>0) 
            {
                p = theMemoryManager->objectFromID(x).memory;
                for (i=s; i; --i) 
                    visit(*p++);
            }
        }
    }
}

int objectCount()
{   
    register int i, j;
    j = 0;
    for (i = 0; i < ObjectTableMax; i++)
        if (theMemoryManager->objectFromID(i).referenceCount > 0)
            j++;
    return j;
}


int garbageCollect(int verbose)
{
    return theMemoryManager->garbageCollect(verbose);
}





MemoryManager::MemoryManager()
{
    int i;

    objectTable.resize(ObjectTableMax);

    /* set all the reference counts to zero */
    for (i = 0; i < ObjectTableMax; i++) 
    {
        objectTable[i].referenceCount = 0; 
        objectTable[i].size = 0; 
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
            if(garbageCollect(false) > 0)
            {
                return allocObject(memorySize);
            }
            else
            {
                printf("Object count: %d\n", objectCount());
                sysError("out of objects","alloc");
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
    objectRef(newObj).setSizeField(-size);
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

bool MemoryManager::sysDecr(object z, int visit)
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
    for(int z=ObjectTableMax-1; z>0; z--)
    {
        // If really unused, sysDecr will take care
        // of cleaning up and adding to the free list.
        if (objectTable[z].referenceCount == 0)
            sysDecr(z, 0);
    }
}

void MemoryManager::visitMethodCache()
{
    for(int i = 0; i < cacheSize; ++i)
    {
        visit(methodCache[i].cacheMessage);
        visit(methodCache[i].lookupClass);
        visit(methodCache[i].cacheClass);
        visit(methodCache[i].cacheMethod);
    }
}

int MemoryManager::garbageCollect(int verbose)
{
    register int j;
    int c=1,f=0;

    if (verbose) 
        fprintf(stderr,"\ngarbage collecting ... ");

    for(int x = ObjectTableMax-1; x>0; --x)
        objectFromID(x).referenceCount = 0;
    /* visit symbols and firstProcess to toggle their referenceCount */

    visit(symbols);
    if(firstProcess) 
        visit(firstProcess);
    visitMethodCache();

    /* add new garbage to objectFreeList 
    * toggle referenceCount 
    * count the objects
    */

    for (j=ObjectTableMax-1; j>0; j--) 
    {
        if (objectFromID(j).referenceCount == 0) 
        {
            if(sysDecr(j,0))
                f++;
        } 
        else
        {
            if (0!=(objectFromID(j).referenceCount = -objectFromID(j).referenceCount))
                c++;
        }
    }

    if (verbose)
        fprintf(stderr," %d objects - %d freed.\n",c,f);

    numAllocated = 0;
    return f;
}

objectStruct& MemoryManager::objectFromID(object id)
{
    return objectTable[id];
}






object objectStruct::classField()
{
    return _class;
}

void objectStruct::setClass(object y)
{
    _class = y;
}

short objectStruct::sizeField()
{
    return size;
}

void objectStruct::setSizeField(short _size)
{
    size = _size;
}

object* objectStruct::sysMemPtr()
{
    return memory;
}

object* objectStruct::memoryPtr()
{
    return sysMemPtr();
}

byte* objectStruct::bytePtr()
{
    return (byte *)memoryPtr();
}

char* objectStruct::charPtr()
{
    return (char *)memoryPtr();
}


void objectStruct::simpleAtPut(int i, object v)
{
    if((i <= 0) || (i > sizeField()))
        sysError("index out of range", "simpleAtPut");
    else
        sysMemPtr()[i-1] = v;
}

void objectStruct::basicAtPut(int i, object v)
{
    simpleAtPut(i, v);
    objectRef(v).incr();
}

void objectStruct::incr()
{
    //++referenceCount;
}

void objectStruct::fieldAtPut(int i, object v)
{
    basicAtPut(i, v);
}

int objectStruct::byteAt(int i)
{
    byte* bp;
    unsigned char t;

    if((i <= 0) || (i > 2 * - sizeField()))
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
    if ((i <= 0) || (i > 2 * - sizeField())) 
    {
        fprintf(stderr,"index %d size %d\n", i, sizeField());
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
    if(( i <= 0) || (i > sizeField()))
        sysError("index out of range", "basicAt");
    else
        return (sysMemPtr()[i-1]);

    return nilobj;
}

