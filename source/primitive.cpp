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
#include "env.h"
#include "memory.h"
#include "names.h"
#include "interp.h"

extern ObjectHandle processStack;
extern int linkPointer;

extern double frexp(), ldexp();
extern object ioPrimitive(INT X OBJP);
extern object sysPrimitive(INT X OBJP);
extern object ffiPrimitive(INT X OBJP);


static object zeroaryPrims(int number)
{   
  short i;
  ObjectHandle returnedObject;

  returnedObject = nilobj;
  switch(number) {

    case 1:
      fprintf(stderr,"did primitive 1\n");
      break;

    case 2:
      fprintf(stderr, "%s", MemoryManager::Instance()->statsString().c_str());
      break;

    case 3:         /* return a random number */
      /* this is hacked because of the representation */
      /* of integers as shorts */
      i = rand() >> 8;  /* strip off lower bits */
      if (i < 0) i = - i;
      returnedObject = MemoryManager::Instance()->newInteger(i>>1);
      break;

    case 4:     /* return time in seconds */
      i = (short) time((long *) 0);
      returnedObject = MemoryManager::Instance()->newInteger(i);
      break;

    case 5:     /* flip watch - done in interp */
      break;

    case 6:
      returnedObject = MemoryManager::Instance()->newCPointer(NULL);
      break;

    case 9:
      exit(0);

    default:        /* unknown primitive */
      sysError("unknown primitive","zeroargPrims");
      break;
  }
  return(returnedObject);
}

static int unaryPrims(int number, object firstarg)
{   
  int i, j, saveLinkPointer;
  ObjectHandle returnedObject, saveProcessStack;

  returnedObject = firstarg;
  switch(number) {
    case 1:     /* class of object */
      returnedObject = objectRef(firstarg)._class;
      break;

    case 2:     /* basic size of object */
      i = objectRef(firstarg).size;
      /* byte objects have negative size */
      if (i < 0) i = (-i);
      returnedObject = MemoryManager::Instance()->newInteger(i);
      break;

    case 3:     /* hash value of object */
      // \todo: Not happy about this, need to review the hashing.
      // This specialises the hash for integers, to ensure values are used, not objects,
      // but there are other cases where the value should be considered, like float.
      if(objectRef(firstarg)._class == globalSymbol("Integer"))
        returnedObject = MemoryManager::Instance()->newInteger(objectRef(firstarg).intValue());
      else
        returnedObject = MemoryManager::Instance()->newInteger(firstarg);
      break;

    case 4:     /* debugging print */
      fprintf(stderr,"primitive 14 %d\n", static_cast<int>(firstarg));
      break;

    case 8:     /* change return point - block return */
      /* first get previous link pointer */
      i = objectRef(processStack->basicAt(linkPointer)).intValue();
      /* then creating context pointer */
      j = objectRef(objectRef(firstarg).basicAt(1)).intValue();
      if (processStack->basicAt(j+1) != firstarg) 
      {
        returnedObject = falseobj;
        break;
      }
      /* first change link pointer to that of creator */
      processStack->basicAtPut(i, 
          processStack->basicAt(j));
      /* then change return point to that of creator */
      processStack->basicAtPut(i+2, 
          processStack->basicAt(j+2));
      returnedObject = trueobj;
      break;

    case 9:         /* process execute */
      /* first save the values we are about to clobber */
      saveProcessStack = processStack;
      saveLinkPointer = linkPointer;
# ifdef SIGNAL
      /* trap control-C */
      signal(SIGINT, brkfun);
      if (setjmp(jb)) 
      {
        returnedObject = falseobj;
      }
      else
# endif
# ifdef CRTLBRK
        /* trap control-C using dos ctrlbrk routine */
        ctrlbrk(brkfun);
      if (setjmp(jb)) 
      {
        returnedObject = falseobj;
      }
      else
# endif
        if (execute(firstarg, 5000))
          returnedObject = trueobj;
        else
          returnedObject = falseobj;
      /* then restore previous environment */
      processStack = saveProcessStack;
      linkPointer = saveLinkPointer;
# ifdef SIGNAL
      signal(SIGINT, brkignore);
# endif
# ifdef CTRLBRK
      ctrlbrk(brkignore);
# endif
      break;

    default:        /* unknown primitive */
      sysError("unknown primitive","unaryPrims");
      break;
  }
  return(returnedObject);
}

