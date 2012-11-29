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

#if defined TW_ENABLE_FFI
extern void initFFISymbols();   /* FFI symbols */
#endif

object firstProcess;
int initial = 1;    /* making initial image */

int main(int argc, char** argv) 
{   
    int i;
    char methbuf[500];

    initialiseInterpreter();
    makeInitialImage();

    initCommonSymbols();
#if defined TW_ENABLE_FFI
    initFFISymbols();
#endif

    for (i = 1; i < argc; i++) 
    {
        snprintf(methbuf, 500, "<120 1 '%s' 'r'>. <123 1>. <121 1>", argv[i]);
        runCode(methbuf);

    }

    /* when we are all done looking at the arguments, do initialization */
    fprintf(stderr,"initialization\n");
    //debugging = TRUE;
    runCode("nil initialize\n");
    fprintf(stderr,"finished\n");

    /* exit and return - belt and suspenders, but it keeps lint happy */
    exit(0); return 0;
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
  SObjectHandle *lock_hashTable = 0;
  SObjectHandle *lock_integerClass = 0;
  SObjectHandle *lock_symbolObj = 0, *lock_symbolClass = 0, *lock_classClass = 0, *lock_metaClassClass = 0;
  object trueobj, falseobj;

  /* first create the table, without class links */
  symbols = allocObject(dictionarySize);
  hashTable = allocObject(3 * 53);
  lock_hashTable = new_SObjectHandle_from_object(hashTable);
  basicAtPut(symbols,tableInDictionary, hashTable);

  /* next create #Symbol, Symbol and Class */
  symbolObj = newSymbol("Symbol");
  lock_symbolObj = new_SObjectHandle_from_object(symbolObj);
  basicAtPut(symbols,tableInDictionary, hashTable);
  symbolClass = newClass("Symbol");
  lock_symbolClass = new_SObjectHandle_from_object(symbolClass);
  basicAtPut(symbols,tableInDictionary, hashTable);
  integerClass = newClass("Integer");
  lock_integerClass = new_SObjectHandle_from_object(integerClass);
  basicAtPut(symbols,tableInDictionary, hashTable);
  symbolObj->_class = symbolClass;
  classClass = newClass("Class");
  lock_classClass = new_SObjectHandle_from_object(classClass);
  basicAtPut(symbols,tableInDictionary, hashTable);
  symbolClass->_class = classClass;
  integerClass->_class = classClass;
  metaClassClass = newClass("MetaClass");
  lock_metaClassClass = new_SObjectHandle_from_object(metaClassClass);
  basicAtPut(symbols,tableInDictionary, hashTable);
  classClass->_class = metaClassClass;

  /* now fix up classes for symbol table */
  /* and make a couple common classes, just to hold their places */
  newClass("Link");
  newClass("ByteArray");
  hashTable->_class = newClass("Array");
  symbols->_class = newClass("Dictionary");
  nilobj->_class = newClass("UndefinedObject");
  newClass("String");
  nameTableInsert(symbols, strHash("symbols"), newSymbol("symbols"), symbols);

  basicAtPut(classClass,methodsInClass, newDictionary(39));
  basicAtPut(metaClassClass,methodsInClass, newDictionary(39));

  /* finally at least make true and false to be distinct */
  trueobj = newSymbol("true");
  nameTableInsert(symbols, strHash("true"), trueobj, trueobj);
  falseobj = newSymbol("false");
  nameTableInsert(symbols, strHash("false"), falseobj, falseobj);

  free_SObjectHandle(lock_hashTable);
  free_SObjectHandle(lock_integerClass);
  free_SObjectHandle(lock_symbolObj);
  free_SObjectHandle(lock_symbolClass);
  free_SObjectHandle(lock_classClass);
  free_SObjectHandle(lock_metaClassClass);
}
