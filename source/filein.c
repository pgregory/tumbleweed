/*
    Little Smalltalk, version 3
    Written by Tim Budd, Oregon State University, June 1988

    routines used in reading in textual descriptions of classes
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lex.h"
#include "interp.h"
#include "env.h"
#include "memory.h"
#include "parser.h"
#include "names.h"

# define MethodTableSize 39

/*
    the following are switch settings, with default values
*/
int savetext = TRUE;

/*
    we read the input a line at a time, putting lines into the following
    buffer.  In addition, all methods must also fit into this buffer.
*/
# define TextBufferSize 16384
static char* textBuffer = NULL;

/*
    findClass gets a class object,
    either by finding it already or making it
    in addition, it makes sure it has a size, by setting
    the size to zero if it is nil.
*/
static object findClass(const char* name)
{   
    object newobj;
    SObjectHandle* lock;

    newobj = globalSymbol(name);
    if (newobj == nilobj)
        newobj = newClass(name);
    if (basicAt(newobj,sizeInClass) == nilobj) 
    {
        lock = new_SObjectHandle_from_object(newobj);
        basicAtPut(newobj,sizeInClass, newInteger(0));
        free_SObjectHandle(lock);
    }
    return newobj;
}

static object findClassWithMeta(const char* name, object metaObj)
{   
    object newObj, nameObj, methTable;
    SObjectHandle *lock_newObj, *lock_nameObj, *lock_methTable, *lock_metaObj;
    int size;

    newObj = globalSymbol(name);
    if (newObj == nilobj)
    {
        lock_metaObj = new_SObjectHandle_from_object(metaObj);

        size = getInteger(basicAt(metaObj,sizeInClass));
        newObj = allocObject(size);
        lock_newObj = new_SObjectHandle_from_object(newObj);
        newObj->_class = metaObj;

        /* now make name */
        nameObj = newSymbol(name);
        lock_nameObj = new_SObjectHandle_from_object(nameObj);
        basicAtPut(newObj,nameInClass, nameObj);
        methTable = newDictionary(39);
        lock_methTable = new_SObjectHandle_from_object(methTable);
        basicAtPut(newObj,methodsInClass, methTable);
        basicAtPut(newObj,sizeInClass, newInteger(size));

        /* now put in global symbols table */
        nameTableInsert(symbols, strHash(name), nameObj, newObj);

        free_SObjectHandle(lock_methTable);
        free_SObjectHandle(lock_nameObj);
        free_SObjectHandle(lock_newObj);
        free_SObjectHandle(lock_metaObj);
    }
    return newObj;
}

/*
 * Create raw class
 */

static object createRawClass(const char* _class, const char* metaclass, const char* superclass)
{
    object classObj, superObj, metaObj;
    int size;

    metaObj = findClass(metaclass);
    classObj = findClassWithMeta(_class, metaObj);
    classObj->_class = metaObj;

    size = 0;
    superObj = nilobj;
    if(NULL != superclass)
    {
        superObj = findClass(superclass);
        basicAtPut(classObj,superClassInClass, superObj);
        size = getInteger(basicAt(superObj,sizeInClass));
    }
    return classObj;
}

