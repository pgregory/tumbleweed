/*
   Little Smalltalk, version 3
   Written by Tim Budd, Oregon State University, July 1988

   Primitive processor

   primitives are how actions are ultimately executed in the Smalltalk 
   system.
   unlike ST-80, Little Smalltalk primitives cannot fail (although
   they can return nil, and methods can take this as an indication
   of failure).  In this respect primitives in Little Smalltalk are
   much more like traditional system calls.

   Primitives are combined into groups of 10 according to 
   argument count and type, and in some cases type checking is performed.

   IMPORTANT NOTE:
   The technique used to tell if an arithmetic operation
   has overflowed in intBinary() depends upon integers
   being 16 bits.  If this is not true, other techniques
   may be required.

   system specific I/O primitives are found in a different file.
   */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#if !defined WIN32
#include <sys/time.h>
#else
#include <Windows.h>
#define SECS_BETWEEN_EPOCHS 11644473600
#endif

#include "env.h"
#include "memory.h"
#include "names.h"
#include "interp.h"
#include "parser.h"
#include "primitive.h"

extern object processStack;
extern int linkPointer;

//extern double frexp(), ldexp();
extern object ioPrimitive(int, object*);
extern object sysPrimitive(int, object*);
#if defined TW_ENABLE_FFI
extern object ffiPrimitive(int, object*);
#endif


/* 0 - 9 */
static object zeroaryPrims(int number)
{   
  long i;
  object returnedObject;
  long ms_now;

  returnedObject = nilobj;
  switch(number) {

    case 3:         /* return a random number */
      /* this is hacked because of the representation */
      /* of integers as shorts */
      i = rand() >> 8;  /* strip off lower bits */
      if (i < 0) i = - i;
      returnedObject = newInteger(i>>1);
      break;

    default:        /* unknown primitive */
      sysError("unknown primitive","zeroargPrims");
      break;
  }
  return(returnedObject);
}

/* 10 - 19 */
static object unaryPrims(int number, object firstarg)
{   
  int i, j, saveLinkPointer;
  object returnedObject;
  object saveProcessStack;
  SObjectHandle* lock_saveProcessStack = 0;

  returnedObject = firstarg;
  switch(number) {
    case 1:     /* class of object */
      returnedObject = firstarg->_class;
      break;

    case 8:     /* change return point - block return */
      /* first get previous link pointer */
      i = getInteger(basicAt(processStack,linkPointer));
      /* then creating context pointer */
      j = getInteger(basicAt(firstarg,1));
      if (basicAt(processStack,j+1) != firstarg) 
      {
        returnedObject = booleanSyms[booleanFalse];
        break;
      }
      /* first change link pointer to that of creator */
      basicAtPut(processStack,i, 
          basicAt(processStack,j));
      /* then change return point to that of creator */
      basicAtPut(processStack,i+2, 
          basicAt(processStack,j+2));
      returnedObject = booleanSyms[booleanTrue];
      break;

    default:        /* unknown primitive */
      sysError("unknown primitive","unaryPrims");
      break;
  }
  return(returnedObject);
}

/* 20 - 29 */
static object binaryPrims(int number, object firstarg, object secondarg)
{   
  char buffer[2000];
  int i;
  object returnedObject;

  returnedObject = firstarg;
  switch(number) 
  {
    case 1:     /* object identity test */
      if (firstarg == secondarg)
        returnedObject = booleanSyms[booleanTrue];
      else
        returnedObject = booleanSyms[booleanFalse];
      break;

    case 4:     /* string cat */
      strcpy(buffer, charPtr(firstarg));
      strcat(buffer, charPtr(secondarg));
      returnedObject = newStString(buffer);
      break;

    case 5:     /* basicAt: */
      returnedObject = basicAt(firstarg,getInteger(secondarg));
      break;

    default:        /* unknown primitive */
      sysError("unknown primitive","binaryprims");
      break;

  }
  return(returnedObject);
}

