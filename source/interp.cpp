/*
   Little Smalltalk version 3
   Written by Tim Budd, Oregon State University, July 1988

   bytecode interpreter module

   given a process object, execute bytecodes in a tight loop.

   performs subroutine calls for
   a) finding a non-cached method
   b) executing a primitive

   otherwise simply loops until time slice has ended
   */

# include <stdio.h>
# include "env.h"
# include "memory.h"
# include "names.h"
# include "interp.h"

bool watching = 0;
extern object primitive( int, object* );

/*
   the following variables are local to this module
   */

static ObjectHandle method, messageToSend;

static int messTest(object obj)
{
  return obj == messageToSend;
}

/* a cache of recently executed methods is used for fast lookup */
# define cacheSize 211
static struct 
{
  ObjectHandle cacheMessage;  /* the message being requested */
  ObjectHandle lookupClass;   /* the class of the receiver */
  ObjectHandle cacheClass;    /* the class of the method */
  ObjectHandle cacheMethod;   /* the method itself */
} methodCache[cacheSize];

/* flush an entry from the cache (usually when its been recompiled) */
void flushCache(object messageToSend, object _class)
{   
  int hash;

  hash = ((hashObject(messageToSend)) + (hashObject(_class))) % cacheSize;
  methodCache[hash].cacheMessage = nilobj;
}

/*
   findMethod
   given a message and a class to start looking in,
   find the method associated with the message
   */
static bool findMethod(object* methodClassLocation)
{   
  object methodTable, methodClass;

  method = nilobj;
  methodClass = *methodClassLocation;


//  printf("Looking for %s starting at %d\n", objectRef(messageToSend).charPtr(), methodClass);
  for (; methodClass != nilobj; methodClass = 
      objectRef(methodClass).basicAt(superClassInClass)) {
//    printf("Looking for %s on %s\n", objectRef(messageToSend).charPtr(), objectRef(objectRef(methodClass).basicAt(nameInClass)).charPtr());
    methodTable = objectRef(methodClass).basicAt(methodsInClass);
    if(methodTable == nilobj)
      printf("Null method table on %s\n", objectRef(objectRef(methodClass).basicAt(nameInClass)).charPtr());
    method = hashEachElement(methodTable, hashObject(messageToSend), messTest);
    if (method != nilobj)
    {
//      printf("...found\n");
      break;
    }
  }

  if (method == nilobj) {       /* it wasn't found */
    methodClass = *methodClassLocation;
    return false;
  }

  *methodClassLocation = methodClass;
  return true;
}

# define nextByte() *(bp + byteOffset++)
# define ipush(x) (*++pst=(x))
# define stackTop() *pst
# define stackTopPut(x) (*pst = x)
# define stackTopFree() *pst-- = nilobj
/* note that ipop leaves x with excess reference count */
# define ipop(x) x = stackTop(); *pst-- = nilobj
# define processStackTop() ((pst-psb)+1)
# define receiverAt(n) *(rcv+n)
# define receiverAtPut(n,x) (receiverAt(n)=(x))
# define argumentsAt(n) *(arg+n)
# define temporaryAt(n) *(temps+n)
# define temporaryAtPut(n,x) (temporaryAt(n)=(x))
# define literalsAt(n) *(lits+n)
# define contextAt(n) *(cntx+n)
# define contextAtPut(n,x) (contextAt(n-1)=(x))
# define processStackAt(n) *(psb+(n-1))


/* the following are manipulated by primitives */
ObjectHandle  processStack;
int     linkPointer;

static object growProcessStack(int top, int toadd)
{   
  int size, i;
  ObjectHandle newStack;

  if (toadd < 100) toadd = 100;
  size = processStack->size + toadd;
  newStack = MemoryManager::Instance()->newArray(size);
  for (i = 1; i <= top; i++) {
    newStack->basicAtPut(i, processStack->basicAt(i));
  }
  return newStack;
}

