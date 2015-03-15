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
#include "memory.h"
#include "names.h"
#include "parser.h"

#include "linenoise.h"

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
      returnedObject = MemoryManager::Instance()->newInteger(system(
            objectRef(arguments[0]).charPtr()));
      break;

    case 1: /* editline, with history support */
      {
        char* command = linenoise(objectRef(arguments[0]).charPtr());
        returnedObject = MemoryManager::Instance()->newStString(command);
        linenoiseHistoryAdd(command);
        free(command);
      }
      break;

    case 2: /* get last error */ 
      {
        returnedObject = MemoryManager::Instance()->newStString(gLastError);
      }
      break;

    case 3: /* force garbage collect */
      {
        MemoryManager::Instance()->garbageCollect();
      }
      break;

    case 4:
      {
        returnedObject = MemoryManager::Instance()->newInteger(MemoryManager::Instance()->objectCount());
      }
      break;

    case 5:
      {
        returnedObject = MemoryManager::Instance()->newStString(MemoryManager::Instance()->statsString().c_str());
      }
      break;

    default:
      sysError("unknown primitive","sysPrimitive");
  }
  return(returnedObject);
}
