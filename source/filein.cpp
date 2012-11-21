/*
    Little Smalltalk, version 3
    Written by Tim Budd, Oregon State University, June 1988

    routines used in reading in textual descriptions of classes
*/


#include <sstream>

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
bool savetext = true;

/*
    we read the input a line at a time, putting lines into the following
    buffer.  In addition, all methods must also fit into this buffer.
*/
# define TextBufferSize 16384
static char* textBuffer = NULL;
static Lexer ll;

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
    if (newobj->basicAt(sizeInClass) == nilobj) 
    {
        lock = new_SObjectHandle_from_object(newobj);
        newobj->basicAtPut(sizeInClass, newInteger(0));
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

        size = getInteger(metaObj->basicAt(sizeInClass));
        newObj = allocObject(size);
        lock_newObj = new_SObjectHandle_from_object(newObj);
        newObj->_class = metaObj;

        /* now make name */
        nameObj = newSymbol(name);
        lock_nameObj = new_SObjectHandle_from_object(nameObj);
        newObj->basicAtPut(nameInClass, nameObj);
        methTable = newDictionary(39);
        lock_methTable = new_SObjectHandle_from_object(methTable);
        newObj->basicAtPut(methodsInClass, methTable);
        newObj->basicAtPut(sizeInClass, newInteger(size));

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
        classObj->basicAtPut(superClassInClass, superObj);
        size = getInteger(superObj->basicAt(sizeInClass));
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
    std::string className, metaName, superName;
    int i, size, instanceTop;
    // todo: fixed length variables array!
    object instanceVariables[15];
    SObjectHandle* lock_instanceVariables[15];

    if (ll.nextToken() != nameconst)
        sysError("bad file format","no name in declaration");
    className = ll.strToken();
    size = 0;
    if (ll.nextToken() == nameconst) 
    { /* read metaclass name */
        metaName = ll.strToken();
        ll.nextToken();
    }
    if (ll.currentToken() == nameconst) 
    { /* read superclass name */
        superName = ll.strToken();
        ll.nextToken();
    }

    classObj = createRawClass(className.c_str(), metaName.c_str(), superName.c_str());
    lock_classObj = new_SObjectHandle_from_object(classObj);

    // Get the current class size, we'll build on this as 
    // we add instance variables.
    size = getInteger(classObj->basicAt(sizeInClass));

    if (ll.currentToken() == nameconst) 
    {     /* read instance var names */
        instanceTop = 0;
        while (ll.currentToken() == nameconst) 
        {
            instanceVariables[instanceTop] = newSymbol(ll.strToken().c_str());
            lock_instanceVariables[instanceTop] = new_SObjectHandle_from_object(instanceVariables[instanceTop++]);
            size++;
            ll.nextToken();
        }
        vars = newArray(instanceTop);
        lock_vars = new_SObjectHandle_from_object(vars);
        for (i = 0; i < instanceTop; i++) 
        {
            vars->basicAtPut(i+1, instanceVariables[i]);
        }
        classObj->basicAtPut(variablesInClass, vars);
    }
    classObj->basicAtPut(sizeInClass, newInteger(size));
    classObj->basicAtPut(methodsInClass, newDictionary(39));

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
    std::string className, superName;
    int i, size, instanceTop = 0;
    // todo: fixed length variables array!
    object instanceVariables[15];
    SObjectHandle* lock_instanceVariables[15];
    // todo: horrible fixed length arrays!
    char metaClassName[100];
    char metaSuperClassName[100];

    if (ll.nextToken() != nameconst)
        sysError("bad file format","no name in declaration");
    className = ll.strToken();
    if (ll.nextToken() == nameconst) 
    { /* read superclass name */
        superName = ll.strToken();
        ll.nextToken();
    }
    // todo: sprintf eradication!
    sprintf(metaClassName, "Meta%s", className.c_str());
    if(!superName.empty())
        sprintf(metaSuperClassName, "Meta%s", superName.c_str());
    else
        sprintf(metaSuperClassName, "Class");

    metaObj = createRawClass(metaClassName, "Class", metaSuperClassName);
    lock_metaObj = new_SObjectHandle_from_object(metaObj);
    classObj = createRawClass(className.c_str(), metaClassName, superName.c_str());
    lock_classObj = new_SObjectHandle_from_object(classObj);
    classObj->_class = metaObj;

    // Get the current class size, we'll build on this as 
    // we add instance variables.
    size = getInteger(classObj->basicAt(sizeInClass));

    if (ll.currentToken() == nameconst) 
    {     /* read instance var names */
        instanceTop = 0;
        while (ll.currentToken() == nameconst) 
        {
            instanceVariables[instanceTop] = newSymbol(ll.strToken().c_str());
            lock_instanceVariables[instanceTop] = new_SObjectHandle_from_object(instanceVariables[instanceTop++]);
            size++;
            ll.nextToken();
        }
        vars = newArray(instanceTop);
        lock_vars = new_SObjectHandle_from_object(vars);
        for (i = 0; i < instanceTop; i++) 
        {
            vars->basicAtPut(i+1, instanceVariables[i]);
        }
        classObj->basicAtPut(variablesInClass, vars);
    }
    classObj->basicAtPut(sizeInClass, newInteger(size));
    classObj->basicAtPut(methodsInClass, newDictionary(39));

    free_SObjectHandle(lock_vars);
    while(--instanceTop >= 0)
        free_SObjectHandle(lock_instanceVariables[instanceTop]);
    free_SObjectHandle(lock_classObj);
    free_SObjectHandle(lock_metaObj);
}

