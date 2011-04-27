/*
	Little Smalltalk, version 3
	Written by Tim Budd, Oregon State University, June 1988

	routines used in reading in textual descriptions of classes
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "env.h"
#include "memory.h"
#include "names.h"
#include "lex.h"

# define MethodTableSize 39

/*
	the following are switch settings, with default values
*/
boolean savetext = true;

/*
	we read the input a line at a time, putting lines into the following
	buffer.  In addition, all methods must also fit into this buffer.
*/
# define TextBufferSize 16384
static char textBuffer[TextBufferSize];

/*
	findClass gets a class object,
	either by finding it already or making it
	in addition, it makes sure it has a size, by setting
	the size to zero if it is nil.
*/
static object findClass(char* name)
{	
  object newobj;

	newobj = globalSymbol(name);
	if (newobj == nilobj)
		newobj = newClass(name);
	if (basicAt(newobj, sizeInClass) == nilobj) 
  {
		basicAtPut(newobj, sizeInClass, newInteger(0));
  }
	return newobj;
}

static object findClassWithMeta(char* name, object metaObj)
{	
  object newObj, nameObj, methTable;
  int size;

	newObj = globalSymbol(name);
	if (newObj == nilobj)
  {
    size = intValue(basicAt(metaObj, sizeInClass));
    newObj = allocObject(size);
    setClass(newObj, metaObj);

    /* now make name */
    nameObj = newSymbol(name);
    basicAtPut(newObj, nameInClass, nameObj);
    methTable = newDictionary(39);
    basicAtPut(newObj, methodsInClass, methTable);
    basicAtPut(newObj, sizeInClass, newInteger(size));

    /* now put in global symbols table */
    nameTableInsert(symbols, strHash(name), nameObj, newObj);
  }
	return newObj;
}

/*
 * Create raw class
 */

static object createRawClass(char* class, char* metaclass, char* superclass)
{
  object classObj, superObj, metaObj;
	int i, size, instanceTop;

  metaObj = findClass(metaclass);
	classObj = findClassWithMeta(class, metaObj);
  setClass(classObj, metaObj);

  //printf("RAWCLASS %s %s %s\n", class, metaclass, superclass);

	size = 0;
  superObj = nilobj;
  if(NULL != superclass)
  {
    superObj = findClass(superclass);
    basicAtPut(classObj, superClassInClass, superObj);
    size = intValue(basicAt(superObj, sizeInClass));
  }

  // Set the size up to now.
	basicAtPut(classObj, sizeInClass, newInteger(size));

  return classObj;
}

/*
	readRawDeclaration reads a declaration of a class
*/
static readRawClassDeclaration()
{	
  object classObj, vars;
  char* className, *metaName, *superName;
	int i, size, instanceTop;
  // todo: fixed length variables array!
	object instanceVariables[15];

	if (nextToken() != nameconst)
		sysError("bad file format","no name in declaration");
  className = strdup(tokenString);
	size = 0;
	if (nextToken() == nameconst) 
  {	/* read metaclass name */
		metaName = strdup(tokenString);
		ignore nextToken();
  }
	if (token == nameconst) 
  {	/* read superclass name */
		superName = strdup(tokenString);
		ignore nextToken();
  }

  classObj = createRawClass(className, metaName, superName);
  free(className);
  free(metaName);
  free(superName);

  // Get the current class size, we'll build on this as 
  // we add instance variables.
  size = intValue(basicAt(classObj, sizeInClass));

	if (token == nameconst) 
  {		/* read instance var names */
		instanceTop = 0;
		while (token == nameconst) 
    {
			instanceVariables[instanceTop++] = newSymbol(tokenString);
			size++;
			ignore nextToken();
    }
		vars = newArray(instanceTop);
		for (i = 0; i < instanceTop; i++) 
    {
			basicAtPut(vars, i+1, instanceVariables[i]);
    }
		basicAtPut(classObj, variablesInClass, vars);
  }
	basicAtPut(classObj, sizeInClass, newInteger(size));
  basicAtPut(classObj, methodsInClass, newDictionary(39));

  object methodsClass = getClass(basicAt(classObj, methodsInClass));
}

