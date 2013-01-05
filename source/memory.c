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
#include "names.h"
#include "primitive.h"
#include "uthash.h"


int debugging = FALSE;
ObjectStruct _nilobj =
{
    NULL,
    //0,0,
    0, 
    0,
    NULL
};
object nilobj = &_nilobj;

extern object firstProcess;
object symbols;     /* table of all symbols created */

static unsigned short NextHashValue = 0;


ObjectPool* g_HeadPool = 0;
ObjectPool* g_CurrentPool = 0;
int g_DisableGC = 0;

typedef struct _visitedObject
{
    object obj;
    UT_hash_handle hh;
} visitedObject;

static struct 
{
    object ref;
    object _classRef;
    long size;
    unsigned short identityHash;
} dummyObject;



typedef struct _visitor
{
    int(*test)(struct _visitor* env);
    void(*preFunc)(struct _visitor* env);
    void(*postFunc)(struct _visitor* env);

    visitedObject* visitedObjects;
    unsigned long count;
    object o;
} Visitor;

typedef struct _fileVisitor
{
    Visitor super;

    FILE* fp;
} FileVisitor;


void visit(Visitor* cb);

//
// ncopy - copy exactly n bytes from place to place 
// 
void ncopy(register char* p, register char* q, register int n)      
{   
    for (; n>0; n--)
        *p++ = *q++;
}


static object* mBlockAlloc(int size)
{
    int i;
    object* result = (object *) malloc((unsigned) size * sizeof(object));

    for(i = 0; i < size; ++i)
        result[i] = nilobj;

    return result;
}

enum { g_FreeListBufferSize = 128 };
/* Opaque buffer element type.  This would be defined by the application. */
typedef struct { object value; } FreeListType;
 
/* Circular buffer object */
typedef struct 
{
    int         size;   /* maximum number of elements           */
    int         start;  /* index of oldest element              */
    int         end;    /* index at which to write new element  */
    FreeListType    elems[g_FreeListBufferSize + 1];  /* vector of elements                   */
} FreeListBuffer;

FreeListBuffer g_FreeListBuffer = 
{
    g_FreeListBufferSize + 1,
    0,
    0
};

int freeListIsFull(FreeListBuffer *cb) 
{
    return (cb->end + 1) % cb->size == cb->start; 
}
 
int freeListIsEmpty(FreeListBuffer *cb) 
{
    return cb->end == cb->start; 
}
 
/* Write an element, overwriting oldest element if buffer is full. App can
   choose to avoid the overwrite by checking freeListIsFull(). */
void freeListAdd(FreeListBuffer *cb, FreeListType *elem) 
{
    cb->elems[cb->end] = *elem;
    cb->end = (cb->end + 1) % cb->size;
    if (cb->end == cb->start)
        cb->start = (cb->start + 1) % cb->size; /* full, overwrite */
}
 
/* Read oldest element. App must ensure !freeListIsEmpty() first. */
void freeListRead(FreeListBuffer *cb, FreeListType *elem) 
{
    *elem = cb->elems[cb->start];
    cb->start = (cb->start + 1) % cb->size;
}

extern object processStack;

void garbageCollect()
{
    Visitor v;
    ObjectPool* pool;
    visitedObject* vo, *tmp;
    int i, freed = 0, held = 0;
    FreeListType elem;
    SObjectHandle* handle;
    object search;

    if(g_DisableGC || freeListIsFull(&g_FreeListBuffer))
        return;

    v.preFunc = NULL;
    v.postFunc = NULL;
    v.test = NULL;
    v.count = 0;
    v.visitedObjects = NULL;

    // Visit the root objects
    v.o = symbols;
    visit(&v);
    v.o = processStack;
    visit(&v);
    v.o = firstProcess;
    visit(&v);

    // Visit all the locked objects
    handle = head;
    while(handle)
    {
        v.o = handle->m_handle;
        visit(&v);
        handle = handle->m_next;
    }

    pool = g_HeadPool;
    while(NULL != pool)
    {
        for(i = 0; i < pool->top; ++i)
        {
            search = &pool->pool[i];
            HASH_FIND_PTR(v.visitedObjects, &search, vo);
            if((search != nilobj) && (vo == NULL))
            {
                elem.value = &(pool->pool[i]);
                freeListAdd(&g_FreeListBuffer, &elem);
                freed++;
                if(freeListIsFull(&g_FreeListBuffer))
                    break;
            }
        }
        pool = pool->next;
        if(freeListIsFull(&g_FreeListBuffer))
            break;
    }

    HASH_ITER(hh, v.visitedObjects, vo, tmp) 
    {
        HASH_DEL(v.visitedObjects, vo);
        free(vo);
    }
}

