/*
    Little Smalltalk, version 2
    Written by Tim Budd, Oregon State University, July 1987
*/
/*
    names and sizes of internally object used internally in the system
*/

# define classSize 5
# define nameInClass 1
# define sizeInClass 2
# define methodsInClass 3
# define superClassInClass 4
# define variablesInClass 5

# define methodSize 9
# define textInMethod 1
# define messageInMethod 2
# define bytecodesInMethod 3
# define literalsInMethod 4
# define stackSizeInMethod 5
# define temporarySizeInMethod 6
# define methodClassInMethod 7
# define watchInMethod 8
# define protocolInMethod 9
# define methodStackSize(x) getInteger(objectRef(x).basicAt(stackSizeInMethod))
# define methodTempSize(x) getInteger(objectRef(x).basicAt(temporarySizeInMethod))

# define contextSize 8
# define linkPtrInContext 1
# define methodInContext 2
# define argumentsInContext 3
# define temporariesInContext 4

# define blockSize 6
# define contextInBlock 1
# define argumentCountInBlock 2
# define argumentLocationInBlock 3
# define bytecountPositionInBlock 4

# define processSize 3
# define stackInProcess 1
# define stackTopInProcess 2
# define linkPtrInProcess 3

# define dictionarySize 1
# define tableInDictionary 1

# define linkSize 3
# define keyInLink 1
# define valueInLink 2
# define nextInLink 3

# define argumentInStack 1
# define prevlinkInStack 2
# define contextInStack 3
# define returnpointInStack 4
# define methodInStack 5
# define bytepointerInStack 6

extern ObjectHandle trueobj;      /* the pseudo variable true */
extern ObjectHandle falseobj;     /* the pseudo variable false */

extern void initCommonSymbols();    /* common symbols */
extern ObjectHandle unSyms[], binSyms[];

extern void nameTableInsert( OBJ X INT X OBJ X OBJ );
/*extern object hashEachElement( OBJ X INT X INT FUNC );*/
extern int strHash ( CSTR );
extern object globalKey ( CSTR );
extern object nameTableLookup ( OBJ X CSTR );
# define globalSymbol(s) nameTableLookup(symbols, s)
object hashEachElement(object dict, register int hash, int(*fun)(object));