bool execute(object aProcess, int maxsteps)
{   
  object returnedObject;
  int returnPoint, timeSliceCounter;
  object *pst, *psb, *rcv, *arg, *temps, *lits, *cntx;
  ObjectHandle contextObject; 
  object *primargs;
  int byteOffset;
  ObjectHandle argarray;
  object methodClass; 
  int i, j;
  register int low;
  int high;
  byte *bp;
  ObjectHandle intClass = globalSymbol("Integer");

  MemoryManager* memmgr = MemoryManager::Instance();

  /* unpack the instance variables from the process */
  processStack    = objectRef(aProcess).basicAt(stackInProcess);
  psb = processStack->sysMemPtr();
  j = getInteger(objectRef(aProcess).basicAt(stackTopInProcess));
  pst = psb + (j-1);
  linkPointer     = getInteger(objectRef(aProcess).basicAt(linkPtrInProcess));

  /* set the process time-slice counter before entering loop */
  timeSliceCounter = maxsteps;

  /* retrieve current values from the linkage area */
readLinkageBlock:
  contextObject  = processStackAt(linkPointer+1);
  returnPoint = getInteger(processStackAt(linkPointer+2));
  byteOffset  = getInteger(processStackAt(linkPointer+4));
  if (contextObject == nilobj) {
    contextObject = processStack;
    cntx = psb;
    arg = cntx + (returnPoint-1);
    method      = processStackAt(linkPointer+3);
    temps = cntx + linkPointer + 5;
  }
  else {    /* read from context object */
    cntx = contextObject->sysMemPtr();
    method = contextObject->basicAt(methodInContext);
    arg = objectRef(contextObject->basicAt(argumentsInContext)).sysMemPtr();
    temps = objectRef(contextObject->basicAt(temporariesInContext)).sysMemPtr();
  }

#if !defined TW_SMALLINTEGER_AS_OBJECT
  if((argumentsAt(0) & 1) == 0)
#endif
    rcv = objectRef(argumentsAt(0)).sysMemPtr();

readMethodInfo:
  lits           = objectRef(method->basicAt(literalsInMethod)).sysMemPtr();
  bp             = objectRef(method->basicAt(bytecodesInMethod)).bytePtr() - 1;

  while ( --timeSliceCounter > 0 ) {
    low = (high = nextByte()) & 0x0F;
    high >>= 4;
    if (high == 0) {
      high = low;
      low = nextByte();
    }

# if 0
    if (debugging) {
      fprintf(stderr,"method %s %d ",objectRef(objectRef(method).basicAt(messageInMethod)).charPtr(), byteOffset);
      fprintf(stderr,"stack %d %d ",pst, *pst);
      fprintf(stderr,"executing %d %d\n", high, low);
    }
# endif
    switch(high) {

      case PushInstance:
        ipush(receiverAt(low));
        break;

      case PushArgument:
        ipush(argumentsAt(low));
        break;

      case PushTemporary:
        ipush(temporaryAt(low));
        break;

      case PushLiteral:
        ipush(literalsAt(low));
        break;

      case PushConstant:
        switch(low) {
          case 0: case 1: case 2:
            ipush(memmgr->newInteger(low));
            break;

          case minusOne:
            ipush(memmgr->newInteger(-1));
            break;

          case contextConst:
            {
              /* check to see if we have made a block context yet */
              if (contextObject == processStack) 
              {
                /* not yet, do it now - first get real return point */
                returnPoint = getInteger(processStackAt(linkPointer+2));
                ObjectHandle args(memmgr->copyFrom(processStack, returnPoint, 
                      linkPointer - returnPoint));
                ObjectHandle temp(memmgr->copyFrom(processStack, linkPointer + 6,
                      methodTempSize(method)));
                contextObject = memmgr->newContext(linkPointer, method,
                    args,
                    temp);
                processStack->basicAtPut(linkPointer+1, contextObject);
                ipush(contextObject);
                /* save byte pointer then restore things properly */
                ObjectHandle temp2(memmgr->newInteger(byteOffset));
                processStack->basicAtPut(linkPointer+4, temp2);
                goto readLinkageBlock;

              }
              ipush(contextObject);
            }
            break;

          case nilConst: 
            ipush(nilobj);
            break;

          case trueConst:
            ipush(booleanSyms[booleanTrue]);
            break;

          case falseConst:
            ipush(booleanSyms[booleanFalse]);
            break;

          default:
            sysError("unimplemented constant","pushConstant");
        }
        break;

      case AssignInstance:
        receiverAtPut(low, stackTop());
        break;

      case AssignTemporary:
        temporaryAtPut(low, stackTop());
        break;

      case MarkArguments:
        returnPoint = (processStackTop() - low) + 1;
        timeSliceCounter++; /* make sure we do send */
        break;

      case SendMessage:
        messageToSend = literalsAt(low);

doSendMessage:
        arg = psb + (returnPoint-1);
#if !defined TW_SMALLINTEGER_AS_OBJECT
        if((argumentsAt(0) & 1) == 0)
#endif
          rcv = objectRef(argumentsAt(0)).sysMemPtr();
        methodClass = getClass(argumentsAt(0));

doFindMessage:
        /* look up method in cache */
        i = ((hashObject(messageToSend)) + (hashObject(methodClass))) % cacheSize;
        if ((methodCache[i].cacheMessage == messageToSend) &&
            (methodCache[i].lookupClass == methodClass)) {
          method = methodCache[i].cacheMethod;
          methodClass = methodCache[i].cacheClass;
//          printf("Cached method for %s on %s\n", objectRef(messageToSend).charPtr(), objectRef(objectRef(methodClass).basicAt(nameInClass)).charPtr());
        }
        else 
        {
          methodCache[i].lookupClass = methodClass;
          if(! findMethod(&methodClass)) 
          {
            /* not found, we invoke a smalltalk method */
            /* to recover */
            j = processStackTop() - returnPoint;
            argarray = memmgr->newArray(j+1);
            for (; j >= 0; j--) 
            {
              ipop(returnedObject);
              argarray->basicAtPut(j+1, returnedObject);
            }
//            printf("Failed to find %s (%s)\n", objectRef(messageToSend).charPtr(), objectRef(objectRef(methodClass).basicAt(nameInClass)).charPtr());
//            printf("Failed to find %s\n", messageToSend->charPtr());
            ipush(argarray->basicAt(1)); /* push receiver back */
            ipush(messageToSend);
            messageToSend = memmgr->newSymbol("message:notRecognizedWithArguments:");
            ipush(argarray);
            /* try again - if fail really give up */
            if (! findMethod(&methodClass)) {
              sysWarn("can't find","error recovery method");
              /* just quit */
              return false;
            }
          }
          methodCache[i].cacheMessage = messageToSend;
          methodCache[i].cacheMethod = method;
          methodCache[i].cacheClass = methodClass;
        }

        if (watching && (method->basicAt(watchInMethod) != nilobj)) 
        {
          /* being watched, we send to method itself */
          j = processStackTop() - returnPoint;
          argarray = memmgr->newArray(j+1);
          for (; j >= 0; j--) {
            ipop(returnedObject);
            argarray->basicAtPut(j+1, returnedObject);
          }
          ipush(method); /* push method */
          ipush(argarray);
          messageToSend = memmgr->newSymbol("watchWith:");
          /* try again - if fail really give up */
          methodClass = getClass(method);
          if (! findMethod(&methodClass)) {
            sysWarn("can't find","watch method");
            /* just quit */
            return false;
          }
        }

        /* save the current byte pointer */
        processStack->basicAtPut(linkPointer+4, memmgr->newInteger(byteOffset));

        /* make sure we have enough room in current process */
        /* stack, if not make stack larger */
        i = 7 + methodTempSize(method) + methodStackSize(method);
        j = processStackTop();
        if ((j + i) > processStack->size) 
        {
          processStack = growProcessStack(j, i);
          psb = processStack->sysMemPtr();
          pst = (psb + j);
          objectRef(aProcess).basicAtPut(stackInProcess, processStack);
        }

        byteOffset = 1;
        /* now make linkage area */
        /* position 0 : old linkage pointer */
        ipush(memmgr->newInteger(linkPointer));
        linkPointer = processStackTop();
        /* position 1 : context object (nil means stack) */
        ipush(nilobj);
        contextObject = processStack;
        cntx = psb;
        /* position 2 : return point */
        ipush(memmgr->newInteger(returnPoint));
        arg = cntx + (returnPoint-1);
        /* position 3 : method */
        ipush(method);
        /* position 4 : bytecode counter */
        ipush(memmgr->newInteger(byteOffset));
        /* then make space for temporaries */
        ipush(nilobj);
        temps = pst+1;
        pst += methodTempSize(method);
        /* break if we are too big and probably looping */
        if (processStack->size > 1800) timeSliceCounter = 0;
        goto readMethodInfo; 

      case SendUnary:
        /* do isNil and notNil as special cases, since */
        /* they are so common */
        if ((! watching) && (low <= 1)) {
          if (stackTop() == nilobj) {
            stackTopPut((low?booleanSyms[booleanFalse]:booleanSyms[booleanTrue]));
            break;
          }
        }
        returnPoint = processStackTop();
        messageToSend = unSyms[low];
        goto doSendMessage;
        break;

      case SendBinary:
        /* optimized as long as arguments are int */
        /* and conversions are not necessary */
        /* and overflow does not occur */
        primargs = pst - 1;
        if ((! watching) && (low <= 12) &&
            (getClass(primargs[0]) == CLASSOBJECT(Integer) && 
              getClass(primargs[1]) == CLASSOBJECT(Integer))) {
          returnedObject = primitive(low+60, primargs);
          if (returnedObject != nilobj) {
            // pop arguments off stack , push on result 
            stackTopFree();
            stackTopPut(returnedObject);
            break;
          }
        }
        /* else we do it the old fashion way */
        returnPoint = processStackTop() - 1;
        messageToSend = binSyms[low];
        goto doSendMessage;

      case DoPrimitive:
        /* low gives number of arguments */
        /* next byte is primitive number */
        primargs = (pst - low) + 1;
        /* next byte gives primitive number */
        i = nextByte();
        /* a few primitives are so common, and so easy, that
           they deserve special treatment */
        switch(i) {
          case 5:   /* set watch */
            watching = ! watching;
            returnedObject = watching?booleanSyms[booleanTrue]:booleanSyms[booleanFalse];
            break;

          case 11: /* class of object */
            returnedObject = getClass(*primargs);
            break;
          case 21: /* object equality test */
            if (*primargs == *(primargs+1))
              returnedObject = booleanSyms[booleanTrue];
            else returnedObject = booleanSyms[booleanFalse];
            break;
          case 25: /* basicAt: */
            j = getInteger(*(primargs+1));
            returnedObject = objectRef(*primargs).basicAt(j);
            break;
          case 31: /* basicAt:Put:*/
            j = getInteger(*(primargs+1));
            objectRef(*primargs).basicAtPut(j, *(primargs+2));
            returnedObject = nilobj;
            break;
          case 53: /* set time slice */
            timeSliceCounter = getInteger(*primargs);
            returnedObject = nilobj;
            break;
          case 58: /* allocObject */
            j = getInteger(*primargs);
            returnedObject = memmgr->allocObject(j);
            break;
          case 87: /* value of symbol */
            returnedObject = globalSymbol(objectRef(*primargs).charPtr());
            break;
          default: 
            returnedObject = primitive(i, primargs); break;
        }
        /* pop off arguments */
        while (low-- > 0) {
          stackTopFree();
        }
        ipush(returnedObject); 
        break;

doReturn:
        returnPoint = getInteger(processStack->basicAt(linkPointer + 2));
        linkPointer = getInteger(processStack->basicAt(linkPointer));
        while (processStackTop() >= returnPoint) {
          stackTopFree();
        }
        ipush(returnedObject); 
        /* now go restart old routine */
        if (linkPointer != 0)
          goto readLinkageBlock;
        else
          return false /* all done */;

      case DoSpecial:
        switch(low) {
          case SelfReturn:
            returnedObject = argumentsAt(0);
            goto doReturn;

          case StackReturn:
            ipop(returnedObject);
            goto doReturn;

          case Duplicate:
            /* avoid possible subtle bug */
            returnedObject = stackTop();
            ipush(returnedObject);
            break;

          case PopTop:
            ipop(returnedObject);
            break;

          case Branch:
            /* avoid a subtle bug here */
            i = nextByte();
            byteOffset = i;
            break;

          case BranchIfTrue:
            ipop(returnedObject);
            i = nextByte();
            if (returnedObject == booleanSyms[booleanTrue]) {
              /* leave nil on stack */
              pst++;
              byteOffset = i;
            }
            break;

          case BranchIfFalse:
            ipop(returnedObject);
            i = nextByte();
            if (returnedObject == booleanSyms[booleanFalse]) {
              /* leave nil on stack */
              pst++;
              byteOffset = i;
            }
            break;

          case AndBranch:
            ipop(returnedObject);
            i = nextByte();
            if (returnedObject == booleanSyms[booleanFalse]) {
              ipush(returnedObject);
              byteOffset = i;
            }
            break;

          case OrBranch:
            ipop(returnedObject);
            i = nextByte();
            if (returnedObject == booleanSyms[booleanTrue]) {
              ipush(returnedObject);
              byteOffset = i;
            }
            break;

          case SendToSuper:
            i = nextByte();
            messageToSend = literalsAt(i);
#if !defined TW_SMALLINTEGER_AS_OBJECT
            if((argumentsAt(0) & 1) == 0)
#endif
              rcv = objectRef(argumentsAt(0)).sysMemPtr();
            methodClass = method->basicAt(methodClassInMethod);
            /* if there is a superclass, use it
               otherwise for class Object (the only 
               class that doesn't have a superclass) use
               the class again */
            returnedObject = 
              objectRef(methodClass).basicAt(superClassInClass);
            if (returnedObject != nilobj)
              methodClass = returnedObject;
            goto doFindMessage;

          default:
            sysError("invalid doSpecial","");
            break;
        }
        break;

      default:
        sysError("invalid bytecode","");
        break;
    }
  }

  /* before returning we put back the values in the current process */
  /* object */

  processStack->basicAtPut(linkPointer+4, memmgr->newInteger(byteOffset));
  objectRef(aProcess).basicAtPut(stackTopInProcess, memmgr->newInteger(processStackTop()));
  objectRef(aProcess).basicAtPut(linkPtrInProcess, memmgr->newInteger(linkPointer));

  return true;
}