/* 30 - 39 */
static object trinaryPrims(int number, object firstarg, object secondarg, object thirdarg)
{   
  // todo: Fixed length buffer
  char *bp, *tp, buffer[4096];
  unsigned int i, j;
  object returnedObject;

  returnedObject = firstarg;
  switch(number) 
  {
    case 1:         /* basicAt:Put: */
      basicAtPut(firstarg,getInteger(secondarg), thirdarg);
      break;

    case 3:         /* string copyFrom:to: */
      bp = charPtr(firstarg);
      i = getInteger(secondarg);
      j = getInteger(thirdarg);
      tp = buffer;
      if (i <= strlen(bp))
        for ( ; (i <= j) && bp[i-1]; i++)
          *tp++ = bp[i-1];
      *tp = '\0';
      returnedObject = newStString(buffer);
      break;

    default:        /* unknown primitive */
      sysError("unknown primitive","trinaryPrims");
      break;
  }
  return(returnedObject);
}

/* 50 - 59 */
static object intUnary(int number, object firstarg)
{   
  object returnedObject;

  switch(number) 
  {
    case 1:     /* float equiv of integer */
      returnedObject = newFloat((double)getInteger(firstarg));
      break;

    case 3: /* set time slice - done in interpreter */
      break;

    case 5:     /* set random number */
      srand((unsigned) getInteger(firstarg));
      returnedObject = nilobj;
      break;

    default:
      sysError("intUnary primitive","not implemented yet");
  }
  return(returnedObject);
}

/* 60 - 79 */
static object intBinary(register int number, register int firstarg, int secondarg)
{   
  int binresult;
  long longresult;
  object returnedObject;

  switch(number) 
  {
    case 0:     /* addition */
      longresult = firstarg;
      longresult += secondarg;
      if (longCanBeInt(longresult))
        firstarg = longresult; 
      else
        goto overflow;
      break;
    case 1:     /* subtraction */
      longresult = firstarg;
      longresult -= secondarg;
      if (longCanBeInt(longresult))
        firstarg = longresult;
      else
        goto overflow;
      break;

    case 2:     /* relationals */
      binresult = firstarg < secondarg; break;
    case 3:
      binresult = firstarg > secondarg; break;
    case 4:
      binresult = firstarg <= secondarg; break;
    case 5:
      binresult = firstarg >= secondarg; break;
    case 6: case 13:
      binresult = firstarg == secondarg; break;
    case 7:
      binresult = firstarg != secondarg; break;

    case 8:     /* multiplication */
      longresult = firstarg;
      longresult *= secondarg;
      if (longCanBeInt(longresult))
        firstarg = longresult;
      else
        goto overflow;
      break;

    case 9:     /* quo: */
      if (secondarg == 0) goto overflow;
      firstarg /= secondarg; break;

    case 10:    /* rem: */
      if (secondarg == 0) goto overflow;
      firstarg %= secondarg; break;

    case 11:    /* bit operations */
      firstarg &= secondarg; break;

    case 12:
      firstarg ^= secondarg; break;

    case 19:    /* shifts */
      if (secondarg < 0)
        firstarg >>= (- secondarg);
      else
        firstarg <<= secondarg;
      break;
  }
  if ((number >= 2) && (number <= 7))
    if (binresult)
      returnedObject = booleanSyms[booleanTrue];
    else
      returnedObject = booleanSyms[booleanFalse];
  else
    returnedObject = newInteger(firstarg);
  return(returnedObject);

  /* on overflow, return nil and let smalltalk code */
  /* figure out what to do */
overflow:
  returnedObject = nilobj;
  return(returnedObject);
}

/* 80 - 89 */
static object strUnary(int number, char* firstargument)
{   
  object returnedObject;

