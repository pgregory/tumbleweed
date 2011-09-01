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

boolean debugging = false;
object sysobj;  /* temporary used to avoid rereference in macros */
object intobj;

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

struct objectStruct *objectTable;

/*
    The following variables are strictly local to the memory
    manager module

    FREELISTMAX defines the maximum size of any object.
*/

# define FREELISTMAX 2000
static object objectFreeList[FREELISTMAX];/* free list of objects */

# ifndef mBlockAlloc

# define MemoryBlockSize 6000   /* the current memory block being hacked up */
static object *memoryBlock;     /* malloc'ed chunck of memory */
static int    currentMemoryPosition;    /* last used position in above */

# endif


/* initialize the memory management module */
noreturn initMemoryManager() 
{
    int i;

  objectTable = calloc(ObjectTableMax, sizeof(struct objectStruct));
  if (! objectTable)
    sysError("cannot allocate","object table");

  /* set all the free list pointers to zero */
  for (i = 0; i < FREELISTMAX; i++)
    objectFreeList[i] = nilobj;

    /* set all the reference counts to zero */
    for (i = 0; i < ObjectTableMax; i++) 
    {
        objectTable[i].referenceCount = 0;
        objectTable[i].size = 0;
    }

  /* make up the initial free lists */
  setFreeLists();

# ifndef mBlockAlloc
  /* force an allocation on first object assignment */
  currentMemoryPosition = MemoryBlockSize + 1;
# endif

  /* object at location 0 is the nil object, so give it nonzero ref */
  objectTable[0].referenceCount = 1;
  objectTable[0].size = 0;
}

/* setFreeLists - initialise the free lists */
setFreeLists() {
#if 0
    int i, size;
    register int z;
    register struct objectStruct *p;

    objectFreeList[0] = nilobj;

    for (z=ObjectTableMax-1; z>0; z--) {
        if (objectTable[z].referenceCount == 0){
      sysDecr(z, 0);
            /* Unreferenced, so do a sort of sysDecr: */
            p= &objectTable[z];
            size = p->size;
            if (size < 0) size = ((-size) + 1)/2;
            p->class = objectFreeList[size];
            objectFreeList[size]= z;
            for (i= size; i>0; )
                p->memory[--i] = nilobj;
            }
        }
#endif
    register unsigned int z;

    /* set all the free list pointers to zero */
    for (z = 0; z < FREELISTMAX; z++)
        objectFreeList[z] = nilobj;

    /* add free objects */
    for (z=ObjectTableMax-1; z>0; z--)
        if (objectTable[z].referenceCount == 0)
        sysDecr(z<<1, 0);
}

/*
  mBlockAlloc - rip out a block (array) of object of the given size from
    the current malloc block 
*/
# ifndef mBlockAlloc

object *mBlockAlloc(int memorySize)
{   
    object *objptr;

    if (currentMemoryPosition + memorySize >= MemoryBlockSize) 
    {
        /* we toss away space here.  Space-Frugal users may want to
        fix this by making a new object of size
        MemoryBlockSize - currentMemoryPositon - 1
        and putting it on the free list, but I think
        the savings is potentially small */

        memoryBlock = (object *) calloc((unsigned) MemoryBlockSize, sizeof(object));
        if (! memoryBlock)
            sysError("out of memory","malloc failed");
        currentMemoryPosition = 0;
    }
    objptr = (object *) &memoryBlock[currentMemoryPosition];
    currentMemoryPosition += memorySize;
    return(objptr);
}
# endif

/* allocate a new memory object */
object allocObject(int memorySize)
{   
    int i;
    register int position;
    boolean done;

    if (memorySize >= FREELISTMAX) 
    {
        fprintf(stderr,"size %d\n", memorySize);
        sysError("allocation bigger than permitted","allocObject");
    }

    /* first try the free lists, this is fastest */
    if ((position = objectFreeList[memorySize]) != 0) 
    {
        objectFreeList[memorySize] = objectTable[position].class;
    }

    /* if not there, next try making a size zero object and
        making it bigger */
    else if ((position = objectFreeList[0]) != 0) 
    {
        objectFreeList[0] = objectTable[position].class;
        objectTable[position].size = memorySize;
        objectTable[position].memory = mBlockAlloc(memorySize);
    }

    else 
    {      /* not found, must work a bit harder */
        done = false;

        /* first try making a bigger object smaller */
        for (i = memorySize + 1; i < FREELISTMAX; i++)
        {
            if ((position = objectFreeList[i]) != 0) 
            {
                objectFreeList[i] = objectTable[position].class;
                /* just trim it a bit */
                objectTable[position].size = memorySize;
                done = true;
                break;
            }
        }

        /* next try making a smaller object bigger */
        if (! done)
        {
            for (i = 1; i < memorySize; i++)
            {
                if ((position = objectFreeList[i]) != 0) 
                {
                    objectFreeList[i] =
                        objectTable[position].class;
                    objectTable[position].size = memorySize;
# ifdef mBlockAlloc
                    free(objectTable[position].memory);
# endif
                    objectTable[position].memory = 
                        mBlockAlloc(memorySize);
                    done = true;
                    break;
                }
            }
        }

        /* if we STILL don't have it then there is nothing */
        /* more we can do */
        if (! done)
        {
            printf("Object count: %d\n", objectCount());
            sysError("out of objects","alloc");
        }
    }

