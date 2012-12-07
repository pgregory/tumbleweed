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


int debugging = FALSE;
ObjectStruct _nilobj =
{
  NULL,
  0,0,
  0, 
  0,
  NULL
};
object nilobj = &_nilobj;

extern object firstProcess;
object symbols;     /* table of all symbols created */

static unsigned short NextHashValue = 0;


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


object allocObject(size_t memorySize)
{
    object ob = (object)calloc(1, sizeof(ObjectStruct));
    if(memorySize)
      ob->memory = mBlockAlloc(memorySize);
    else
      ob->memory = NULL;
    ob->size = memorySize;
    ob->_class = nilobj;
    ob->referenceCount = 0;
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

    newobj = allocObject(1);
    basicAtPut(newobj,1, newInteger(value));
    newobj->_class = CLASSOBJECT(Char);
    return(newobj);
}

object newClass(const char* name)
{   
    object newObj, nameObj, methTable;

    newObj = allocObject(classSize);
    newObj->_class = CLASSOBJECT(Class);

    /* now make name */
    nameObj = newSymbol(name);
    basicAtPut(newObj,nameInClass, nameObj); methTable = newDictionary(39);
    basicAtPut(newObj,methodsInClass, methTable);
    basicAtPut(newObj,sizeInClass, newInteger(classSize));

    /* now put in global symbols table */
    nameTableInsert(symbols, strHash(name), nameObj, newObj);

    return newObj;
}

object newContext(int link, object method, object args, object temp)
{   
    object newObj;

    newObj = allocObject(contextSize);
    newObj->_class = CLASSOBJECT(Context);
    basicAtPut(newObj,linkPtrInContext, newInteger(link));
    basicAtPut(newObj,methodInContext, method);
    basicAtPut(newObj,argumentsInContext, args);
    basicAtPut(newObj,temporariesInContext, temp);
    return newObj;
}

object newDictionary(int size)
{   
    object newObj;

    newObj = allocObject(dictionarySize);
    newObj->_class = CLASSOBJECT(Dictionary);
    basicAtPut(newObj,tableInDictionary, newArray(size));
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
    return (i << 1) + 1;
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

    /* first see if it is already there */
    newObj = globalKey(str);
    if (newObj != nilobj) 
        return newObj;

    /* not found, must make */
    newObj = allocStr(str);
    newObj->_class = CLASSOBJECT(Symbol);
    nameTableInsert(symbols, strHash(str), newObj, nilobj);
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

    long count;
    object o;
} Visitor;

typedef struct _fileVisitor
{
    Visitor super;

    FILE* fp;
} FileVisitor;

int testFlagZero(Visitor* _this)
{
    return _this->o->referenceCount == 0;
}

int testFlagNonZero(Visitor* _this)
{
    return _this->o->referenceCount != 0;
}

void clearFlag(Visitor* _this)
{
    _this->o->referenceCount = 0;
}

void setFlag(Visitor* _this)
{
    _this->o->referenceCount = 1;
    _this->count++;
}

void visit(Visitor* cb)
{
    int i, s;
    object *p;

    if(cb->o != nilobj && cb->test(cb))
    {
        // Cache object name, as we're going to replace it for
        // nested calls.
        object current = cb->o;

        if(cb->preFunc)
            cb->preFunc(cb);
        cb->o = current->_class;
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

  dummyObject.ref = _this->o;
  dummyObject._classRef = _this->o->_class;
  dummyObject.size = size = _this->o->size;
  dummyObject.identityHash = _this->o->identityHash;
  fw(fcb->fp, (char *) &dummyObject, sizeof(dummyObject));
  if (size < 0) 
    size = ((- size) + 1) / 2;
  if (size != 0)
    fw(fcb->fp, (char *) _this->o->memory, sizeof(object) * size);
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

  v.preFunc = &setFlag;
  v.postFunc = 0;
  v.test = &testFlagZero;
  v.o = symbols;
  v.count = 0;
  visit(&v);

  fw(fp, (char *) &v.count, sizeof(v.count));
  printf("Number of objects to save: %ld\n", v.count);

  fv.super.o = symbols;
  fv.super.count = v.count;
  fv.fp = fp;
  fv.super.test = &testFlagNonZero;
  fv.super.preFunc = &clearFlag;
  fv.super.postFunc = &saveObject;
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
      printf("No fixup found for class %p!\n", _this->o->_class);
    // Then fixup all the object references if it's an object list object.
    if(_this->o->size > 0)
    {
      for(i = 0; i < _this->o->size; ++i)
      {
        if(!fixupLink(lv->map, _this->count, &(_this->o->memory[i])))
          printf("No fixup found for member %p!\n", _this->o->memory[i]);
      }
    }
  }
  _this->o->referenceCount = 1;
}

void imageRead(FILE* fp)
{   
  long i, count, size;
  unsigned short identityHashHold;
  object oldRef, newRef, nilAtStore;
  mapEntry* map;
  LinkVisitor lv;

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
  lv.super.test = &testFlagZero;
  lv.super.preFunc = &relinkObject;
  lv.super.postFunc = 0;
  lv.super.count = count;
  lv.map = map;
  visit((Visitor*)&lv);

  // Fixup the class of the single nil object
  _nilobj._class = globalSymbol("UndefinedObject");

  // Now reset the identityHash counter, to ensure we're
  // exactly like we were at the time of snapshot.
  NextHashValue = identityHashHold;

  free(map);
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
        return 0;
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