  switch(number) 
  {
    case 1:     /* length of string */
      returnedObject = newInteger(strlen(firstargument));
      break;

    case 2:     /* hash value of symbol */
      returnedObject = newInteger(strHash(firstargument));
      break;

    case 3:     /* string as symbol */
      returnedObject = newSymbol(firstargument);
      break;

    case 7:     /* value of symbol */
      returnedObject = globalSymbol(firstargument);
      break;

    case 8:
# ifndef NOSYSTEM
      returnedObject = newInteger(system(firstargument));
# endif
      break;

    default:
      sysError("unknown primitive", "strUnary");
      break;
  }

  return(returnedObject);
}

/* 100 - 109 */
static object floatUnary(int number, double firstarg)
{   
  char buffer[20];
  double temp;
  int i, j;
  object returnedObject;

  switch(number) 
  {
    case 1:     /* floating value asString */
      sprintf(buffer,"%g", firstarg);
      returnedObject = newStString(buffer);
      break;

    case 2:     /* log */
      returnedObject = newFloat(log(firstarg));
      break;

    case 3:     /* exp */
      returnedObject = newFloat(exp(firstarg));
      break;

    case 6:     /* integer part */
      /* return two integers n and m such that */
      /* number can be written as n * 2** m */
# define ndif 12
      temp = frexp(firstarg, &i);
      if ((i >= 0)&&(i <= ndif)) {temp=ldexp(temp, i); i=0;}
      else { i -= ndif; temp = ldexp(temp, ndif); }
      j = (int) temp;
      returnedObject = newArray(2);
      basicAtPut(returnedObject,1, newInteger(j));
      basicAtPut(returnedObject,2, newInteger(i));
# ifdef trynew
      /* if number is too big it can't be integer anyway */
      if (firstarg > 2e9)
        returnedObject = nilobj;
      else {
        modf(firstarg, &temp);
        ltemp = (long) temp;
        if (longCanBeInt(ltemp))
          returnedObject = newInteger((int) temp);
        else
          returnedObject = newFloat(temp);
      }
# endif
      break;

    default:
      sysError("unknown primitive","floatUnary");
      break;
  }

  return(returnedObject);
}

/* 110 - 119 */
static object floatBinary(int number, double first, double second)
{    
  int binResult;
  object returnedObject;

  switch(number) 
  {
    case 0: first += second; break;

    case 1: first -= second; break;
    case 2: binResult = (first < second); break;
    case 3: binResult = (first > second); break;
    case 4: binResult = (first <= second); break;
    case 5: binResult = (first >= second); break;
    case 6: binResult = (first == second); break;
    case 7: binResult = (first != second); break;
    case 8: first *= second; break;
    case 9: first /= second; break;
    default:    
            sysError("unknown primitive", "floatBinary");
            break;
  }

  if ((number >= 2) && (number <= 7))
    if (binResult)
      returnedObject = booleanSyms[booleanTrue];
    else
      returnedObject = booleanSyms[booleanFalse];
  else
    returnedObject = newFloat(first);
  return(returnedObject);
}


/* 140 - 149 */
static object cPointerUnary(int number, void* firstarg)
{   
  object returnedObject;
  char cpointerString[100];

  switch(number) 
  {
    default:
      sysError("unknown primitive","cPointerUnary");
      break;
  }

  return(returnedObject);
}

/* 200 - 209 */
static object exceptionPrimitive(int number, object* arguments)
{
  object returnedObject;

  switch(number)
  {
    case 1:     // 201 on:do:
      returnedObject = newStString("on:do:");
      break;

    case 2:     // signal
      fprintf(stderr, "signal from: %s\n", charPtr(basicAt(arguments[0]->_class, nameInClass)));
      break;

    default:
      sysError("unknown primitive", "exceptionPrimitive");
      break;
  }
  return returnedObject;
}


/* primitive -
   the main driver for the primitive handler
   */
object primitive(register int primitiveNumber, object* arguments)
{   
  register int primitiveGroup = primitiveNumber / 10;
  object returnedObject;

