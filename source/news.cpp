/*
    little smalltalk, version 3.1
    written by tim budd, July 1988

    new object creation routines
    built on top of memory allocation, these routines
    handle the creation of various kinds of objects
*/

# include <stdio.h>
# include "env.h"
# include "memory.h"
# include "names.h"

static object arrayClass = nilobj;  /* the class Array */
static object intClass = nilobj;    /* the class Integer */
static object stringClass = nilobj; /* the class String */
static object symbolClass = nilobj; /* the class Symbol */

void ncopy(register char* p, register char* q, register int n)      /* ncopy - copy exactly n bytes from place to place */
{   
    for (; n>0; n--)
        *p++ = *q++;
}

object getClass(register object obj)    /* getClass - get the class of an object */
{
    return objectRef(obj).classField();
}

object newArray(int size)
{   
    object newObj;

    newObj = allocObject(size);
    if (arrayClass == nilobj)
        arrayClass = globalSymbol("Array");
    objectRef(newObj).setClass(arrayClass);
    return newObj;
}

object newBlock()
{   
    object newObj;

    newObj = allocObject(blockSize);
    objectRef(newObj).setClass(globalSymbol("Block"));
    return newObj;
}

object newByteArray(int size)
{   
    object newobj;

    newobj = allocByte(size);
    objectRef(newobj).setClass(globalSymbol("ByteArray"));
    return newobj;
}

object newChar(int value)
{   
    object newobj;

    newobj = allocObject(1);
    basicAtPut(newobj, 1, newInteger(value));
    objectRef(newobj).setClass(globalSymbol("Char"));
    return(newobj);
}

object newClass(const char* name)
{   
  object newObj, nameObj, methTable;

    newObj = allocObject(classSize);
    objectRef(newObj).setClass(globalSymbol("Class"));

    /* now make name */
    nameObj = newSymbol(name);
    basicAtPut(newObj, nameInClass, nameObj);
  methTable = newDictionary(39);
  basicAtPut(newObj, methodsInClass, methTable);
  basicAtPut(newObj, sizeInClass, newInteger(classSize));

    /* now put in global symbols table */
    nameTableInsert(symbols, strHash(name), nameObj, newObj);

    return newObj;
}

object copyFrom(object obj, int start, int size)
{   
    object newObj;
    int i;

    newObj = newArray(size);
    for (i = 1; i <= size; i++) {
        basicAtPut(newObj, i, basicAt(obj, start));
        start++;
        }
    return newObj;
}

object newContext(int link, object method, object args, object temp)
{   
    object newObj;

    newObj = allocObject(contextSize);
    objectRef(newObj).setClass(globalSymbol("Context"));
    basicAtPut(newObj, linkPtrInContext, newInteger(link));
    basicAtPut(newObj, methodInContext, method);
    basicAtPut(newObj, argumentsInContext, args);
    basicAtPut(newObj, temporariesInContext, temp);
    return newObj;
}

object newDictionary(int size)
{   
    object newObj;

    newObj = allocObject(1);
    objectRef(newObj).setClass(globalSymbol("Dictionary"));
    basicAtPut(newObj, 1, newArray(size));
    return newObj;
}

object newFloat(double d)
{   
    object newObj;

    newObj = allocByte((int) sizeof (double));
    ncopy(objectRef(newObj).charPtr(), (char *) &d, (int) sizeof (double));
    objectRef(newObj).setClass(globalSymbol("Float"));
    return newObj;
}

object newInteger(int i)
{   
    object newObj;

    newObj = allocByte((int) sizeof (int));
    ncopy(objectRef(newObj).charPtr(), (char *) &i, (int) sizeof (int));
    objectRef(newObj).setClass(globalSymbol("Integer"));
    return newObj;
}

double floatValue(object o)
{   
    double d;

    ncopy((char *) &d, objectRef(o).charPtr(), (int) sizeof(double));
    return d;
}

int intValue(object o)
{   
    int d;

  if(o == nilobj)
    return 0;
    ncopy((char *) &d, objectRef(o).charPtr(), (int) sizeof(int));
    return d;
}

object newCPointer(void* l)
{   
  object newObj;

  int s = sizeof(void*);
    newObj = allocByte((int) sizeof (void*));
    ncopy(objectRef(newObj).charPtr(), (char *) &l, (int) sizeof (void*));
    objectRef(newObj).setClass(globalSymbol("CPointer"));
    return newObj;
}

void* cPointerValue(object o)
{   
  if(NULL == objectRef(o).charPtr())
    sysError("invalid cPointer","cPointerValue");

  void* l;

  int s = sizeof(void*);
    ncopy((char *) &l, objectRef(o).charPtr(), (int) sizeof(void*));
    return l;
}

object newLink(object key, object value)
{   
    object newObj;

    newObj = allocObject(3);
    objectRef(newObj).setClass(globalSymbol("Link"));
    basicAtPut(newObj, 1, key);
    basicAtPut(newObj, 2, value);
    return newObj;
}

object newMethod()
{   object newObj;

    newObj = allocObject(methodSize);
    objectRef(newObj).setClass(globalSymbol("Method"));
    return newObj;
}

object newStString(const char* value)
{   
  object newObj;

    newObj = allocStr(value);
    if (stringClass == nilobj)
        stringClass = globalSymbol("String");
    objectRef(newObj).setClass(stringClass);
    return(newObj);
}

object newSymbol(const char* str)
{    
    object newObj;

    /* first see if it is already there */
    newObj = globalKey(str);
    if (newObj) 
        return newObj;

    /* not found, must make */
    newObj = allocStr(str);
    if (symbolClass == nilobj)
        symbolClass = globalSymbol("Symbol");
    objectRef(newObj).setClass(symbolClass);
    nameTableInsert(symbols, strHash(str), newObj, nilobj);
    return newObj;
}
