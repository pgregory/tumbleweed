/*
    Little Smalltalk, version 3
    Written by Tim Budd, June 1988

    initial image maker
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "env.h"
#include "memory.h"
#include "names.h"
#include "interp.h"

void makeInitialImage();
void goDoIt(const char * text);

extern noreturn initFFISymbols();   /* FFI symbols */

object firstProcess;
int initial = 1;    /* making initial image */

int main(int argc, char** argv) 
{   
  char methbuf[MAX_PATH];
    int i;

    initMemoryManager();

    makeInitialImage();

    initCommonSymbols();
  initFFISymbols();

    for (i = 1; i < argc; i++) {
        //fprintf(stderr,"%s:\n", argv[i]);
        ignore sprintf(methbuf, 
            "x <120 1 '%s' 'r'>. <123 1>. <121 1>", 
                argv[i]);
        goDoIt(methbuf);

        }

    /* when we are all done looking at the arguments, do initialization */
    fprintf(stderr,"initialization\n");
    /*debugging = true;*/
    goDoIt("x nil initialize\n");
    fprintf(stderr,"finished\n");

    /* exit and return - belt and suspenders, but it keeps lint happy */
    exit(0); return 0;
}

void goDoIt(const char * text)
{   
  object stack, method;

    method = newMethod();
    setInstanceVariables(nilobj);
    ignore parse(method, text, false);

    firstProcess = theMemoryManager->allocObject(processSize);
    stack = theMemoryManager->allocObject(50);

    /* make a process */
    objectRef(firstProcess).basicAtPut(stackInProcess, stack);
    objectRef(firstProcess).basicAtPut(stackTopInProcess, newInteger(10));
    objectRef(firstProcess).basicAtPut(linkPtrInProcess, newInteger(2));

    /* put argument on stack */
    objectRef(stack).basicAtPut(1, nilobj);   /* argument */
    /* now make a linkage area in stack */
    objectRef(stack).basicAtPut(2, nilobj);   /* previous link */
    objectRef(stack).basicAtPut(3, nilobj);   /* context object (nil = stack) */
    objectRef(stack).basicAtPut(4, newInteger(1));    /* return point */
    objectRef(stack).basicAtPut(5, method);   /* method */
    objectRef(stack).basicAtPut(6, newInteger(1));    /* byte offset */

    /* now go execute it */
    while (execute(firstProcess, 15000)) fprintf(stderr,"..");
}

/*
    there is a sort of chicken and egg problem with regards to making
    the initial image
*/
void makeInitialImage()
{   
    object hashTable;
    object integerClass;
    object symbolObj, symbolClass, classClass, metaClassClass;
  object objectClass, metaObjectClass;

    /* first create the table, without class links */
    symbols = theMemoryManager->allocObject(1);
    hashTable = theMemoryManager->allocObject(3 * 53);
    objectRef(symbols).basicAtPut(1, hashTable);

    /* next create #Symbol, Symbol and Class */
    symbolObj = newSymbol("Symbol");
    symbolClass = newClass("Symbol");
    integerClass = newClass("Integer");
    objectRef(symbolObj).setClass(symbolClass);
    classClass = newClass("Class");
    objectRef(symbolClass).setClass(classClass);
    objectRef(integerClass).setClass(classClass);
  metaClassClass = newClass("MetaClass");
    objectRef(classClass).setClass(metaClassClass);
  objectClass = newClass("Object");
  metaObjectClass = newClass("MetaObject");
  objectRef(objectClass).setClass(metaObjectClass);
  objectRef(metaObjectClass).setClass(classClass);

  objectRef(metaClassClass).basicAtPut(superClassInClass, metaObjectClass);
  objectRef(metaObjectClass).basicAtPut(superClassInClass, classClass);
    objectRef(metaObjectClass).basicAtPut(sizeInClass, newInteger(classSize));

    /* now fix up classes for symbol table */
    /* and make a couple common classes, just to hold their places */
    ignore newClass("Link");
    ignore newClass("ByteArray");
    objectRef(hashTable).setClass(newClass("Array"));
    objectRef(symbols).setClass(newClass("Dictionary"));
    objectRef(nilobj).setClass(newClass("UndefinedObject"));
    ignore newClass("String");
    nameTableInsert(symbols, strHash("symbols"), newSymbol("symbols"), symbols);

  objectRef(classClass).basicAtPut(methodsInClass, newDictionary(39));

    /* finally at least make true and false to be distinct */
    trueobj = newSymbol("true");
    nameTableInsert(symbols, strHash("true"), trueobj, trueobj);
    falseobj = newSymbol("false");
    nameTableInsert(symbols, strHash("false"), falseobj, falseobj);
}