/*
    readRawDeclaration reads a declaration of a class
*/
static void readRawClassDeclaration()
{   
    object classObj, vars;
    SObjectHandle *lock_classObj, *lock_vars;
    char className[100], metaName[100], superName[100];
    int i, size, instanceTop;
    // todo: fixed length variables array!
    object instanceVariables[15];
    SObjectHandle* lock_instanceVariables[15];

    if (nextToken() != nameconst)
        sysError("bad file format","no name in declaration");
    strncpy(className, strToken(), 100);
    size = 0;
    if (nextToken() == nameconst) 
    { /* read metaclass name */
        strncpy(metaName, strToken(), 100);
        nextToken();
    }
    if (currentToken() == nameconst) 
    { /* read superclass name */
        strncpy(superName, strToken(), 100);
        nextToken();
    }

    classObj = createRawClass(className, metaName, superName);
    lock_classObj = new_SObjectHandle_from_object(classObj);

    // Get the current class size, we'll build on this as 
    // we add instance variables.
    size = getInteger(basicAt(classObj,sizeInClass));

    if (currentToken() == nameconst) 
    {     /* read instance var names */
        instanceTop = 0;
        while (currentToken() == nameconst) 
        {
            instanceVariables[instanceTop] = newSymbol(strToken());
            lock_instanceVariables[instanceTop] = new_SObjectHandle_from_object(instanceVariables[instanceTop++]);
            size++;
            nextToken();
        }
        vars = newArray(instanceTop);
        lock_vars = new_SObjectHandle_from_object(vars);
        for (i = 0; i < instanceTop; i++) 
        {
            basicAtPut(vars,i+1, instanceVariables[i]);
        }
        basicAtPut(classObj,variablesInClass, vars);
    }
    basicAtPut(classObj,sizeInClass, newInteger(size));
    basicAtPut(classObj,methodsInClass, newDictionary(39));

    free_SObjectHandle(lock_vars);
    while(--instanceTop >= 0)
        free_SObjectHandle(lock_instanceVariables[instanceTop]);
    free_SObjectHandle(lock_classObj);
}

/*
    readDeclaration reads a declaration of a class
*/
static void readClassDeclaration()
{   
    object classObj, metaObj, vars;
    SObjectHandle *lock_classObj, *lock_metaObj, *lock_vars = 0;
    char className[100], superName[100];
    int i, size, instanceTop = 0;
    // todo: fixed length variables array!
    object instanceVariables[15];
    SObjectHandle* lock_instanceVariables[15];
    // todo: horrible fixed length arrays!
    char metaClassName[100];
    char metaSuperClassName[100];

    superName[0] = '\0';

    if (nextToken() != nameconst)
        sysError("bad file format","no name in declaration");
    strncpy(className, strToken(), 100);
    if (nextToken() == nameconst) 
    { /* read superclass name */
        strncpy(superName, strToken(), 100);
        nextToken();
    }
    // todo: sprintf eradication!
    sprintf(metaClassName, "Meta%s", className);
    if(strlen(superName) != 0)
        sprintf(metaSuperClassName, "Meta%s", superName);
    else
        sprintf(metaSuperClassName, "Class");

    metaObj = createRawClass(metaClassName, "Class", metaSuperClassName);
    lock_metaObj = new_SObjectHandle_from_object(metaObj);
    classObj = createRawClass(className, metaClassName, superName);
    lock_classObj = new_SObjectHandle_from_object(classObj);
    classObj->_class = metaObj;

    // Get the current class size, we'll build on this as 
    // we add instance variables.
    size = getInteger(basicAt(classObj,sizeInClass));

    if (currentToken() == nameconst) 
    {     /* read instance var names */
        instanceTop = 0;
        while (currentToken() == nameconst) 
        {
            instanceVariables[instanceTop] = newSymbol(strToken());
            lock_instanceVariables[instanceTop] = new_SObjectHandle_from_object(instanceVariables[instanceTop++]);
            size++;
            nextToken();
        }
        vars = newArray(instanceTop);
        lock_vars = new_SObjectHandle_from_object(vars);
        for (i = 0; i < instanceTop; i++) 
        {
            basicAtPut(vars,i+1, instanceVariables[i]);
        }
        basicAtPut(classObj,variablesInClass, vars);
    }
    basicAtPut(classObj,sizeInClass, newInteger(size));
    basicAtPut(classObj,methodsInClass, newDictionary(39));

    free_SObjectHandle(lock_vars);
    while(--instanceTop >= 0)
        free_SObjectHandle(lock_instanceVariables[instanceTop]);
    free_SObjectHandle(lock_classObj);
    free_SObjectHandle(lock_metaObj);
}