#if defined TW_DEBUG
static void checkNotReferenced(object o)
{
    Visitor v;
    visitedObject* vo, *tmp;

    v.preFunc = NULL;
    v.postFunc = NULL;
    v.test = NULL;
    v.o = symbols;
    v.count = 0;
    v.visitedObjects = NULL;
    visit(&v);

    HASH_FIND_PTR(v.visitedObjects, &o, vo);
    if(vo != NULL)
        printf("Object is referenced unexpectedly!\n");

    HASH_ITER(hh, v.visitedObjects, vo, tmp) 
    {
        HASH_DEL(v.visitedObjects, vo);
        free(vo);
    }
}
#endif

static object reserveObject()
{
    FreeListType elem;

    if(NULL == g_HeadPool || NULL == g_CurrentPool)
    {
        g_HeadPool = g_CurrentPool = (ObjectPool*)malloc(sizeof(ObjectPool));
        g_HeadPool->next = NULL;
        g_HeadPool->prev = NULL;
        g_HeadPool->top = 0;
    }

    if(g_CurrentPool->top >= g_DefaultObjectPoolSize)
    {
        if(freeListIsEmpty(&g_FreeListBuffer))
            garbageCollect();
        if(freeListIsEmpty(&g_FreeListBuffer))
        {
            // Check if we can do a GC to release some objects.
            g_CurrentPool->next = (ObjectPool*)malloc(sizeof(ObjectPool));
            g_CurrentPool->next->next = NULL;
            g_CurrentPool->next->prev = g_CurrentPool;
            g_CurrentPool->next->top = 0;
            g_CurrentPool = g_CurrentPool->next;
        }
        else
        {
            freeListRead(&g_FreeListBuffer, &elem);
            //checkNotReferenced(elem.value);
            return elem.value;
        }
    }

    return &g_CurrentPool->pool[g_CurrentPool->top++];
}


object allocObject(size_t memorySize)
{
    object ob = reserveObject();
    if(memorySize)
        ob->memory = mBlockAlloc(memorySize);
    else
        ob->memory = NULL;
    ob->size = memorySize;
    ob->_class = nilobj;
    ob->identityHash = NextHashValue++;

    return ob;
}

object allocByte(size_t size)
{
    object newObj;

    newObj = allocObject((size + 1) / 2);
    /* negative size fields indicate bit objects */
    newObj->size = -(long)size;
    return newObj;
}

object allocStr(register const char* str)
{
    register object newSym;
    char* t;

    if(NULL != str)
    {
        newSym = allocByte(1 + strlen(str));
        t = charPtr(newSym);
        strcpy(t, str);
    }
    else
    {
        newSym = allocByte(1);
        charPtr(newSym)[0] = '\0';
    }
    return(newSym);
}

int destroyObject(object z)
{
    free(z->memory);
    free(z);

    return TRUE;
}


size_t objectCount()
{   
    return storageSize() - freeSlotsCount();
}

size_t storageSize()
{
    return 0;
}

size_t freeSlotsCount()
{   
    return 0;
}

object newArray(int size)
{   
    object newObj;

    newObj = allocObject(size);
    newObj->_class = CLASSOBJECT(Array);
    return newObj;
}

object newBlock()
{   
    object newObj;

    newObj = allocObject(blockSize);
    newObj->_class = CLASSOBJECT(Block);
    return newObj;
}

object newByteArray(int size)
{   
    object newobj;