    /* set class and type */
    objectTable[position].referenceCount = 0;
    objectTable[position].class = nilobj;
    objectTable[position].size = memorySize;
    return(position << 1);
}

object allocByte(int size)
{  
    object newObj;

    newObj = allocObject((size + 1) / 2);
    /* negative size fields indicate bit objects */
    sizeField(newObj) = - size;
    return newObj;
}

object allocStr(register char* str)
{   
    register object newSym;
    char* t;

    if(NULL != str)
    {
        newSym = allocByte(1 + strlen(str));
        t = charPtr(newSym);
        ignore strcpy(t, str);
    }
    else
    {
        newSym = allocByte(1);
        charPtr(newSym)[0] = '\0';
    }
    return(newSym);
}

void incr(object o)
{
    if(nilobj != o)
        objectTable[o>>1].referenceCount++;
}

void decr(object o)
{
  if(nilobj != o)
  {
    if( --objectTable[o>>1].referenceCount <= 0)
      sysDecr(o, 1);
  }
}

/* do the real work in the decr procedure */
void sysDecr(object z, int visit)
{   
    register struct objectStruct *p;
    register int i;
    int size;

    p = &objectTable[z>>1];
    if (p->referenceCount < 0) 
    {
        fprintf(stderr,"object %d\n", z);
        sysError("negative reference count","");
    }
    if(visit) decr(p->class);
    size = p->size;
    if (size < 0) size = ((- size) + 1) /2;
    p->class = objectFreeList[size];
    objectFreeList[size] = z>>1;
    if (size > 0) 
    {
        if (visit && p->size > 0)
        {
            for (i = size; i; )
                decr(p->memory[--i]);
        }
        for (i = size; i > 0; )
            p->memory[--i] = nilobj;
    }
    p->size = size;
}



object basicAt(object o, int i)
{
    if(( i <= 0) || (i > sizeField(o)))
        sysError("index out of range", "basicAt");
    else
        return (sysMemPtr(o)[i-1]);

    return nilobj;
}

void simpleAtPut(object o, int i, object v)
{
    if((i <= 0) || (i > sizeField(o)))
        sysError("index out of range", "simpleAtPut");
    else
        sysMemPtr(o)[i-1] = v;
}

void basicAtPut(object o, int i, object v)
{
    simpleAtPut(o, i, v);
    incr(v);
}

void fieldAtPut(object o, int i, object v)
{
    decr(basicAt(o, i));
    basicAtPut(o, i, v);
}

int byteAt(object o, int i)
{
    byte* bp;
    unsigned char t;

    if((i <= 0) || (i > 2 * - sizeField(o)))
        sysError("index out of range", "byteAt");
    else
    {
        bp = bytePtr(o);
        t = bp[i-1];
        i = (int) t;
    }
    return i;
}

void byteAtPut(object z, int i, int x)
{      
    byte *bp;
    if ((i <= 0) || (i > 2 * - sizeField(z))) 
    {
        fprintf(stderr,"index %d size %d\n", i, sizeField(z));
        sysError("index out of range", "byteAtPut");
    }
    else 
    {
        bp = bytePtr(z);
        bp[i-1] = x;
    }
}

/*
  Written by Steven Pemberton:
  The following routine assures that objects read in are really referenced,
  eliminating junk that may be in the object file but not referenced.
  It is essentially a marking garbage collector algorithm using the 
  reference counts as the mark
  
  Modified by Paul Gregory based on ideas by Michael Koehne
*/

visit(register object x)
{
    int i, s;
    object *p;

    if (x) 
    {
        if (objectTable[x>>1].referenceCount > 0)
            objectTable[x>>1].referenceCount = 0;
        if (--(objectTable[x>>1].referenceCount) == -1) 
        {
    //      if (++(objectTable[x>>1].referenceCount) == 1) 
    //      {
                /* then it's the first time we've visited it, so: */
                visit(objectTable[x>>1].class);
                s = sizeField(x);
                if (s>0) 
                {
                    p = objectTable[x>>1].memory;
                    for (i=s; i; --i) visit(*p++);
                }
    //      }
        }
    }
}

int objectCount()
{   
    register int i, j;
    j = 0;
    for (i = 0; i < ObjectTableMax; i++)
        if (objectTable[i].referenceCount > 0)
            j++;
    return j;
}


int garbageCollect(int verbose)
{
    register int j;
    int c=1,f=0;

    if (verbose) 
    {
        fprintf(stderr,"\ngarbage collecting ... ");
        fprintf(stderr,"%d objects ...", objectCount());
    }

    /* visit symbols and firstProcess to toggle their referenceCount */

    visit(symbols);
    if(firstProcess) 
    visit(firstProcess);

    /* add new garbage to objectFreeList 
    * toggle referenceCount 
    * count the objects
    */

    for (j=ObjectTableMax-1; j>0; j--) 
    {
        if (objectTable[j].referenceCount > 0) 
        {
            objectTable[j].referenceCount = 0;
            sysDecr(j<<1,0);
            f++;
        } 
        else
        {
            if (0!=(objectTable[j].referenceCount =
                -objectTable[j].referenceCount))
            c++;
        }
    }

    if (verbose)
        fprintf(stderr," %d objects.\n",c);
    return c;
}