/*
    readClass reads a class method description
*/
static void readMethods(FILE* fd, bool printit)
{   
    object classObj, methTable, theMethod, selector;
    SObjectHandle *lock_classObj = 0, *lock_methTable = 0, *lock_theMethod = 0, *lock_selector = 0;
# define LINEBUFFERSIZE 16384
    char *cp, *eoftest, lineBuffer[LINEBUFFERSIZE];
    object protocol = 0;
    SObjectHandle *lock_protocol = 0;
    Parser pp;

    lineBuffer[0] = '\0';
    protocol = nilobj;
    if (ll.nextToken() != nameconst)
        sysError("missing name","following Method keyword");
    classObj = findClass(ll.strToken().c_str());
    lock_classObj = new_SObjectHandle_from_object(classObj);

    pp.setInstanceVariables(classObj);
    if (printit)
        cp = classObj->basicAt(nameInClass)->charPtr();

    /* now find or create a method table */
    methTable = classObj->basicAt(methodsInClass);
    if (methTable == nilobj) 
    {  /* must make */
        methTable = newDictionary(MethodTableSize);
        classObj->basicAtPut(methodsInClass, methTable);
    }
    lock_methTable = new_SObjectHandle_from_object(methTable);

    if(ll.nextToken() == strconst) 
    {
        protocol = newStString(ll.strToken().c_str());
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
        pp.setLexer(Lexer(textBuffer));
        if (pp.parseMessageHandler(theMethod, savetext)) {
            selector = theMethod->basicAt(messageInMethod);
            lock_selector = new_SObjectHandle_from_object(selector);
            theMethod->basicAtPut(methodClassInMethod, classObj);
            theMethod->basicAtPut(protocolInMethod, protocol);
            if (printit)
                dspMethod(cp, selector->charPtr());
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
void fileIn(FILE* fd, bool printit)
{
    textBuffer = new char[TextBufferSize];
    while(fgets(textBuffer, TextBufferSize, fd) != NULL) 
    {
        ll.reset(textBuffer);
        if (ll.currentToken() == inputend)
            ; /* do nothing, get next line */
        else if ((ll.currentToken() == binary) && ll.strToken().compare("*") == 0)
            ; /* do nothing, its a comment */
        else if ((ll.currentToken() == nameconst) && ll.strToken().compare("RawClass") == 0)
            readRawClassDeclaration();
        else if ((ll.currentToken() == nameconst) && ll.strToken().compare("Class") == 0)
            readClassDeclaration();
        else if ((ll.currentToken() == nameconst) && ll.strToken().compare("Methods") == 0)
            readMethods(fd, printit);
        else 
            sysError("unrecognized line", textBuffer);
        //garbageCollect();
    }
    delete[](textBuffer);
}
