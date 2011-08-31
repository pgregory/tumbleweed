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

extern noreturn initFFISymbols();   /* FFI symbols */

object firstProcess;
int initial = 1;    /* making initial image */

/* lightspeed C not using argc/argv features */
# ifdef NOARGC
char *argv[] = {"initial", "basic.st","mag.st","collect.st", "file.st",
        "mult.st", 
# ifdef STDWIN
        "graphics.st", "stdwin.st", 0};
int argc = 8;
# endif
# ifndef STDWIN
        "tty.st", 0};
int argc = 7;
# endif

main()
# endif
# ifndef NOARGC
main(argc, argv) 
int argc;
char **argv;
# endif
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

 goDoIt(text)
char *text;
{   
  object stack, method;

    method = newMethod();
    incr(method);
    setInstanceVariables(nilobj);
    ignore parse(method, text, false);

    firstProcess = allocObject(processSize);
    incr(firstProcess);
    stack = allocObject(50);
    incr(stack);

    /* make a process */
    basicAtPut(firstProcess, stackInProcess, stack);
    basicAtPut(firstProcess, stackTopInProcess, newInteger(10));
    basicAtPut(firstProcess, linkPtrInProcess, newInteger(2));

    /* put argument on stack */
    basicAtPut(stack, 1, nilobj);   /* argument */
    /* now make a linkage area in stack */
    basicAtPut(stack, 2, nilobj);   /* previous link */
    basicAtPut(stack, 3, nilobj);   /* context object (nil = stack) */
    basicAtPut(stack, 4, newInteger(1));    /* return point */
    basicAtPut(stack, 5, method);   /* method */
    basicAtPut(stack, 6, newInteger(1));    /* byte offset */

    /* now go execute it */
    while (execute(firstProcess, 15000)) fprintf(stderr,"..");
}

/*
    there is a sort of chicken and egg problem with regards to making
    the initial image
*/
 makeInitialImage()
{   object hashTable;
    object integerClass;
    object symbolObj, symbolClass, classClass, metaClassClass;
  object objectClass, metaObjectClass;

    /* first create the table, without class links */
    symbols = allocObject(1);
    incr(symbols);
    hashTable = allocObject(3 * 53);
    basicAtPut(symbols, 1, hashTable);

    /* next create #Symbol, Symbol and Class */
    symbolObj = newSymbol("Symbol");
    symbolClass = newClass("Symbol");
    integerClass = newClass("Integer");
    setClass(symbolObj, symbolClass);
    classClass = newClass("Class");
    setClass(symbolClass, classClass);
    setClass(integerClass, classClass);
  metaClassClass = newClass("MetaClass");
    setClass(classClass, metaClassClass);
  objectClass = newClass("Object");
  metaObjectClass = newClass("MetaObject");
  setClass(objectClass, metaObjectClass);
  setClass(metaObjectClass, classClass);

  basicAtPut(metaClassClass, superClassInClass, metaObjectClass);
  basicAtPut(metaObjectClass, superClassInClass, classClass);
    basicAtPut(metaObjectClass, sizeInClass, newInteger(classSize));

    /* now fix up classes for symbol table */
    /* and make a couple common classes, just to hold their places */
    ignore newClass("Link");
    ignore newClass("ByteArray");
    setClass(hashTable, newClass("Array"));
    setClass(symbols, newClass("Dictionary"));
    setClass(nilobj, newClass("UndefinedObject"));
    ignore newClass("String");
    nameTableInsert(symbols, strHash("symbols"), newSymbol("symbols"), symbols);

  basicAtPut(classClass, methodsInClass, newDictionary(39));

    /* finally at least make true and false to be distinct */
    trueobj = newSymbol("true");
    nameTableInsert(symbols, strHash("true"), trueobj, trueobj);
    falseobj = newSymbol("false");
    nameTableInsert(symbols, strHash("false"), falseobj, falseobj);
}