  switch(primitiveGroup) 
  {
    case 0:
      returnedObject = zeroaryPrims(primitiveNumber);
      break;
    case 1:
      returnedObject = unaryPrims(primitiveNumber - 10, arguments[0]);
      break;
    case 2:
      returnedObject = binaryPrims(primitiveNumber-20, arguments[0], arguments[1]);
      break;
    case 3:
      returnedObject = trinaryPrims(primitiveNumber-30, arguments[0], arguments[1], arguments[2]);
      break;

    case 5:         /* integer unary operations */
      returnedObject = intUnary(primitiveNumber-50, arguments[0]);
      break;

    case 6: case 7:     /* integer binary operations */
      returnedObject = intBinary(primitiveNumber-60,
          getInteger(arguments[0]), 
          getInteger(arguments[1]));
      break;

    case 8:         /* string unary */
      returnedObject = strUnary(primitiveNumber-80, charPtr(arguments[0]));
      break;

    case 10:        /* float unary */
      returnedObject = floatUnary(primitiveNumber-100, floatValue(arguments[0]));
      break;

    case 11:        /* float binary */
      returnedObject = floatBinary(primitiveNumber-110,
          floatValue(arguments[0]),
          floatValue(arguments[1]));
      break;

    case 12: case 13:   /* file operations */

      returnedObject = ioPrimitive(primitiveNumber-120, arguments);
      break;

    case 14:  /* long unary */
      returnedObject = cPointerUnary(primitiveNumber-140, cPointerValue(arguments[0]));
      break;

    case 15:
      /* system dependent primitives, handled in separate module */
      returnedObject = sysPrimitive(primitiveNumber, arguments);
      break;

    case 18:
#if defined TW_ENABLE_FFI
      returnedObject = ffiPrimitive(primitiveNumber, arguments);
#else
      sysError("FFI not enabled on this platform", "");
#endif
      break;

    case 20:
      returnedObject = exceptionPrimitive(primitiveNumber-200, arguments);
      break;

    default:
      sysError("unknown primitive number","doPrimitive");
      break;
  }

  return (returnedObject);
}


/*! New primitive mechanism
 */

object PRIM_debugPrint(int primitiveNumber, object* args, int argc)
{
  object returnedObject;
 
  if(argc != 1)
    sysError("invalid number of arguments to primitive", "PRIM_debugPrint");

  fprintf(stdout, "debugPrint : %s\n", charPtr(args[0]));
  returnedObject = newInteger(strlen(charPtr(args[0])));

  return returnedObject;
}

object PRIM_testFunc(int primitiveNumber, object* args, int argc)
{
  fprintf(stdout, "Hello from testFunc\n");

  return nilobj;
}


PrimitiveTableEntry vmPrimitiveTable[];
#define checkArgCount(n) if(argc != (n)) sysError("invalid number of arguments to primitive", vmPrimitiveTable[primitiveNumber].name)
object vmPrimitiveHandler(int primitiveNumber, object* args, int argc)
{
  object returnedObject = nilobj;
  unsigned int i;
  char cpointerString[100];

