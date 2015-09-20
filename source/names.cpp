/*
    Little Smalltalk, version 2
    Written by Tim Budd, Oregon State University, July 1987

    Name Table module

    A name table is the term used for a Dictionary indexed by symbols.
    There are two name tables used internally by the bytecode interpreter.
    The first is the table, contained in the variable globalNames,
    that contains the names and values of all globally accessible 
    identifiers.  The second is the table of methods associated with
    every class.  Notice that in neither of these cases does the
    system ever put anything INTO the tables, thus there are only
    routines here for reading FROM tables.

    One complication of instances of class Symbol is that all
    symbols must be unique, not only so that == will work as expected,
    but so that memory does not get overly clogged up with symbols.
    Thus all symbols are kept in a hash table, and when new symbols
    are created (via createSymbol(), below) they are inserted into this
    table, if not already there.

    This module also manages the definition of various symbols that are
    given fixed values for efficiency sake.  These include the objects
    nil, true, false, and various classes.
*/

# include <stdio.h>
# include <string.h>
# include "env.h"
# include "objmemory.h"
# include "names.h"

void nameTableInsert(object dict, int hash, object key, object value)
{   
    ObjectHandle table, link, nwLink, nextLink, tablentry;

    /* first get the hash table */
    table = objectRef(dict).basicAt(tableInDictionary);

    if (table->size < 3)
        sysError("attempt to insert into","too small name table");
    else {
        hash = 3 * ( hash % (table->size / 3));
        tablentry = table->basicAt(hash+1);
        if ((tablentry == nilobj) || (tablentry == key)) {
            table->basicAtPut(hash+1, key);
            table->basicAtPut(hash+2, value);
            }
        else {
            nwLink = newLink(key, value);
            link = table->basicAt(hash+3);
            if (link == nilobj) {
                table->basicAtPut(hash+3, nwLink);
                }
            else
            {
                int iter = 0;
                while(1)
                {
                    if (link->basicAt(keyInLink) == key) {
                        link->basicAtPut(valueInLink, value);
                        break;
                        }
                    else if ((nextLink = link->basicAt(nextInLink)) == nilobj) {
                        link->basicAtPut(nextInLink, nwLink);
                        break;
                        }
                    else
                        link = nextLink;
                    iter++;
                }
            }
        }
    }
}

object hashEachElement(object dict, register int hash, int (*fun)(object))
{   
    object table, key, value, link;
    register object *hp;
    int tablesize;

    table = objectRef(dict).basicAt(tableInDictionary);

    /* now see if table is valid */
    if ((tablesize = objectRef(table).size) < 3)
        sysError("system error","lookup on null table");
    else 
    {
        hash = 1+ (3 * (hash % (tablesize / 3)));
        hp = objectRef(table).sysMemPtr() + (hash-1);
        key = *hp++; /* table at: hash */
        value = *hp++; /* table at: hash + 1 */
        if ((key != nilobj) && (*fun)(key)) 
            return value;
        for (link = *hp; link != nilobj; link = *hp) 
        {
            hp = objectRef(link).sysMemPtr();
            key = *hp++; /* link at: 1 */
            value = *hp++; /* link at: 2 */
            if ((key != nilobj) && (*fun)(key))
                return value;
        }
    }
    return nilobj;
}

int strHash(const char* str)    /* compute hash value of string ---- strHash */
{   
    register int hash;
    register const char *p;

    hash = 0;
    for (p = str; *p; p++)
        hash += *p;
    if (hash < 0) hash = - hash;
    /* make sure it can be a smalltalk integer */
    if (hash > 16384)
        hash >>= 2;
    return hash;
}

static object objBuffer;
static const char   *charBuffer;

static int strTest(object key) /* test for string equality ---- strTest */
{
    if (objectRef(key).charPtr() && streq(objectRef(key).charPtr(), charBuffer)) {
        objBuffer = key;
        return 1;
        }
    return 0;
}

object globalKey(const char *str)   /* return key associated with global symbol */
{
    objBuffer = nilobj;
    charBuffer = str;
    hashEachElement(symbols, strHash(str), strTest);
    return objBuffer;
}

object nameTableLookup(object dict, const char* str)
{
    charBuffer = str;
    return hashEachElement(dict, strHash(str), strTest);
}

std::vector<ObjectHandle> booleanSyms;
std::vector<ObjectHandle> unSyms;
std::vector<ObjectHandle> binSyms;
ObjectHandle classSyms[k__lastClass+1];

const char *unStrs[] = {"isNil", "notNil", "value", "new", "class", "size",
"basicSize", "print", "printString", 0};

const char *binStrs[] = {"+", "-", "<", ">", "<=", ">=", "=", "~=", "*", 
"quo:", "rem:", "bitAnd:", "bitXor:", "==",
",", "at:", "basicAt:", "do:", "coerce:", "error:", "includesKey:",
"isMemberOf:", "new:", "to:", "value:", "whileTrue:", "addFirst:", "addLast:",
0};

struct ClassRef classStrs[] = {
    { "Array", kArray },
    { "Block", kBlock },
    { "ByteArray", kByteArray },
    { "Char", kChar },
    { "Class", kClass },
    { "Context", kContext },
    { "Dictionary", kDictionary },
    { "Float", kFloat },
    { "Integer", kInteger },
    { "CPointer", kCPointer },
    { "Link", kLink },
    { "Method", kMethod },
    { "String", kString },
    { "Symbol", kSymbol },
    { "Process", kProcess },

    { 0, k__lastClass },
};