    newobj = allocByte(size);
    newobj->_class = CLASSOBJECT(ByteArray);
    return newobj;
}

object newChar(int value)
{   
    object newobj;
    SObjectHandle* newobj_handle;

    newobj = allocObject(1);
    newobj_handle = new_SObjectHandle_from_object(newobj);
    basicAtPut(newobj,1, newInteger(value));
    newobj->_class = CLASSOBJECT(Char);
    free_SObjectHandle(newobj_handle);

    return(newobj);
}

object newClass(const char* name)
{   
    object newObj, nameObj, methTable;
    SObjectHandle *newObj_handle, *nameObj_handle, *methTable_handle;

    newObj = allocObject(classSize);
    newObj_handle = new_SObjectHandle_from_object(newObj);
    newObj->_class = CLASSOBJECT(Class);

    /* now make name */
    nameObj = newSymbol(name);
    nameObj_handle = new_SObjectHandle_from_object(nameObj);
    basicAtPut(newObj,nameInClass, nameObj); 
    methTable = newDictionary(39);
    methTable_handle = new_SObjectHandle_from_object(methTable);
    basicAtPut(newObj,methodsInClass, methTable);
    basicAtPut(newObj,sizeInClass, newInteger(classSize));

    /* now put in global symbols table */
    nameTableInsert(symbols, strHash(name), nameObj, newObj);

    free_SObjectHandle(methTable_handle);
    free_SObjectHandle(nameObj_handle);
    free_SObjectHandle(newObj_handle);

    return newObj;
}

object newContext(int link, object method, object args, object temp)
{   
    object newObj;
    SObjectHandle* newObj_handle;

    newObj = allocObject(contextSize);
    newObj_handle = new_SObjectHandle_from_object(newObj);
    newObj->_class = CLASSOBJECT(Context);
    basicAtPut(newObj,linkPtrInContext, newInteger(link));
    basicAtPut(newObj,methodInContext, method);
    basicAtPut(newObj,argumentsInContext, args);
    basicAtPut(newObj,temporariesInContext, temp);
    free_SObjectHandle(newObj_handle);

    return newObj;
}

object newDictionary(int size)
{   
    object newObj;
    SObjectHandle* newObj_handle;

    newObj = allocObject(dictionarySize);
    newObj_handle = new_SObjectHandle_from_object(newObj);
    newObj->_class = CLASSOBJECT(Dictionary);
    basicAtPut(newObj,tableInDictionary, newArray(size));
    free_SObjectHandle(newObj_handle);

    return newObj;
}

object newFloat(double d)
{   
    object newObj;

    newObj = allocByte((int) sizeof (double));
    ncopy(charPtr(newObj), (char *) &d, (int) sizeof (double));
    newObj->_class = CLASSOBJECT(Float);
    return newObj;
}

object newInteger(int i)
{   
#if defined TW_SMALLINTEGER_AS_OBJECT
    object newObj;

    newObj = allocByte((int) sizeof (int));
    ncopy(charPtr(newObj), (char *) &i, (int) sizeof (int));
    newObj->_class = CLASSOBJECT(Integer);
    return newObj;
#else
    return (object)((i << 1) + 1);
#endif
}

object newCPointer(void* l)
{   
    object newObj;

    int s = sizeof(void*);
    newObj = allocByte((int) sizeof (void*));
    ncopy(charPtr(newObj), (char *) &l, (int) sizeof (void*));
    newObj->_class = CLASSOBJECT(CPointer);
    return newObj;
}

object newLink(object key, object value)
{   
    object newObj;

    newObj = allocObject(linkSize);
    newObj->_class = CLASSOBJECT(Link);
    basicAtPut(newObj,keyInLink, key);
    basicAtPut(newObj,valueInLink, value);
    return newObj;
}

object newMethod()
{   object newObj;

    newObj = allocObject(methodSize);
    newObj->_class = CLASSOBJECT(Method);
    return newObj;
}

object newStString(const char* value)
{   
    object newObj;

    newObj = allocStr(value);
    newObj->_class = CLASSOBJECT(String);
    return(newObj);
}