  switch(primitiveNumber)
  {
    case 0:     /* exit */
      exit(0);
      break;

    case 1:     /* error */
      checkArgCount(1);
      sysError("fatal error", charPtr(args[0]));
      break;

    case 2:     /* new_CPointer */
      returnedObject = newCPointer(NULL);
      break;

    case 3:     /* new_Object */
      checkArgCount(1);
      returnedObject = allocObject(getInteger(args[0]));
      break;

    case 4:     /* new_ByteObject */
      checkArgCount(1);
      returnedObject = allocByte(getInteger(args[0]));
      break;

    case 5:     /* setClass */
      checkArgCount(2);
      args[0]->_class = args[1];
      returnedObject = args[0];
      break;

    case 6:     /* basicSize */
      checkArgCount(1);
      i = args[0]->size;
      /* byte objects have negative size */
      if (i < 0) 
        i = (-i);
      returnedObject = newInteger(i);
      break;

    case 7:     /* identityHash */
      checkArgCount(1);
      // \todo: Not happy about this, need to review the hashing.
      // This specialises the hash for integers, to ensure values are used, not objects,
      // but there are other cases where the value should be considered, like float.
      if(args[0]->_class == globalSymbol("Integer"))
        returnedObject = newInteger(getInteger(args[0]));
      else
        returnedObject = newInteger(hashObject(args[0]));
      break;

    case 8:     /* class */
      checkArgCount(1);
      returnedObject = getClass(args[0]);
      break;

    case 9:     /* byteAt */
      checkArgCount(2);
      i = byteAt(args[0], getInteger(args[1]));
      if (i < 0) 
        i += 256;
      returnedObject = newInteger(i);
      break;

    case 10:    /* byteAtPut */
      checkArgCount(3);
      byteAtPut(args[0], getInteger(args[1]), getInteger(args[2]));
      break;

    case 11:    /* symbolSet */
      checkArgCount(2);
      nameTableInsert(symbols, strHash(charPtr(args[0])),
          args[0], args[1]);
      break;

    case 12:    /* CPointerAsString */
      checkArgCount(1);
      snprintf(cpointerString, 100, "%p", args[0]);
      returnedObject = newStString(cpointerString);
      break;

    default:
      break;
  }
  return returnedObject;
}
#undef checkArgCount

PrimitiveTableEntry executePrimitiveTable[];
#define checkArgCount(n) if(argc != (n)) sysError("invalid number of arguments to primitive", executePrimitiveTable[primitiveNumber].name)
object executePrimitiveHandler(int primitiveNumber, object* args, int argc)
{
  int i,j, saveLinkPointer;
  object saveProcessStack;
  SObjectHandle* lock_saveProcessStack = 0;
  object returnedObject = nilobj;

  switch(primitiveNumber)
  {
    case 0:     /* execute */
      checkArgCount(1);
      /* first save the values we are about to clobber */
      saveProcessStack = processStack;
      // \todo: not sure if this lock is necessary.
      lock_saveProcessStack = new_SObjectHandle_from_object(saveProcessStack);
      saveLinkPointer = linkPointer;
      if (execute(args[0], 5000))
        returnedObject = booleanSyms[booleanTrue];
      else
        returnedObject = booleanSyms[booleanFalse];
      /* then restore previous environment */
      processStack = saveProcessStack;
      free_SObjectHandle(lock_saveProcessStack);
      linkPointer = saveLinkPointer;
      break;

    case 1:     /* blockStart */
      checkArgCount(2);
      /* first get previous link */
      i = getInteger(basicAt(processStack,linkPointer));
      /* change context and byte pointer */
      basicAtPut(processStack,i+1, args[0]);
      basicAtPut(processStack,i+4, args[1]);
      break;

    case 2:     /* blockReturn */
      checkArgCount(1);
      /* first get previous link pointer */
      i = getInteger(basicAt(processStack,linkPointer));
      /* then creating context pointer */
      j = getInteger(basicAt(args[0],1));
      if (basicAt(processStack,j+1) != args[0]) 
      {
        returnedObject = booleanSyms[booleanFalse];
        break;
      }
      /* first change link pointer to that of creator */
      basicAtPut(processStack,i, basicAt(processStack,j));
      /* then change return point to that of creator */
      basicAtPut(processStack,i+2, basicAt(processStack,j+2));
      returnedObject = booleanSyms[booleanTrue];
      break;

    case 3:     /* duplicate a block, adding a new context to it */
      checkArgCount(2);
      returnedObject = newBlock();
      basicAtPut(returnedObject,contextInBlock, args[1]);
      basicAtPut(returnedObject,argumentCountInBlock, basicAt(args[0],2));
      basicAtPut(returnedObject,argumentLocationInBlock, basicAt(args[0],3));
      basicAtPut(returnedObject,bytecountPositionInBlock, basicAt(args[0],4));
      break;

    default:
      break;
  }

  return returnedObject;
}
#undef checkArgCount

