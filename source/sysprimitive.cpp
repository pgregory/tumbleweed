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
#include <dlfcn.h>

#include "env.h"
#include "memory.h"
#include "names.h"

extern boolean parseok;

static char gLastError[1024];

/* report a fatal system error */
noreturn sysError(const char* s1, const char* s2)
{
  ignore fprintf(stderr,"%s\n%s\n", s1, s2);
  ignore snprintf(gLastError, 1024, "%s\n%s\n", s1, s2);
  ignore abort();
}

/* report a nonfatal system error */
noreturn sysWarn(const char * s1, const char * s2)
{
  ignore fprintf(stderr,"%s\n%s\n", s1, s2);
}

noreturn compilWarn(const char* selector, const char* str1, const char* str2)
{
  ignore fprintf(stderr,"compiler warning: Method %s : %s %s\n", 
      selector, str1, str2);
}


noreturn compilError(const char* selector, const char* str1, const char* str2)
{
  ignore fprintf(stderr,"compiler error: Method %s : %s %s\n", 
      selector, str1, str2);
  ignore snprintf(gLastError, 1024, "compiler error: Method %s : %s %s", selector, str1, str2);
  parseok = false;
}

noreturn dspMethod(char* cp, char* mp)
{
  ignore fprintf(stderr,"%s %s\n", cp, mp);
}

void givepause()
{   char buffer[80];

  ignore fprintf(stderr,"push return to continue\n");
  ignore fgets(buffer,80,stdin);
}

object sysPrimitive(int number, object* arguments)
{   
  object returnedObject = nilobj;

  /* someday there will be more here */
  switch(number - 150) {
    case 0:     /* do a system() call */
      returnedObject = newInteger(system(
            objectRef(arguments[0]).charPtr()));
      break;

    case 1: /* readline, with history support */
/*      {
        char* p = readline(objectRef(arguments[0]).charPtr());
        add_history(p);
        returnedObject = newStString(p);
      }*/
      break;

    case 2: /* get last error */ 
      {
        returnedObject = newStString(gLastError);
      }
      break;

    case 3: /* force garbage collect */
      {
        garbageCollect(1);
      }
      break;

    default:
      sysError("unknown primitive","sysPrimitive");
  }
  return(returnedObject);
}