/* initialize common symbols used by the parser and interpreter */
void initCommonSymbols()
{   
    int i;

    booleanSyms.push_back(globalSymbol("true"));
    booleanSyms.push_back(globalSymbol("false"));
    for (i = 0; unStrs[i]; ++i)
        unSyms.push_back(createSymbol(unStrs[i]));
    for (i = 0; binStrs[i]; ++i)
        binSyms.push_back(createSymbol(binStrs[i]));
    //classSyms.resize(k__lastClass);
    for (i = 0; classStrs[i].index != k__lastClass; ++i)
    {
        ObjectHandle h = globalSymbol(classStrs[i].name);
        classSyms[classStrs[i].index] = globalSymbol(classStrs[i].name);
    }
}



object createSymbol(const char* name)
{
    /* first see if it is already there */
    object nameObj = globalKey(name);
    if (nameObj) 
        return nameObj;

    /* not found, must make */
    nameObj = newSymbol(name);
    nameTableInsert(symbols, strHash(name), nameObj, nilobj);

    return nameObj;
}

object createAndRegisterNewClass(const char* name)
{
    object newObj = newClass(name);
    /* now put in global symbols table */
    object nameObj = newSymbol(name);
    nameTableInsert(symbols, strHash(name), nameObj, newObj);
}


object newArray(int size)
{   
    object newObj;

    newObj = MemoryManager::Instance()->allocObject(size);
    objectRef(newObj)._class = classObject(kArray);
    return newObj;
}

object newBlock()
{   
    object newObj;

    newObj = MemoryManager::Instance()->allocObject(blockSize);
    objectRef(newObj)._class = classObject(kBlock);
    return newObj;
}

object newByteArray(int size)
{   
    object newobj;

    newobj = MemoryManager::Instance()->allocByte(size);
    objectRef(newobj)._class = classObject(kByteArray);
    return newobj;
}

object newChar(int value)
{   
    object newobj;

    newobj = MemoryManager::Instance()->allocObject(1);
    objectRef(newobj).basicAtPut(1, newInteger(value));
    objectRef(newobj)._class = classObject(kChar);
    return(newobj);
}

object newClass(const char* name)
{   
    object newObj, nameObj, methTable;

    newObj = MemoryManager::Instance()->allocObject(classSize);
    objectRef(newObj)._class = classObject(kClass);

    /* now make name */
    //nameObj = newSymbol(name);
    objectRef(newObj).basicAtPut(nameInClass, nameObj); methTable = newDictionary(39);
    objectRef(newObj).basicAtPut(methodsInClass, methTable);
    objectRef(newObj).basicAtPut(sizeInClass, newInteger(classSize));

    return newObj;
}

object newContext(int link, object method, object args, object temp)
{   
    object newObj;

    newObj = MemoryManager::Instance()->allocObject(contextSize);
    objectRef(newObj)._class = classObject(kContext);
    objectRef(newObj).basicAtPut(linkPtrInContext, newInteger(link));
    objectRef(newObj).basicAtPut(methodInContext, method);
    objectRef(newObj).basicAtPut(argumentsInContext, args);
    objectRef(newObj).basicAtPut(temporariesInContext, temp);
    return newObj;
}

object newDictionary(int size)
{   
    object newObj;

    newObj = MemoryManager::Instance()->allocObject(dictionarySize);
    objectRef(newObj)._class = classObject(kDictionary);
    objectRef(newObj).basicAtPut(tableInDictionary, newArray(size));
    return newObj;
}

object newFloat(double d)
{   
    object newObj;

    newObj = MemoryManager::Instance()->allocByte((int) sizeof (double));
    memcpy(objectRef(newObj).charPtr(), (char *) &d, (int) sizeof (double));
    objectRef(newObj)._class = classObject(kFloat);
    return newObj;
}

object newInteger(long i)
{   
#if defined TW_SMALLINTEGER_AS_OBJECT
    object newObj;

    newObj = MemoryManager::Instance()->allocByte((int) sizeof (int));
    memcpy(objectRef(newObj).charPtr(), (char *) &i, (int) sizeof (int));
    objectRef(newObj)._class = classObject(kInteger);
    return newObj;
#else
    return (i << 1) + 1;
#endif
}

object newCPointer(void* l)
{   
    object newObj;

    int s = sizeof(void*);
    newObj = MemoryManager::Instance()->allocByte((int) sizeof (void*));
    memcpy(objectRef(newObj).charPtr(), (char *) &l, (int) sizeof (void*));
    objectRef(newObj)._class = classObject(kCPointer);
    return newObj;
}

object newLink(object key, object value)
{   
    object newObj;

    newObj = MemoryManager::Instance()->allocObject(linkSize);
    objectRef(newObj)._class = classObject(kLink);
    objectRef(newObj).basicAtPut(keyInLink, key);
    objectRef(newObj).basicAtPut(valueInLink, value);
    return newObj;
}

object newMethod()
{   object newObj;

    newObj = MemoryManager::Instance()->allocObject(methodSize);
    objectRef(newObj)._class = classObject(kMethod);
    return newObj;
}

object newStString(const char* value)
{   
  object newObj;

    newObj = MemoryManager::Instance()->allocStr(value);
    objectRef(newObj)._class = classObject(kString);
    return(newObj);
}

object newSymbol(const char* str)
{    
    object newObj;

    newObj = MemoryManager::Instance()->allocStr(str);
    objectRef(newObj)._class = classObject(kSymbol);
    //nameTableInsert(symbols, strHash(str), newObj, nilobj);
    return newObj;
}

object copyFrom(object obj, int start, int size)
{   
    object newObj;
    int i;

    newObj = newArray(size);
    for (i = 1; i <= size; i++) 
    {
        objectRef(newObj).basicAtPut(i, objectRef(obj).basicAt(start));
        start++;
    }
    return newObj;
}