ObjectHandle sendMessageToObject(ObjectHandle receiver, const char* message, ObjectHandle* args, int cargs)
{
  MemoryManager* memmgr = MemoryManager::Instance();
  object methodClass = getClass(receiver);

  messageToSend = memmgr->newSymbol(message);
  if (! findMethod(&methodClass)) 
  {
    return ObjectHandle();
  }

  int linkOffset = 2 + cargs;
  int stackTop = 10 + cargs;

  // Create a new process
  ObjectHandle process = MemoryManager::Instance()->allocObject(processSize);
  // Create a stack for it
  ObjectHandle stack = MemoryManager::Instance()->newArray(50);
  process->basicAtPut(stackInProcess, stack);
  // Set the stack top to past the arguments, link and context data
  process->basicAtPut(stackTopInProcess, MemoryManager::Instance()->newInteger(stackTop));
  // Set the link pointer to past the arguments
  process->basicAtPut(linkPtrInProcess, MemoryManager::Instance()->newInteger(linkOffset));
  // Context is nil, meaning use the stack for context information
  stack->basicAtPut(contextInStack + cargs, nilobj);
  // Fill in the context data.
  stack->basicAtPut(methodInStack + cargs, method);
  stack->basicAtPut(returnpointInStack + cargs, MemoryManager::Instance()->newInteger(1));
  stack->basicAtPut(bytepointerInStack + cargs, MemoryManager::Instance()->newInteger(1));

  // Fill in the first argument, as the receiver.
  stack->basicAtPut(1, receiver);
  // And add the remainder of the arguments
  for(int i = 0; i < cargs; ++i)
    stack->basicAtPut(2 + i, args[i]);

  ObjectHandle saveProcessStack = processStack;
  int saveLinkPointer = linkPointer;
  while(execute(process, 15000));
  // Re-read the stack object, in case it had to grow during execution and 
  // was replaced.
  stack = process->basicAt(stackInProcess);
  object ro = stack->basicAt(1);
  processStack = saveProcessStack;
  linkPointer = saveLinkPointer;

  return ro;
}
