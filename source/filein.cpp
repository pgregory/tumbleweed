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
static char textBuffer[TextBufferSize];

/*
    findClass gets a class object,
    either by finding it already or making it
    in addition, it makes sure it has a size, by setting
    the size to zero if it is nil.
*/
static ObjectHandle findClass(const char* name)
{   
    ObjectHandle newobj;

    newobj = globalSymbol(name);
    if (newobj == nilobj)
        newobj = MemoryManager::Instance()->newClass(name);
    if (newobj->basicAt(sizeInClass) == nilobj) 
    {
        newobj->basicAtPut(sizeInClass, MemoryManager::Instance()->newInteger(0));
    }
    return newobj;
}

static ObjectHandle findClassWithMeta(const char* name, ObjectHandle metaObj)
{   
    ObjectHandle newObj, nameObj, methTable;
    int size;

    newObj = globalSymbol(name);
    if (newObj == nilobj)
    {
        size = getInteger(metaObj->basicAt(sizeInClass));
        newObj = MemoryManager::Instance()->allocObject(size);
        newObj->_class = metaObj;

        /* now make name */
        nameObj = MemoryManager::Instance()->newSymbol(name);
        newObj->basicAtPut(nameInClass, nameObj);
        methTable = MemoryManager::Instance()->newDictionary(39);
        newObj->basicAtPut(methodsInClass, methTable);
        newObj->basicAtPut(sizeInClass, MemoryManager::Instance()->newInteger(size));

        /* now put in global symbols table */
        nameTableInsert(symbols, strHash(name), nameObj, newObj);
    }
    return newObj;
}

/*
 * Create raw class
 */

static ObjectHandle createRawClass(const char* _class, const char* metaclass, const char* superclass)
{
    ObjectHandle classObj, superObj, metaObj;
    int size;

    metaObj = findClass(metaclass);
    classObj = findClassWithMeta(_class, metaObj);
    classObj->_class = metaObj;

    //printf("RAWCLASS %s %s %s\n", class, metaclass, superclass);

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
    ObjectHandle classObj, vars;
    std::string className, metaName, superName;
    int i, size, instanceTop;
    // todo: fixed length variables array!
    ObjectHandle instanceVariables[15];

    if (nextToken() != nameconst)
        sysError("bad file format","no name in declaration");
    className = tokenString;
    size = 0;
    if (nextToken() == nameconst) 
    { /* read metaclass name */
        metaName = tokenString;
        nextToken();
    }
    if (token == nameconst) 
    { /* read superclass name */
        superName = tokenString;
        nextToken();
    }

    classObj = createRawClass(className.c_str(), metaName.c_str(), superName.c_str());

    // Get the current class size, we'll build on this as 
    // we add instance variables.
    size = getInteger(classObj->basicAt(sizeInClass));

    if (token == nameconst) 
    {     /* read instance var names */
        instanceTop = 0;
        while (token == nameconst) 
        {
            instanceVariables[instanceTop++] = MemoryManager::Instance()->newSymbol(tokenString);
            size++;
            nextToken();
        }
        vars = MemoryManager::Instance()->newArray(instanceTop);
        for (i = 0; i < instanceTop; i++) 
        {
            vars->basicAtPut(i+1, instanceVariables[i]);
        }
        classObj->basicAtPut(variablesInClass, vars);
    }
    classObj->basicAtPut(sizeInClass, MemoryManager::Instance()->newInteger(size));
    classObj->basicAtPut(methodsInClass, MemoryManager::Instance()->newDictionary(39));
}