PrimitiveTableEntry compilePrimitiveTable[];
#define checkArgCount(n) if(argc != (n)) sysError("invalid number of arguments to primitive", compilePrimitiveTable[primitiveNumber].name)
object compilePrimitiveHandler(int primitiveNumber, object* args, int argc)
{
  object returnedObject = nilobj;

  checkArgCount(3);

  resetLexer(charPtr(args[1]));
  setInstanceVariables(args[0]);
  if (parseMessageHandler(args[2], FALSE)) {
    flushCache(basicAt(args[2],messageInMethod), args[0]);
    basicAtPut(args[2],methodClassInMethod, args[0]);
    returnedObject = booleanSyms[booleanTrue];
  }
  else
    returnedObject = booleanSyms[booleanFalse];

  return returnedObject;
}
#undef checkArgCount

PrimitiveTableEntry timePrimitiveTable[];
#define checkArgCount(n) if(argc != (n)) sysError("invalid number of arguments to primitive", timePrimitiveTable[primitiveNumber].name)
object timePrimitiveHandler(int primitiveNumber, object* args, int argc)
{
  long i;
  long ms_now;
  object returnedObject = nilobj;

  switch(primitiveNumber)
  {
    case 0:     /* seconds */
      i = 0;
      returnedObject = newInteger(i);
      break;

    case 1:   /* milliseconds */
      {
#if !defined WIN32
        struct timeval time;

        long mtime, seconds, useconds;    

        gettimeofday(&time, NULL);

        seconds  = time.tv_sec;
        useconds = time.tv_usec;

        mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;

        returnedObject = newInteger(seconds%86400);
#else
        SYSTEMTIME st_now;
        GetSystemTime(&st_now);
        ms_now = ((st_now.wHour * 60 * 60) + (st_now.wMinute * 60) + (st_now.wSecond)) * 1000 + st_now.wMilliseconds;
        returnedObject = newInteger(ms_now);
#endif
      }
      break;
  }
  return returnedObject;
}
#undef checkArgCount

const char* vmPrimId = "30e2b312-7f0f-4151-9f79-b44155f16231";
PrimitiveTableEntry vmPrimitiveTable[] =
{
  { "exit", vmPrimitiveHandler },
  { "error", vmPrimitiveHandler },
  { "new_CPointer", vmPrimitiveHandler },
  { "new_Object", vmPrimitiveHandler },
  { "new_ByteObject", vmPrimitiveHandler },
  { "set_Class", vmPrimitiveHandler },
  { "basicSize", vmPrimitiveHandler },
  { "identityHash", vmPrimitiveHandler },
  { "class", vmPrimitiveHandler },
  { "byteAt", vmPrimitiveHandler },
  { "byteAtPut", vmPrimitiveHandler },
  { "symbolSet", vmPrimitiveHandler },
  { "CPointerAsString", vmPrimitiveHandler },
  { NULL, NULL }
};

const char* executePrimId = "4b24f7c5-8c07-4b66-9731-bda0e2037b07";
PrimitiveTableEntry executePrimitiveTable[] =
{
  { "execute", executePrimitiveHandler },
  { "returnToBlock", executePrimitiveHandler },
  { "blockReturn", executePrimitiveHandler },
  { "blockCreateContext", executePrimitiveHandler },
  { NULL, NULL }
};

const char* compilePrimId = "ac23be20-4b32-421e-844e-d3dce6c2fc46";
PrimitiveTableEntry compilePrimitiveTable[] =
{
  { "compile", compilePrimitiveHandler },
  { NULL, NULL }
};

const char* timePrimId = "bf74a739-5fa1-4a82-8aec-725cf462545f";
PrimitiveTableEntry timePrimitiveTable[] =
{
  { "seconds", timePrimitiveHandler },
  { "milliseconds", timePrimitiveHandler },
  { NULL, NULL }
};

const char* debugPrimId = "5b667e1c-a22d-4974-b969-d1058632a1c5";
PrimitiveTableEntry debugPrims[] =
{
  { "debugPrint", PRIM_debugPrint },
  { NULL, NULL }
};