static int binaryPrims(int number, object firstarg, object secondarg)
{   
  char buffer[2000];
  int i;
  ObjectHandle returnedObject;

  returnedObject = firstarg;
  switch(number) 
  {
    case 1:     /* object identity test */
      if (firstarg == secondarg)
        returnedObject = trueobj;
      else
        returnedObject = falseobj;
      break;

    case 2:     /* set class of object */
      objectRef(firstarg)._class = secondarg;
      returnedObject = firstarg;
      break;

    case 3:     /* debugging stuff */
      fprintf(stderr,"primitive 23 %d %d\n", static_cast<int>(firstarg), static_cast<int>(secondarg));
      break;

    case 4:     /* string cat */
      ignore strcpy(buffer, objectRef(firstarg).charPtr());
      ignore strcat(buffer, objectRef(secondarg).charPtr());
      returnedObject = MemoryManager::Instance()->newStString(buffer);
      break;

    case 5:     /* basicAt: */
      returnedObject = objectRef(firstarg).basicAt(objectRef(secondarg).intValue());
      break;

    case 6:     /* byteAt: */
      i = objectRef(firstarg).byteAt(objectRef(secondarg).intValue());
      if (i < 0) i += 256;
      returnedObject = MemoryManager::Instance()->newInteger(i);
      break;

    case 7:     /* symbol set */
      nameTableInsert(symbols, strHash(objectRef(firstarg).charPtr()),
          firstarg, secondarg);
      break;

    case 8:     /* block start */
      /* first get previous link */
      i = objectRef(processStack->basicAt(linkPointer)).intValue();
      /* change context and byte pointer */
      processStack->basicAtPut(i+1, firstarg);
      processStack->basicAtPut(i+4, secondarg);
      break;

    case 9:     /* duplicate a block, adding a new context to it */
      returnedObject = MemoryManager::Instance()->newBlock();
      returnedObject->basicAtPut(1, secondarg);
      returnedObject->basicAtPut(2, objectRef(firstarg).basicAt(2));
      returnedObject->basicAtPut(3, objectRef(firstarg).basicAt(3));
      returnedObject->basicAtPut(4, objectRef(firstarg).basicAt(4));
      break;

    default:        /* unknown primitive */
      sysError("unknown primitive","binaryprims");
      break;

  }
  return(returnedObject);
}

static int trinaryPrims(int number, object firstarg, object secondarg, object thirdarg)
{   
  // todo: Fixed length buffer
  char *bp, *tp, buffer[4096];
  int i, j;
  ObjectHandle returnedObject;

  returnedObject = firstarg;
  switch(number) 
  {
    case 1:         /* basicAt:Put: */
      fprintf(stderr,"IN BASICATPUT %d %d %d\n", static_cast<int>(firstarg), objectRef(secondarg).intValue(), static_cast<int>(thirdarg));
      objectRef(firstarg).basicAtPut(objectRef(secondarg).intValue(), thirdarg);
      break;

    case 2:         /* basicAt:Put: for bytes */
      objectRef(firstarg).byteAtPut(objectRef(secondarg).intValue(), objectRef(thirdarg).intValue());
      break;

    case 3:         /* string copyFrom:to: */
      bp = objectRef(firstarg).charPtr();
      i = objectRef(secondarg).intValue();
      j = objectRef(thirdarg).intValue();
      tp = buffer;
      if (i <= strlen(bp))
        for ( ; (i <= j) && bp[i-1]; i++)
          *tp++ = bp[i-1];
      *tp = '\0';
      returnedObject = MemoryManager::Instance()->newStString(buffer);
      break;

    case 9:         /* compile method */
      setInstanceVariables(firstarg);
      if (parse(thirdarg, objectRef(secondarg).charPtr(), false)) {
        flushCache(objectRef(thirdarg).basicAt(messageInMethod), firstarg);
        objectRef(thirdarg).basicAtPut(methodClassInMethod, firstarg);
        returnedObject = trueobj;
      }
      else
        returnedObject = falseobj;
      break;

    default:        /* unknown primitive */
      sysError("unknown primitive","trinaryPrims");
      break;
  }
  return(returnedObject);
}

static int intUnary(int number, object firstarg)
{   
  ObjectHandle returnedObject;

  switch(number) 
  {
    case 1:     /* float equiv of integer */
      returnedObject = MemoryManager::Instance()->newFloat((double) firstarg);
      break;

    case 2:     /* print - for debugging purposes */
      fprintf(stderr,"debugging print %d\n", static_cast<int>(firstarg));
      break;

    case 3: /* set time slice - done in interpreter */
      break;

    case 5:     /* set random number */
      ignore srand((unsigned) firstarg);
      returnedObject = nilobj;
      break;

    case 8:
      returnedObject = MemoryManager::Instance()->allocObject(firstarg);
      break;

    case 9:
      returnedObject = MemoryManager::Instance()->allocByte(firstarg);
      break;

    default:
      sysError("intUnary primitive","not implemented yet");
  }
  return(returnedObject);
}

static object intBinary(register int number, register int firstarg, int secondarg)
{   
  boolean binresult;
  long longresult;
  ObjectHandle returnedObject;

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
      returnedObject = trueobj;
    else
      returnedObject = falseobj;
  else
    returnedObject = MemoryManager::Instance()->newInteger(firstarg);
  return(returnedObject);

  /* on overflow, return nil and let smalltalk code */
  /* figure out what to do */
overflow:
  returnedObject = nilobj;
  return(returnedObject);
}

