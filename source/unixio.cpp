/*
   Little Smalltalk, version 2

   Unix specific input and output routines
   written by tim budd, January 1988
   */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "env.h"
#include "memory.h"
#include "names.h"

void fileIn(FILE* fd, boolean printit);

struct SDummy
{
  int di;
  object cl;
  short ds;
} dummyObject;

/*
   imageRead - read in an object image
   we toss out the free lists built initially,
   reconstruct the linkages, then rebuild the free
   lists around the new objects.
   The only objects with nonzero reference counts
   will be those reachable from either symbols
   */
static int fr(FILE* fp, char* p, int s)
{   
  int r;

  r = fread(p, s, 1, fp);
  if (r && (r != 1))
    sysError("imageRead count error","");
  return r;
}

void imageRead(FILE* fp)
{   
  long i, size;

  ignore fr(fp, (char *) &symbols, sizeof(object));
  i = 0;

  while(fr(fp, (char *) &dummyObject, sizeof(dummyObject))) 
  {
    i = dummyObject.di;

    if ((i < 0) || (i > ObjectTableMax))
      sysError("reading index out of range","");
    theMemoryManager->objectFromID(i)._class = dummyObject.cl;
    if ((theMemoryManager->objectFromID(i)._class < 0) || 
        ((theMemoryManager->objectFromID(i)._class) > ObjectTableMax)) {
      fprintf(stderr,"index %d\n", static_cast<int>(dummyObject.cl));
      sysError("class out of range","imageRead");
    }
    theMemoryManager->objectFromID(i).size = size = dummyObject.ds;
    if (size < 0) size = ((- size) + 1) / 2;
    if (size != 0) {
      theMemoryManager->objectFromID(i).memory = mBlockAlloc((int) size);
      ignore fr(fp, (char *) theMemoryManager->objectFromID(i).memory,
          sizeof(object) * (int) size);
    }
    else
      theMemoryManager->objectFromID(i).memory = (object *) 0;

        theMemoryManager->objectFromID(i).referenceCount = 666;
  }
  theMemoryManager->setFreeLists();
}

/*
   imageWrite - write out an object image
   */

static void fw(FILE* fp, char* p, int s)
{
  if (fwrite(p, s, 1, fp) != 1) {
    sysError("imageWrite size error","");
  }
}

void imageWrite(FILE* fp)
{   
  long i, size;

  fprintf(stderr, "Object count before GC: %d\n", objectCount());
  theMemoryManager->garbageCollect(false);
  fprintf(stderr, "Object count after GC: %d\n", objectCount());

  fw(fp, (char *) &symbols, sizeof(object));

  for (i = 0; i < ObjectTableMax; i++) {
    if (theMemoryManager->objectFromID(i).referenceCount > 0) {
      dummyObject.di = i;
      dummyObject.cl = theMemoryManager->objectFromID(i)._class;
      dummyObject.ds = size = theMemoryManager->objectFromID(i).size;
      fw(fp, (char *) &dummyObject, sizeof(dummyObject));
      if (size < 0) size = ((- size) + 1) / 2;
      if (size != 0)
        fw(fp, (char *) theMemoryManager->objectFromID(i).memory,
            sizeof(object) * size);
    }
  }
}

/* i/o primitives - necessarily rather UNIX dependent;
   basically, files are all kept in a large array.
   File operations then just give an index into this array 
   */
# define MAXFILES 20
/* we assume this is initialized to NULL */
static FILE *fp[MAXFILES];

object ioPrimitive(int number, object* arguments)
{   
  int i, j;
  char *p, buffer[1024];
  object returnedObject;

  returnedObject = nilobj;

  i = intValue(arguments[0]);

  switch(number) {
    case 0:     /* file open */
      i = intValue(arguments[0]);
      p = objectRef(arguments[1]).charPtr();
      if(NULL == p)
        returnedObject = nilobj;
      else 
      {
        if (streq(p, "stdin")) 
          fp[i] = stdin;
        else if (streq(p, "stdout"))
          fp[i] = stdout;
        else if (streq(p, "stderr"))
          fp[i] = stderr;
        else {
          fp[i] = fopen(p, objectRef(arguments[2]).charPtr());
        }
        if (fp[i] == NULL)
          returnedObject = nilobj;
        else
          returnedObject = newInteger(i);
      }
      break;

    case 1:     /* file close - recover slot */
      if (fp[i]) ignore fclose(fp[i]);
      fp[i] = NULL;
      break;

    case 2:     /* file size */
    case 3:     /* file in */
      if (fp[i]) fileIn(fp[i], false);
      break;

    case 4:     /* get character */
      sysError("file operation not implemented yet","");

    case 5:     /* get string */
      if (! fp[i]) break;
      j = 0; buffer[j] = '\0';
      while (1) {
        if (fgets(&buffer[j], 512, fp[i]) == NULL)
          return(nilobj); /* end of file */
        if (fp[i] == stdin) {
          /* delete the newline */
          j = strlen(buffer);
          if (buffer[j-1] == '\n')
            buffer[j-1] = '\0';
        }
        j = strlen(buffer)-1;
        if (buffer[j] != '\\')
          break;
        /* else we loop again */
      }
      returnedObject = newStString(buffer);
      break;

    case 7:     /* write an object image */
      if (fp[i]) imageWrite(fp[i]);
      returnedObject = trueobj;
      break;

    case 8:     /* print no return */
    case 9:     /* print string */
      if (! fp[i]) break; 
      ignore fputs(objectRef(arguments[1]).charPtr(), fp[i]);
      if (number == 8)
        ignore fflush(fp[i]);
      else
        ignore fputc('\n', fp[i]);
      break;

    default:
      sysError("unknown primitive","filePrimitive");
  }

  return(returnedObject);
}