/*
    readClass reads a class method description
*/
static void readMethods(FILE* fd, int printit)
{   
    object classObj, methTable, theMethod, selector;
    SObjectHandle *lock_classObj = 0, *lock_methTable = 0, *lock_theMethod = 0, *lock_selector = 0;
# define LINEBUFFERSIZE 16384
    char *cp, *eoftest, lineBuffer[LINEBUFFERSIZE];
    object protocol = 0;
    SObjectHandle *lock_protocol = 0;

    lineBuffer[0] = '\0';
    protocol = nilobj;
    if (nextToken() != nameconst)
        sysError("missing name","following Method keyword");
    classObj = findClass(strToken());
    lock_classObj = new_SObjectHandle_from_object(classObj);

    setInstanceVariables(classObj);
    if (printit)
        cp = charPtr(basicAt(classObj,nameInClass));

    /* now find or create a method table */
    methTable = basicAt(classObj,methodsInClass);
    if (methTable == nilobj) 
    {  /* must make */
        methTable = newDictionary(MethodTableSize);
        basicAtPut(classObj,methodsInClass, methTable);
    }
    lock_methTable = new_SObjectHandle_from_object(methTable);

    if(nextToken() == strconst) 
    {
        protocol = newStString(strToken());
        lock_protocol = new_SObjectHandle_from_object(protocol);
    }

    /* now go read the methods */
    do 
    {
        if (lineBuffer[0] == '|')   /* get any left over text */
            strcpy(textBuffer,&lineBuffer[1]);
        else
            textBuffer[0] = '\0';
        while((eoftest = fgets(lineBuffer, LINEBUFFERSIZE, fd)) != NULL) {
            if ((lineBuffer[0] == '|') || (lineBuffer[0] == ']'))
                break;
            strcat(textBuffer, lineBuffer);
        }
        if (eoftest == NULL) {
            sysError("unexpected end of file","while reading method");
            break;
        }

        /* now we have a method */
        theMethod = newMethod();
        lock_theMethod = new_SObjectHandle_from_object(theMethod);
        resetLexer(textBuffer);
        if (parseMessageHandler(theMethod, savetext)) {
            selector = basicAt(theMethod,messageInMethod);
            lock_selector = new_SObjectHandle_from_object(selector);
            basicAtPut(theMethod,methodClassInMethod, classObj);
            basicAtPut(theMethod,protocolInMethod, protocol);
            if (printit)
                dspMethod(cp, charPtr(selector));
            nameTableInsert(methTable, hashObject(selector), selector, theMethod);
        }
        else {
            /* get rid of unwanted method */
            givepause();
        }

    } while (lineBuffer[0] != ']');

    free_SObjectHandle(lock_protocol);
    free_SObjectHandle(lock_theMethod);
    free_SObjectHandle(lock_methTable);
    free_SObjectHandle(lock_classObj);
}


/*
    fileIn reads in a module definition
*/
void fileIn(FILE* fd, int printit)
{
    textBuffer = (char*)calloc(TextBufferSize, sizeof(char));
    while(fgets(textBuffer, TextBufferSize, fd) != NULL) 
    {
        resetLexer(textBuffer);
        if (currentToken() == inputend)
            ; /* do nothing, get next line */
        else if ((currentToken() == binary) && strcmp(strToken(), "*") == 0)
            ; /* do nothing, its a comment */
        else if ((currentToken() == nameconst) && strcmp(strToken(), "RawClass") == 0)
            readRawClassDeclaration();
        else if ((currentToken() == nameconst) && strcmp(strToken(), "Class") == 0)
            readClassDeclaration();
        else if ((currentToken() == nameconst) && strcmp(strToken(), "Methods") == 0)
            readMethods(fd, printit);
        else 
            sysError("unrecognized line", textBuffer);
        //garbageCollect();
    }
    free(textBuffer);
}