object newSymbol(const char* str)
{    
    object newObj;
    SObjectHandle* newObj_lock;

    /* first see if it is already there */
    newObj = globalKey(str);
    if (newObj != nilobj) 
        return newObj;

    /* not found, must make */
    newObj = allocStr(str);
    newObj_lock = new_SObjectHandle_from_object(newObj);
    newObj->_class = CLASSOBJECT(Symbol);
    nameTableInsert(symbols, strHash(str), newObj, nilobj);
    free_SObjectHandle(newObj_lock);
    return newObj;
}


object copyFrom(object obj, int start, int size)
{   
    object newObj;
    int i;

    newObj = newArray(size);
    for (i = 1; i <= size; i++) 
    {
        basicAtPut(newObj,i, basicAt(obj,start));
        start++;
    }
    return newObj;
}


void visit(Visitor* cb)
{
    int i, s;
    object *p;
    visitedObject* vo;

    if(NULL == cb->o)
        return;

#if !defined TW_SMALLINTEGER_AS_OBJECT
    if(!isInteger(cb->o))
#endif
    {
        HASH_FIND_PTR(cb->visitedObjects, &cb->o, vo);
        if(cb->o != nilobj && (vo == NULL) && ((cb->test == NULL) || cb->test(cb)))
        {
            // Cache object name, as we're going to replace it for
            // nested calls.
            object current = cb->o;

            vo = (visitedObject*)calloc(1, sizeof(visitedObject));
            vo->obj = current;
            HASH_ADD_PTR(cb->visitedObjects, obj, vo);

            if(cb->preFunc)
                cb->preFunc(cb);

            cb->o = getClass(current);
            visit(cb);
            s = current->size;
            if (s>0) 
            {
                p = current->memory;
                for (i=s; i; --i) 
                {
                    cb->o = *p++;
                    visit(cb);
                }
            }
            cb->o = current;

            if(cb->postFunc)
                cb->postFunc(cb);

            cb->count++;
        }
    }
}


/*
   imageWrite - write out an object image
   */

static void fw(FILE* fp, char* p, int s)
{
    if (fwrite(p, s, 1, fp) != 1) 
    {
        sysError("imageWrite size error","");
    }
}

void saveObject(Visitor* _this)
{
    long size;

    FileVisitor* fcb = (FileVisitor*)_this;

#if !defined TW_SMALLINTEGER_AS_OBJECT
    if(!isInteger(_this->o))
#endif
    {
        dummyObject.ref = _this->o;
        dummyObject._classRef = getClass(_this->o);
        dummyObject.size = size = getSize(_this->o);
        dummyObject.identityHash = getIdentityHash(_this->o);
        fw(fcb->fp, (char *) &dummyObject, sizeof(dummyObject));
        if (size < 0) 
            size = ((- size) + 1) / 2;
        if (size != 0)
            fw(fcb->fp, (char *) _this->o->memory, sizeof(object) * size);
    }
}


void imageWrite(FILE* fp)
{
    Visitor v;
    FileVisitor fv;

    // Write the symbols object to use during fixup.
    fw(fp, (char *) &symbols, sizeof(object));

    // Write the nil object to use during fixup.
    fw(fp, (char *) &nilobj, sizeof(object));

    // Write the next hash value, so that they hashing continues
    // from the same place when the image is loaded.
    fw(fp, (char *) &NextHashValue, sizeof(short));

    /* Write the primitive table id's */
    writePrimitiveTables(fp);

    /* First visit pass is just to get a count */
    v.preFunc = NULL;
    v.postFunc = NULL;
    v.test = NULL;
    v.o = symbols;
    v.count = 0;
    v.visitedObjects = NULL;
    visit(&v);

    fw(fp, (char *) &v.count, sizeof(v.count));
    printf("Number of objects to save: %ld\n", v.count);

    fv.super.o = symbols;
    fv.super.count = v.count;
    fv.fp = fp;
    fv.super.test = NULL;
    fv.super.preFunc = NULL;
    fv.super.postFunc = &saveObject;
    fv.super.visitedObjects = NULL;
    visit((Visitor*)&fv);
}



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

