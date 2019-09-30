/*
   Little Smalltalk, version 3
   Written by Tim Budd, January 1989

   tty interface routines
   this is used by those systems that have a bare tty interface
   systems using another interface, such as the stdwin interface
   will replace this file with another.
   */

#include <stdio.h>
#include <stdlib.h>

#include "env.h"
#include "objmemory.h"
#include "names.h"
#include "parser.h"

#include "linenoise.h"

extern char* unStrs[];
extern char* binStrs[];

static char gLastError[1024];

/* report a fatal system error */
void sysError(const char* s1, const char* s2)
{
  fprintf(stderr,"%s\n%s\n", s1, s2);
  //_snprintf(gLastError, 1024, "%s\n%s\n", s1, s2);
  abort();
}

/* report a nonfatal system error */
void sysWarn(const char * s1, const char * s2)
{
  fprintf(stderr,"%s\n%s\n", s1, s2);
}

void dspMethod(char* cp, char* mp)
{
  fprintf(stderr,"%s %s\n", cp, mp);
}

void disassembleMethod(object method)
{
    ObjectStruct& methodObj = objectRef(method);
    ObjectStruct& bytecodes = objectRef(methodObj.basicAt(bytecodesInMethod));
    long size = -bytecodes.size;
    
    fprintf(stderr, "Disassemble Method: %s\n", objectRef(methodObj.basicAt(messageInMethod)).charPtr());
    fprintf(stderr, "  size: %ld\n", size);

    byte* bp = bytecodes.bytePtr();
    int byteOffset = 0;
    byte low, high, i;

    while(byteOffset < size) {
        low = (high = bp[byteOffset++]) & 0x0F;
        high >>= 4;
        if (high == 0) {
          high = low;
          low = bp[byteOffset++];
        }
        switch(high) {
          case PushInstance:
                fprintf(stderr, "    PushInstance %d\n", low);
                //ipush(receiverAt(low));
                break;

          case PushArgument:
                fprintf(stderr, "    PushArgument %d\n", low);
                //ipush(argumentsAt(low));
                break;

          case PushTemporary:
                fprintf(stderr, "    PushTemporary %d\n", low);
                //ipush(temporaryAt(low));
                break;

          case PushLiteral:
                fprintf(stderr, "    PushLiteral %d\n", low);
                //ipush(literalsAt(low));
                break;

          case PushConstant:
                fprintf(stderr, "    PushConstant ");
                switch(low) {
                    case 0: case 1: case 2:
                        fprintf(stderr, "%d\n", low);
                        //ipush(newInteger(low));
                        break;

                    case minusOne:
                        fprintf(stderr, "-1\n");
                        //ipush(newInteger(-1));
                        break;

                    case contextConst:
                        fprintf(stderr, " CONTEXT\n");
                        break;

                    case nilConst:
                        fprintf(stderr, "nil\n");
                        //ipush(nilobj);
                        break;

                    case trueConst:
                        fprintf(stderr, "true\n");
                        //ipush(booleanSyms[booleanTrue]);
                        break;

                    case falseConst:
                        fprintf(stderr, "false\n");
                        //ipush(booleanSyms[booleanFalse]);
                        break;

                    default:
                        fprintf(stderr, "ERROR UNIMPLEMENTED CONSTANT %d\n", low);
                        //sysError("unimplemented constant","pushConstant");
                }
                break;

          case AssignInstance:
                fprintf(stderr, "    AssignInstance %d\n", low);
                //receiverAtPut(low, stackTop());
                break;

          case AssignTemporary:
                fprintf(stderr, "    AssignTemporary %d\n", low);
                //temporaryAtPut(low, stackTop());
                break;

          case MarkArguments:
                fprintf(stderr, "    MarkArguments %d\n", low);
                //returnPoint = (processStackTop() - low) + 1;
                //timeSliceCounter++; /* make sure we do send */
                break;

          case SendMessage:
                fprintf(stderr, "    SendMessage %d\n", low);
                break;

          case SendUnary:
                fprintf(stderr, "    SendUnary %s\n", unStrs[low]);
                break;

          case SendBinary:
                fprintf(stderr, "    SendBinary %s\n", binStrs[low]);
                break;

          case DoPrimitive:
                i = bp[byteOffset++];
                fprintf(stderr, "    DoPrimitive %d %d\n", i, low);
                break;

          case DoSpecial:
                fprintf(stderr, "    DoSpecial ");
                switch(low) {
                    case SelfReturn:
                        fprintf(stderr, "SelfReturn\n");
                        break;

                    case StackReturn:
                        fprintf(stderr, "StackReturn\n");
                        break;

                    case Duplicate:
                        fprintf(stderr, "Duplicate\n");
                        break;

                    case PopTop:
                        fprintf(stderr, "PopTop\n");
                        break;

                    case Branch:
                        i = bp[byteOffset++];
                        fprintf(stderr, "Branch %d\n", i);
                        break;

                    case BranchIfTrue:
                        i = bp[byteOffset++];
                        fprintf(stderr, "BranchIfTrue %d\n", i);
                        break;

                    case BranchIfFalse:
                        i = bp[byteOffset++];
                        fprintf(stderr, "BranchIfFalse %d\n", i);
                        break;

                    case AndBranch:
                        i = bp[byteOffset++];
                        fprintf(stderr, "AndBranch %d\n", i);
                        break;

                    case OrBranch:
                        i = bp[byteOffset++];
                        fprintf(stderr, "OrBranch %d\n", i);
                        break;

                    case SendToSuper:
                        i = bp[byteOffset++];
                        fprintf(stderr, "SendToSuper %d\n", i);
                        //messageToSend = literalsAt(i);

                    default:
                        fprintf(stderr, "INVALID DOSPECIAL %d\n", low);
                        //sysError("invalid doSpecial","");
                        break;
                }
                break;

          default:
                fprintf(stderr, "    INVALID BYTECODE %d\n", high);
                //sysError("invalid bytecode","");
                break;
        }
    }
}

void givepause()
{   
  char buffer[80];

  fprintf(stderr,"push return to continue\n");
  fgets(buffer,80,stdin);
}

object sysPrimitive(int number, object* arguments)
{   
  ObjectHandle returnedObject = nilobj;

  /* someday there will be more here */
  switch(number - 150) {
    case 0:     /* do a system() call */
      returnedObject = newInteger(system(
            objectRef(arguments[0]).charPtr()));
      break;

    case 1: /* editline, with history support */
      {
        char* command = linenoise_nb(objectRef(arguments[0]).charPtr());
        returnedObject = newStString(command);
        if(command) 
        {
          linenoiseHistoryAdd(command);
          free(command);
        }
      }
      break;

    case 2: /* get last error */ 
      {
        returnedObject = newStString(gLastError);
      }
      break;

    case 3: /* force garbage collect */
      {
        MemoryManager::Instance()->garbageCollect();
      }
      break;

    case 4:
      {
        returnedObject = newInteger(MemoryManager::Instance()->objectCount());
      }
      break;

    case 5:
      {
        returnedObject = newStString(MemoryManager::Instance()->statsString().c_str());
      }
      break;

    case 6:
      {
          disassembleMethod(arguments[0]);
          returnedObject = nilobj;
      }
      break;

    default:
      sysError("unknown primitive","sysPrimitive");
  }
  return(returnedObject);
}