const char* testPrimId = "3f70b9df-4d19-476a-8f33-df1b841def61";
PrimitiveTableEntry testPrims[] =
{
  { "testFunc", PRIM_testFunc },
  { NULL, NULL }
};

PrimitiveTable* PrimitiveTableRoot = NULL;
PrimitiveTable** PrimitiveTableAddresses = NULL;
int PrimitiveTableCount = 0;
int PrimitiveTableAddressCount = 0;
#define PRIM_TABLE_ADDRESS_SIZE_INCR 10

void addPrimitiveTable(const char* name, UUID id, PrimitiveTableEntry* primitives)
{
  PrimitiveTable* table;

  table = (PrimitiveTable*)calloc(sizeof(PrimitiveTable), 1);
  memcpy((char*)&table->id, (char*)&id, sizeof(UUID));
  table->name = strdup(name);
  table->next = NULL;
  table->primitives = primitives;

  if(NULL == PrimitiveTableRoot)
  {
    PrimitiveTableRoot = table;
  }
  else
  {
    table->next = PrimitiveTableRoot;
    PrimitiveTableRoot = table;
  }

  /* Add the table to the indexing list, creating
   * it if it doesn't already exist */
  if(NULL == PrimitiveTableAddresses || PrimitiveTableCount >= PrimitiveTableAddressCount)
  {
    PrimitiveTableAddressCount += PRIM_TABLE_ADDRESS_SIZE_INCR;
    PrimitiveTableAddresses = (PrimitiveTable**)realloc(PrimitiveTableAddresses, sizeof(PrimitiveTable*)*PrimitiveTableAddressCount);
  }
  PrimitiveTableAddresses[PrimitiveTableCount] = table;

  PrimitiveTableCount++;
}

int findPrimitiveByName(const char* name, int* tableIndex)
{
  int index;
  PrimitiveTable* table;

  *tableIndex = 0;

  for(*tableIndex = 0; *tableIndex < PrimitiveTableCount; ++(*tableIndex))
  {
    table = PrimitiveTableAddresses[*tableIndex];
    if(NULL != table)
    {
      index = 0;
      while(table->primitives[index].name)
      {
        if(!strcmp(table->primitives[index].name, name))
          return index;
        index++;
      }
    }
  }
  return -1;
}

object executePrimitive(int tableIndex, int index, object* args, int argc)
{
  PrimitiveTable* table = PrimitiveTableAddresses[tableIndex];

  if(table)
    return table->primitives[index].fn(index, args, argc);
  else
  {
    sysWarn("missing primitive table","");
    return nilobj;
  }
}


static UUID stringToUUID(const char* strUUID)
{
  UUID id;
  char* end;
  char lvalue[9];
  char svalue[5];
  char bvalue[3];
  int i;

  lvalue[8] = svalue[4] = bvalue[2] = '\0'; 

  /* \todo: Check format there */

  /* Format example: 5b667e1c-a22d-4974-b969-d1058632a1c5 */
  strncpy(lvalue, &strUUID[0], 8);
  id.data1 = strtol(lvalue, NULL, 16);
  strncpy(svalue, &strUUID[9], 4);
  id.data2 = strtol(svalue, NULL, 16);
  strncpy(svalue, &strUUID[14], 4);
  id.data3 = strtol(svalue, NULL, 16);
  strncpy(svalue, &strUUID[19], 4);
  id.data4 = strtol(svalue, NULL, 16);
  for(i = 0; i < 6; ++i)
  {
    strncpy(bvalue, &strUUID[24 + (2*i)], 2);
    id.data5[i] = strtol(bvalue, NULL, 16);
  }

  return id;
} 