typedef struct _mapEntry
{
    object oldRef;
    object newRef;
} mapEntry;

typedef struct _linkVisitor
{
    Visitor super;

    mapEntry* map;
} LinkVisitor;

int fixupLink(mapEntry* map, long count, object* ref)
{
    long i;

    if(*ref == nilobj)
    {
        *ref = nilobj;
        return TRUE;
    }

    for(i = 0; i < count; ++i)
    {
        if(*ref == map[i].oldRef)
        {
            //printf("Fixing up %p with %p\n", *ref, map[i].newRef);
            *ref = map[i].newRef;
            return TRUE;
        }
    }
    return FALSE;
}

void relinkObject(Visitor* _this)
{
    long i;

    LinkVisitor* lv = (LinkVisitor*)_this;

    if(_this->o)
    {
        // Fixup the class reference first
        if(!fixupLink(lv->map, _this->count, &(_this->o->_class)))
            printf("No fixup found for class %p!\n", getClass(_this->o));
        // Then fixup all the object references if it's an object list object.
        if(_this->o->size > 0)
        {
            for(i = 0; i < _this->o->size; ++i)
            {
#if !defined TW_SMALLINTEGER_AS_OBJECT
                if(!isInteger(_this->o->memory[i]))
#endif
                {
                    if(!fixupLink(lv->map, _this->count, &(_this->o->memory[i])))
                        printf("No fixup found for member %p!\n", _this->o->memory[i]);
                }
            }
        }
    }
    //_this->o->referenceCount = 1;
}

void imageRead(FILE* fp)
{   
    long i, count, size;
    unsigned short identityHashHold;
    object oldRef, newRef, nilAtStore, integerAtStore;
    mapEntry* map;
    LinkVisitor lv;

    g_DisableGC = 1;

    fr(fp, (char *) &symbols, sizeof(object));
    fr(fp, (char *) &nilAtStore, sizeof(object));

    // Read the stored hash value, so that identity 
    // hash values for objects continue from the same
    // index.
    fr(fp, (char *) &NextHashValue, sizeof(short));
    // Hold the next hash value, as we don't want the
    // re-building of the snapshot to alter the starting
    // point.
    identityHashHold = NextHashValue;

    /* Read the primitive table id's */
    readPrimitiveTables(fp);

    fr(fp, (char *) &count, sizeof(long));
    // Add one for nil.
    ++count;
    i = 0;

    // Allocate a remapping table.
    map = (mapEntry*)calloc(count, sizeof(mapEntry));

    // Store the nilobj mapping
    map[i].oldRef = nilAtStore;
    map[i].newRef = nilobj;
    ++i;

    while(fr(fp, (char *) &dummyObject, sizeof(dummyObject))) 
    {
        oldRef = dummyObject.ref;
        size = dummyObject.size;

        map[i].oldRef = oldRef;
        // Allocate a new object, the size of the snapshot'd one.
        // make sure to check if it was a byte or object object.
        if (size < 0) 
        {
            newRef = allocByte(-size);
            // adjust temporary size, so we read in the right amount.
            size = ((- size) + 1) / 2;
        }
        else
            newRef = allocObject(size);

        map[i].newRef = newRef;
        ++i;

        newRef->_class = dummyObject._classRef;
        newRef->identityHash = dummyObject.identityHash;
        if (size != 0) 
            fr(fp, (char *) newRef->memory, sizeof(object) * (int) size);
        else
            newRef->memory = (object *) 0;
    }
    // Now that everything is read in, and the mapping table created, 
    // scan the objects, and fixup the addresses.

    if(!fixupLink(map, count, &symbols))
        printf("Failed to fixup symbols!\n");

    lv.super.o = symbols;
    lv.super.test = NULL;
    lv.super.preFunc = &relinkObject;
    lv.super.postFunc = NULL;
    lv.super.count = count;
    lv.super.visitedObjects = NULL;
    lv.map = map;
    visit((Visitor*)&lv);

    // Fixup the class of the single nil object
    _nilobj._class = globalSymbol("UndefinedObject");

    // Now reset the identityHash counter, to ensure we're
    // exactly like we were at the time of snapshot.
    NextHashValue = identityHashHold;

    free(map);

    g_DisableGC = 0;
}




