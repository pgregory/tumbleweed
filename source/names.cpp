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
    are created (via newSymbol(), below) they are inserted into this
    table, if not already there.

    This module also manages the definition of various symbols that are
    given fixed values for efficiency sake.  These include the objects
    nil, true, false, and various classes.
*/

# include <stdio.h>
# include <string.h>
# include "env.h"
# include "memory.h"
# include "names.h"

void nameTableInsert(object dict, int hash, object key, object value)
{   
    object table, link, nwLink, nextLink, tablentry;

    /* first get the hash table */
    table = dict->basicAt(tableInDictionary);

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

    table = dict->basicAt(tableInDictionary);

    /* now see if table is valid */
    if ((tablesize = table->size) < 3)
        sysError("system error","lookup on null table");
    else 
    {
        hash = 1+ (3 * (hash % (tablesize / 3)));
        hp = table->sysMemPtr() + (hash-1);
        key = *hp++; /* table at: hash */
        value = *hp++; /* table at: hash + 1 */
        if ((key != nilobj) && (*fun)(key)) 
            return value;
        for (link = *hp; link != nilobj; link = *hp) 
        {
            hp = link->sysMemPtr();
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
    if (key->charPtr() && streq(key->charPtr(), charBuffer)) {
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

const char *unStrs[] = {
    "isNil", 
    "notNil", 
    "value", 
    "new", 
    "class", 
    "size",
    "basicSize", 
    "print", 
    "printString", 
};

const int num_unSyms = sizeof(unStrs)/sizeof(unStrs[0]);
object unSyms[num_unSyms];

const char *binStrs[] = {
    "+", 
    "-", 
    "<", 
    ">", 
    "<=", 
    ">=", 
    "=", 
    "~=", 
    "*", 
    "quo:", 
    "rem:", 
    "bitAnd:", 
    "bitXor:", 
    "==",
    ",", 
    "at:", 
    "basicAt:", 
    "do:", 
    "coerce:", 
    "error:", 
    "includesKey:",
    "isMemberOf:", 
    "new:", 
    "to:", 
    "value:", 
    "whileTrue:", 
    "addFirst:", 
    "addLast:",
};

const int num_binSyms = sizeof(binStrs)/sizeof(binStrs[0]);
object binSyms[num_binSyms];

const struct { 
    const char* name; 
    int index; 
} classStrs[] = {
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
};

const int num_classSyms = sizeof(classStrs)/sizeof(classStrs[0]);
object classSyms[num_classSyms];

object booleanSyms[2];

/* initialize common symbols used by the parser and interpreter */
void initCommonSymbols()
{   
    int i;

    booleanSyms[0] = globalSymbol("true");
    booleanSyms[1] = globalSymbol("false");
    for (i = 0; i < num_unSyms; ++i)
        unSyms[i] = newSymbol(unStrs[i]);
    for (i = 0; i < num_binSyms; ++i)
        binSyms[i] = newSymbol(binStrs[i]);
    for (i = 0; i < num_classSyms; ++i)
        classSyms[classStrs[i].index] = globalSymbol(classStrs[i].name);
}