/*
	readDeclaration reads a declaration of a class
*/
static readClassDeclaration()
{	
  object classObj, metaObj, vars;
  char* className, *superName;
	int i, size, instanceTop;
  // todo: fixed length variables array!
	object instanceVariables[15];
  // todo: horrible fixed length arrays!
  char metaClassName[100];
  char metaSuperClassName[100];

  className = superName = NULL;

	if (nextToken() != nameconst)
		sysError("bad file format","no name in declaration");
  className = strdup(tokenString);
	if (nextToken() == nameconst) 
  {	/* read superclass name */
		superName = strdup(tokenString);
		ignore nextToken();
  }
  // todo: sprintf eradication!
  sprintf(metaClassName, "Meta%s", className);
  if(NULL != superName)
    sprintf(metaSuperClassName, "Meta%s", superName);
  else
    sprintf(metaSuperClassName, "Class");

  metaObj = createRawClass(metaClassName, "Class", metaSuperClassName);
  classObj = createRawClass(className, metaClassName, superName);
  setClass(classObj, metaObj);

  size = intValue(basicAt(metaObj, sizeInClass));
  instanceVariables[0] = newSymbol("theInstance");
  vars = newArray(1);
  basicAtPut(vars, 1, instanceVariables[0]);
  basicAtPut(metaObj, variablesInClass, vars);
	basicAtPut(metaObj, sizeInClass, newInteger(size+1));

  // Get the current class size, we'll build on this as 
  // we add instance variables.
  size = intValue(basicAt(classObj, sizeInClass));

	if (token == nameconst) 
  {		/* read instance var names */
		instanceTop = 0;
		while (token == nameconst) 
    {
			instanceVariables[instanceTop++] = newSymbol(tokenString);
			size++;
			ignore nextToken();
    }
		vars = newArray(instanceTop);
		for (i = 0; i < instanceTop; i++) 
    {
			basicAtPut(vars, i+1, instanceVariables[i]);
    }
		basicAtPut(classObj, variablesInClass, vars);
  }
	basicAtPut(classObj, sizeInClass, newInteger(size));
  basicAtPut(classObj, methodsInClass, newDictionary(39));

  free(className);
  free(superName);
}

/*
	readClass reads a class method description
*/
static readMethods(fd, printit)
FILE *fd;
boolean printit;
{	object classObj, methTable, theMethod, selector;
# define LINEBUFFERSIZE 16384
	char *cp, *eoftest, lineBuffer[LINEBUFFERSIZE];
  object protocol;

  protocol = nilobj;
	if (nextToken() != nameconst)
		sysError("missing name","following Method keyword");
	classObj = findClass(tokenString);
	setInstanceVariables(classObj);
	if (printit)
		cp = charPtr(basicAt(classObj, nameInClass));

	/* now find or create a method table */
	methTable = basicAt(classObj, methodsInClass);
	if (methTable == nilobj) {	/* must make */
		methTable = newDictionary(MethodTableSize);
		basicAtPut(classObj, methodsInClass, methTable);
		}

  if(nextToken() == strconst) {
    protocol = newStString(tokenString);
  }

	/* now go read the methods */
	do {
		if (lineBuffer[0] == '|')	/* get any left over text */
			strcpy(textBuffer,&lineBuffer[1]);
		else
			textBuffer[0] = '\0';
		while((eoftest = fgets(lineBuffer, LINEBUFFERSIZE, fd)) != NULL) {
			if ((lineBuffer[0] == '|') || (lineBuffer[0] == ']'))
				break;
			ignore strcat(textBuffer, lineBuffer);
			}
		if (eoftest == NULL) {
			sysError("unexpected end of file","while reading method");
			break;
			}

		/* now we have a method */
		theMethod = newMethod();
		if (parse(theMethod, textBuffer, savetext)) {
			selector = basicAt(theMethod, messageInMethod);
			basicAtPut(theMethod, methodClassInMethod, classObj);
      basicAtPut(theMethod, protocolInMethod, protocol);
			if (printit)
				dspMethod(cp, charPtr(selector));
			nameTableInsert(methTable, (int) selector, selector, theMethod);
			}
		else {
			/* get rid of unwanted method */
			incr(theMethod);
			decr(theMethod);
			givepause();
			}
		
	} while (lineBuffer[0] != ']');
}

/*
	fileIn reads in a module definition
*/
fileIn(fd, printit)
FILE *fd;
boolean printit;
{
	while(fgets(textBuffer, TextBufferSize, fd) != NULL) {
		lexinit(textBuffer);
		if (token == inputend)
			; /* do nothing, get next line */
		else if ((token == binary) && streq(tokenString, "*"))
			; /* do nothing, its a comment */
		else if ((token == nameconst) && streq(tokenString, "RawClass"))
			readRawClassDeclaration();
		else if ((token == nameconst) && streq(tokenString, "Class"))
			readClassDeclaration();
		else if ((token == nameconst) && streq(tokenString,"Methods"))
			readMethods(fd, printit);
		else 
			sysError("unrecognized line", textBuffer);
		}
}