SObjectHandle* head = 0;
SObjectHandle* tail = 0;



void appendToList(SObjectHandle* h)
{
    // Register the handle with the global list.
    if(0 == tail)
        head = tail = h;
    else
    {
        tail->m_next = h;
        h->m_prev = tail;
        tail = h;
    }
}

void removeFromList(SObjectHandle* h)
{
    // Unlink from the list pointers
    if(0 != h->m_prev)
        h->m_prev->m_next = h->m_next;

    if(0 != h->m_next)
        h->m_next->m_prev = h->m_prev;

    if(head == h)
        head = h->m_next;
    if(tail == h)
        tail = h->m_prev;
}

SObjectHandle* new_SObjectHandle()
{
    SObjectHandle* h = (SObjectHandle*)calloc(1, sizeof(SObjectHandle));
    h->m_handle = nilobj;
    h->m_next = h->m_prev = 0;

    appendToList(h);

    return h;
}


SObjectHandle* new_SObjectHandle_from_object(object from)
{
    SObjectHandle* h = new_SObjectHandle();
    h->m_handle = from;

    return h;
}


void free_SObjectHandle(SObjectHandle* h)
{
    if(h)
    {
        removeFromList(h);
        free(h);
    }
}




object* sysMemPtr(object _this)
{
    return _this->memory;
}

byte* bytePtr(object _this)
{
    return (byte *)sysMemPtr(_this);
}

char* charPtr(object _this)
{
    return (char *)sysMemPtr(_this);
}


void basicAtPut(object _this, int i, object v)
{
    if((i <= 0) || (i > _this->size))
        sysError("index out of range", "basicAtPut");
    else
        sysMemPtr(_this)[i-1] = v;
}

int byteAt(object _this, int i)
{
    byte* bp;
    unsigned char t;

    if((i <= 0) || (i > 2 * - _this->size))
        sysError("index out of range", "byteAt");
    else
    {
        bp = bytePtr(_this);
        t = bp[i-1];
        i = (int) t;
    }
    return i;
}

void byteAtPut(object _this, int i, int x)
{
    byte *bp;
    if ((i <= 0) || (i > 2 * - _this->size)) 
    {
        sysError("index out of range", "byteAtPut");
    }
    else 
    {
        bp = bytePtr(_this);
        bp[i-1] = x;
    }
}

object basicAt(object _this, int i)
{
    if(( i <= 0) || (i > _this->size))
    {
        fprintf(stderr, "Indexing: %d\n", i);
        sysError("index out of range", "basicAt");
    }
    else
        return (sysMemPtr(_this)[i-1]);

    return nilobj;
}

double floatValue(object _this)
{   
    double d;

    ncopy((char *) &d, charPtr(_this), (int) sizeof(double));
    return d;
}

int intValue(object _this)
{   
    int d;

    if(_this == nilobj)
    {
        //printf("Getting Integer from nil\n");
        return 0;
    }
    ncopy((char *) &d, charPtr(_this), (int) sizeof(int));
    return d;
}

void* cPointerValue(object _this)
{   
    void* l;
    int s;

    if(NULL == charPtr(_this))
        sysError("invalid cPointer","cPointerValue");

    s = sizeof(void*);
    ncopy((char *) &l, charPtr(_this), (int) sizeof(void*));
    return l;
}

unsigned short hashObject(object o)
{
    return o->identityHash;
}

unsigned long allocatedObjectCount()
{
    ObjectPool* pool;
    long count = 0;

    pool = g_HeadPool;
    while(pool)
    {
        count += pool->top;
        pool = pool->next;
    }
    return count;
}

unsigned long referencedObjects()
{
    Visitor v;

    v.preFunc = NULL;
    v.postFunc = NULL;
    v.test = NULL;
    v.o = symbols;
    v.count = 0;
    v.visitedObjects = NULL;
    visit(&v);

    return v.count;
}
