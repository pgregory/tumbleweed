/*
    Little Smalltalk, version 3
    Written by Tim Budd, June 1988

    initial image maker
*/
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "env.h"
#include "memory.h"
#include "names.h"
#include "interp.h"

void makeInitialImage();
void goDoIt(const char * text);

#if defined TW_ENABLE_FFI
extern void initFFISymbols();   /* FFI symbols */
#endif

ObjectHandle firstProcess;
int initial = 1;    /* making initial image */

int main(int argc, char** argv) 
{   
    int i;

    makeInitialImage();

    initCommonSymbols();
#if defined TW_ENABLE_FFI
    initFFISymbols();
#endif

    for (i = 1; i < argc; i++) {
        std::stringstream methbuf;
        //fprintf(stderr,"%s:\n", argv[i]);
        methbuf << "x <120 1 '" << argv[i] << "' 'r'>. <123 1>. <121 1>";
        goDoIt(methbuf.str().c_str());

    }

    /* when we are all done looking at the arguments, do initialization */
    fprintf(stderr,"initialization\n");
    //debugging = true;
    goDoIt("x nil initialize\n");
    fprintf(stderr,"finished\n");

    /* exit and return - belt and suspenders, but it keeps lint happy */
    exit(0); return 0;
}

void goDoIt(const char * text)
{   
    ObjectHandle stack, method;

    method = MemoryManager::Instance()->newMethod();
    setInstanceVariables(nilobj);
    parse(method, text, false);

    firstProcess = MemoryManager::Instance()->allocObject(processSize);
    stack = MemoryManager::Instance()->allocObject(50);

    /* make a process */
    firstProcess->basicAtPut(stackInProcess, stack);
    firstProcess->basicAtPut(stackTopInProcess, MemoryManager::Instance()->newInteger(10));
    firstProcess->basicAtPut(linkPtrInProcess, MemoryManager::Instance()->newInteger(2));

    /* put argument on stack */
    stack->basicAtPut(argumentInStack, nilobj);   /* argument */
    /* now make a linkage area in stack */
    stack->basicAtPut(prevlinkInStack, nilobj);   /* previous link */
    stack->basicAtPut(contextInStack, nilobj);   /* context object (nil = stack) */
    stack->basicAtPut(returnpointInStack, MemoryManager::Instance()->newInteger(1));    /* return point */
    stack->basicAtPut(methodInStack, method);   /* method */
    stack->basicAtPut(bytepointerInStack, MemoryManager::Instance()->newInteger(1));    /* byte offset */

    /* now go execute it */
    while (execute(firstProcess, 15000)) fprintf(stderr,"..");
}

/*
    there is a sort of chicken and egg problem with regards to making
    the initial image
*/
void makeInitialImage()
{   
  ObjectHandle hashTable;
  ObjectHandle integerClass;
  ObjectHandle symbolObj, symbolClass, classClass, metaClassClass;
  ObjectHandle objectClass, metaObjectClass;

  /* first create the table, without class links */
  symbols = MemoryManager::Instance()->allocObject(dictionarySize);
  hashTable = MemoryManager::Instance()->allocObject(3 * 53);
  objectRef(symbols).basicAtPut(tableInDictionary, hashTable);

  /* next create #Symbol, Symbol and Class */
  symbolObj = MemoryManager::Instance()->newSymbol("Symbol");
  symbolClass = MemoryManager::Instance()->newClass("Symbol");
  integerClass = MemoryManager::Instance()->newClass("Integer");
  symbolObj->_class = symbolClass;
  classClass = MemoryManager::Instance()->newClass("Class");
  symbolClass->_class = classClass;
  integerClass->_class = classClass;
  metaClassClass = MemoryManager::Instance()->newClass("MetaClass");
  classClass->_class = metaClassClass;

  /* now fix up classes for symbol table */
  /* and make a couple common classes, just to hold their places */
  MemoryManager::Instance()->newClass("Link");
  MemoryManager::Instance()->newClass("ByteArray");
  hashTable->_class = MemoryManager::Instance()->newClass("Array");
  objectRef(symbols)._class = MemoryManager::Instance()->newClass("Dictionary");
  objectRef(nilobj)._class = MemoryManager::Instance()->newClass("UndefinedObject");
  MemoryManager::Instance()->newClass("String");
  nameTableInsert(symbols, strHash("symbols"), MemoryManager::Instance()->newSymbol("symbols"), symbols);

  classClass->basicAtPut(methodsInClass, MemoryManager::Instance()->newDictionary(39));
  metaClassClass->basicAtPut(methodsInClass, MemoryManager::Instance()->newDictionary(39));

  /* finally at least make true and false to be distinct */
  ObjectHandle trueobj = MemoryManager::Instance()->newSymbol("true");
  nameTableInsert(symbols, strHash("true"), trueobj, trueobj);
  ObjectHandle falseobj = MemoryManager::Instance()->newSymbol("false");
  nameTableInsert(symbols, strHash("false"), falseobj, falseobj);
}