static int strUnary(int number, char* firstargument)
{   
  ObjectHandle returnedObject;

  switch(number) 
  {
    case 1:     /* length of string */
      returnedObject = MemoryManager::Instance()->newInteger(strlen(firstargument));
      break;

    case 2:     /* hash value of symbol */
      returnedObject = MemoryManager::Instance()->newInteger(strHash(firstargument));
      break;

    case 3:     /* string as symbol */
      returnedObject = MemoryManager::Instance()->newSymbol(firstargument);
      break;

    case 7:     /* value of symbol */
      returnedObject = globalSymbol(firstargument);
      break;

    case 8:
# ifndef NOSYSTEM
      returnedObject = MemoryManager::Instance()->newInteger(system(firstargument));
# endif
      break;

    case 9:
      sysError("fatal error", firstargument);
      break;

    default:
      sysError("unknown primitive", "strUnary");
      break;
  }

  return(returnedObject);
}

static int floatUnary(int number, double firstarg)
{   
  char buffer[20];
  double temp;
  int i, j;
  ObjectHandle returnedObject;

  switch(number) 
  {
    case 1:     /* floating value asString */
      ignore sprintf(buffer,"%g", firstarg);
      returnedObject = MemoryManager::Instance()->newStString(buffer);
      break;

    case 2:     /* log */
      returnedObject = MemoryManager::Instance()->newFloat(log(firstarg));
      break;

    case 3:     /* exp */
      returnedObject = MemoryManager::Instance()->newFloat(exp(firstarg));
      break;

    case 6:     /* integer part */
      /* return two integers n and m such that */
      /* number can be written as n * 2** m */
# define ndif 12
      temp = frexp(firstarg, &i);
      if ((i >= 0)&&(i <= ndif)) {temp=ldexp(temp, i); i=0;}
      else { i -= ndif; temp = ldexp(temp, ndif); }
      j = (int) temp;
      returnedObject = MemoryManager::Instance()->newArray(2);
      returnedObject->basicAtPut(1, MemoryManager::Instance()->newInteger(j));
      returnedObject->basicAtPut(2, MemoryManager::Instance()->newInteger(i));
# ifdef trynew
      /* if number is too big it can't be integer anyway */
      if (firstarg > 2e9)
        returnedObject = nilobj;
      else {
        ignore modf(firstarg, &temp);
        ltemp = (long) temp;
        if (longCanBeInt(ltemp))
          returnedObject = MemoryManager::Instance()->newInteger((int) temp);
        else
          returnedObject = MemoryManager::Instance()->newFloat(temp);
      }
# endif
      break;

    default:
      sysError("unknown primitive","floatUnary");
      break;
  }

  return(returnedObject);
}

static object floatBinary(int number, double first, double second)
{    
  boolean binResult;
  ObjectHandle returnedObject;

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
      returnedObject = trueobj;
    else
      returnedObject = falseobj;
  else
    returnedObject = MemoryManager::Instance()->newFloat(first);
  return(returnedObject);
}


static int cPointerUnary(int number, void* firstarg)
{   
  char buffer[20];
  ObjectHandle returnedObject;

  switch(number) 
  {
    case 1:     /* cPointer value asString */
      //ignore sprintf(buffer,"0x%X", reinterpret_cast<unsigned int>(firstarg));
      returnedObject = MemoryManager::Instance()->newStString(buffer);
      break;
    default:
      sysError("unknown primitive","cPointerUnary");
      break;
  }

  return(returnedObject);
}


/* primitive -
   the main driver for the primitive handler
   */
object primitive(register int primitiveNumber, object* arguments)
{   
  register int primitiveGroup = primitiveNumber / 10;
  ObjectHandle returnedObject;

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
      returnedObject = intUnary(primitiveNumber-50, objectRef(arguments[0]).intValue());
      break;

    case 6: case 7:     /* integer binary operations */
      returnedObject = intBinary(primitiveNumber-60,
          objectRef(arguments[0]).intValue(), 
          objectRef(arguments[1]).intValue());
      break;

    case 8:         /* string unary */
      returnedObject = strUnary(primitiveNumber-80, objectRef(arguments[0]).charPtr());
      break;

    case 10:        /* float unary */
      returnedObject = floatUnary(primitiveNumber-100, objectRef(arguments[0]).floatValue());
      break;

    case 11:        /* float binary */
      returnedObject = floatBinary(primitiveNumber-110,
          objectRef(arguments[0]).floatValue(),
          objectRef(arguments[1]).floatValue());
      break;

    case 12: case 13:   /* file operations */

      returnedObject = ioPrimitive(primitiveNumber-120, arguments);
      break;

    case 14:  /* long unary */
      returnedObject = cPointerUnary(primitiveNumber-140, objectRef(arguments[0]).cPointerValue());
      break;

    case 15:
      /* system dependent primitives, handled in separate module */
      returnedObject = sysPrimitive(primitiveNumber, arguments);
      break;

    case 18:
      returnedObject = ffiPrimitive(primitiveNumber, arguments);
      break;

    default:
      sysError("unknown primitive number","doPrimitive");
      break;
  }

  return (returnedObject);
}