static char* UUIDToString(UUID id)
{
  char* result = (char*)calloc(UUID_STRING_LENGTH, sizeof(char));

  snprintf(result, UUID_STRING_LENGTH, "%8.8lx-%4.4hx-%4.4hx-%4.4hx-%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x",
      id.data1,
      id.data2,
      id.data3,
      id.data4,
      id.data5[0], id.data5[1], id.data5[2], id.data5[3], id.data5[4], id.data5[5]);

  return result;
}

int UUIDCompare(UUID a, UUID b)
{
  if(a.data1 == b.data1 &&
     a.data2 == b.data2 &&
     a.data3 == b.data3 &&
     a.data4 == b.data4 &&
     a.data5[0] == b.data5[0] &&
     a.data5[1] == b.data5[1] &&
     a.data5[2] == b.data5[2] &&
     a.data5[3] == b.data5[3] &&
     a.data5[4] == b.data5[4] &&
     a.data5[5] == b.data5[5])
    return TRUE;
  else
    return FALSE;
} 

/* \todo: this is duplicated in memory.c */
static void fw(FILE* fp, char* p, int s)
{
    if (fwrite(p, s, 1, fp) != 1) 
    {
        sysError("imageWrite size error","");
    }
}
static int fr(FILE* fp, char* p, int s)
{   
  int r;

  r = fread(p, s, 1, fp);
  if (r && (r != 1))
    sysError("imageRead count error","");
  return r;
}

void writePrimitiveTables(FILE* fp)
{
  int i;

  /* Write the table count */
  fw(fp, (char*)&PrimitiveTableCount, sizeof(int));

  for(i = 0; i < PrimitiveTableCount; ++i)
    fw(fp, (char*)&(PrimitiveTableAddresses[i]->id), sizeof(UUID));
}
  
void readPrimitiveTables(FILE* fp)
{
  int i, j;
  int count;
  UUID id;

  /* Read the table count */
  fr(fp, (char*)&count, sizeof(int));

  /* Remap the tables to the same order 
   * as the time of the snapshot
   */
  PrimitiveTable** oldAddresses = (PrimitiveTable**)calloc(PrimitiveTableCount, sizeof(PrimitiveTable*));
  memcpy((char*)oldAddresses, PrimitiveTableAddresses, sizeof(PrimitiveTable*)*PrimitiveTableCount);
  free(PrimitiveTableAddresses);
  PrimitiveTableAddresses = (PrimitiveTable**)calloc(count, sizeof(PrimitiveTable*));
  for(i = 0; i < count; ++i)
  {
    fr(fp, (char*)&id, sizeof(UUID));
    /* printf("%s\n", UUIDToString(id)); */
    /* Find it in the current VM table list */
    int found = FALSE;
    for(j = 0; j < PrimitiveTableCount; ++j)
    {
      if(TRUE == UUIDCompare(id, oldAddresses[j]->id))
      {
        found = TRUE;
        PrimitiveTableAddresses[i] = oldAddresses[j];
        break;
      }
    }
    if(found != TRUE)
      printf("Missing primitive table [%s]\n", UUIDToString(id));
  }
#if defined TW_DEBUG
  for(i = 0; i < count; ++i)
  {
    printf("%p : ", PrimitiveTableAddresses[i]);
    if(PrimitiveTableAddresses[i] != NULL)
      printf("%s", UUIDToString(PrimitiveTableAddresses[i]->id));
    printf("\n");
  }
#endif
  PrimitiveTableCount = count;
  free(oldAddresses);
}

void initialiseDebugPrims()
{
  addPrimitiveTable("debug", stringToUUID(debugPrimId), debugPrims);
  addPrimitiveTable("test", stringToUUID(testPrimId), testPrims);
  addPrimitiveTable("execute", stringToUUID(executePrimId), executePrimitiveTable);
  addPrimitiveTable("vm", stringToUUID(vmPrimId), vmPrimitiveTable);
  addPrimitiveTable("time", stringToUUID(timePrimId), timePrimitiveTable);
//#ifndef TW_NO_COMPILER
  addPrimitiveTable("compile", stringToUUID(compilePrimId), compilePrimitiveTable);
//#endif
}