/*
    readDeclaration reads a declaration of a class
*/
static void readClassDeclaration()
{   
    ObjectHandle classObj, metaObj, vars;
    std::string className, superName;
    int i, size, instanceTop;
    // todo: fixed length variables array!
    ObjectHandle instanceVariables[15];
    // todo: horrible fixed length arrays!
    char metaClassName[100];
    char metaSuperClassName[100];

    if (nextToken() != nameconst)
        sysError("bad file format","no name in declaration");
    className = tokenString;
    if (nextToken() == nameconst) 
    { /* read superclass name */
        superName = tokenString;
        nextToken();
    }
    // todo: sprintf eradication!
    sprintf(metaClassName, "Meta%s", className.c_str());
    if(!superName.empty())
        sprintf(metaSuperClassName, "Meta%s", superName.c_str());
    else
        sprintf(metaSuperClassName, "Class");

    metaObj = createRawClass(metaClassName, "Class", metaSuperClassName);
    classObj = createRawClass(className.c_str(), metaClassName, superName.c_str());
    classObj->_class = metaObj;

    // Get the current class size, we'll build on this as 
    // we add instance variables.
    size = getInteger(classObj->basicAt(sizeInClass));

    if (token == nameconst) 
    {     /* read instance var names */
        instanceTop = 0;
        while (token == nameconst) 
        {
            instanceVariables[instanceTop++] = MemoryManager::Instance()->newSymbol(tokenString);
            size++;
            nextToken();
        }
        vars = MemoryManager::Instance()->newArray(instanceTop);
        for (i = 0; i < instanceTop; i++) 
        {
            vars->basicAtPut(i+1, instanceVariables[i]);
        }
        classObj->basicAtPut(variablesInClass, vars);
    }
    classObj->basicAtPut(sizeInClass, MemoryManager::Instance()->newInteger(size));
    classObj->basicAtPut(methodsInClass, MemoryManager::Instance()->newDictionary(39));
}

/*
    readClass reads a class method description
*/
static void readMethods(FILE* fd, bool printit)
{   
    ObjectHandle classObj, methTable, theMethod, selector;
# define LINEBUFFERSIZE 16384
    char *cp, *eoftest, lineBuffer[LINEBUFFERSIZE];
    ObjectHandle protocol;

    lineBuffer[0] = '\0';
    protocol = nilobj;
    if (nextToken() != nameconst)
        sysError("missing name","following Method keyword");
    classObj = findClass(tokenString);
    setInstanceVariables(classObj);
    if (printit)
        cp = objectRef(classObj->basicAt(nameInClass)).charPtr();

    /* now find or create a method table */
    methTable = classObj->basicAt(methodsInClass);
    if (methTable == nilobj) 
    {  /* must make */
        methTable = MemoryManager::Instance()->newDictionary(MethodTableSize);
        classObj->basicAtPut(methodsInClass, methTable);
    }

    if(nextToken() == strconst) 
    {
        protocol = MemoryManager::Instance()->newStString(tokenString);
    }

    /* now go read the methods */
    do {
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
        theMethod = MemoryManager::Instance()->newMethod();
        if (parseMessageHandler(theMethod, textBuffer, savetext)) {
            selector = theMethod->basicAt(messageInMethod);
            theMethod->basicAtPut(methodClassInMethod, classObj);
            theMethod->basicAtPut(protocolInMethod, protocol);
            if (printit)
                dspMethod(cp, selector->charPtr());
            nameTableInsert(methTable, selector.hash(), selector, theMethod);
        }
        else {
            /* get rid of unwanted method */
            givepause();
        }

    } while (lineBuffer[0] != ']');

}

extern ObjectHandle processStack;
extern int linkPointer;


/*
    fileIn reads in a module definition
*/
void fileIn(FILE* fd, bool printit)
{
    while(fgets(textBuffer, TextBufferSize, fd) != NULL) 
    {
        lexinit(textBuffer);
        if (token == inputend)
            ; /* do nothing, get next line */
        else if ((token == binary) && streq(tokenString, "*"))
            ; /* do nothing, its a comment */
        else if ((token == nameconst) && streq(tokenString, "RawClass"))
            readRawClassDeclaration();
        else if ((token == nameconst) && streq(tokenString, "Class"))
            readClassDeclaration();
        else if ((token == nameconst) && streq(tokenString,"Methods"))
            readMethods(fd, printit);
        else 
            sysError("unrecognized line", textBuffer);
        MemoryManager::Instance()->garbageCollect();
    }
}
