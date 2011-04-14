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
#include <editline/readline.h>
#include <dlfcn.h>
#include <assert.h>
#include <ffi.h>
#include <limits.h>

#include "env.h"
#include "memory.h"
#include "names.h"

typedef void* FFI_LibraryHandle;
typedef void* FFI_FunctionHandle;

typedef struct FFI_Lib_S {
  FFI_LibraryHandle handle;
  int                 numFunctions;
  FFI_FunctionHandle* functions;
  struct FFI_Lib_S*   next;
} FFI_Lib;

typedef union FFI_DataType_U {
  char* charPtr;
  int   integer;
  float _float;
  struct {
    void* pointer;
    int integer;
  } outInteger;
} FFI_DataType;

static FFI_Lib* gLibraries = NULL;

typedef enum FFI_Symbols_E {
  FFI_CHAR,
  FFI_CHAR_OUT,
  FFI_STRING,
  FFI_STRING_OUT,
  FFI_SYMBOL,
  FFI_SYMBOL_OUT,
  FFI_INT,
  FFI_UINT,
  FFI_LONG,
  FFI_ULONG,
  FFI_DOUBLE,
  FFI_LONGDOUBLE,
  FFI_VOID,
  FFI_WCHAR,
  FFI_WSTRING,
  FFI_WSTRING_OUT,
  FFI_COBJECT,
  FFI_SMALLTALK,
} FFI_Symbols;


static char* ffiStrs[] = {
  "char",               // FFI_CHAR,       
  "charOut",            // FFI_CHAR_OUT,
  "string",             // FFI_STRING,
  "stringOut",          // FFI_STRING_OUT,
  "symbol",             // FFI_SYMBOL,
  "symbolOut",          // FFI_SYMBOL_OUT,
  "int",                // FFI_INT,
  "uInt",               // FFI_UINT,
  "long",               // FFI_LONG,
  "uLong",              // FFI_ULONG,
  "double",             // FFI_DOUBLE,
  "longDouble",         // FFI_LONGDOUBLE,
  "void",               // FFI_VOID,
  "wchar",              // FFI_WCHAR,
  "wstring",            // FFI_WSTRING,
  "wstringOut",         // FFI_WSTRING_OUT
  "cObject",            // FFI_COBJECT,
  "smalltalk"           // FFI_SMALLTALK,
};
static int ffiNumStrs = sizeof(ffiStrs)/sizeof(ffiStrs[0]);
static object *ffiSyms;
static void* ffiLSTTypes[] = {
  &ffi_type_uchar,      // FFI_CHAR,
  &ffi_type_pointer,    // FFI_CHAR_OUT,
  &ffi_type_pointer,    // FFI_STRING,
  &ffi_type_pointer,    // FFI_STRING_OUT,
  &ffi_type_pointer,    // FFI_SYMBOL,
  &ffi_type_pointer,    // FFI_SYMBOL_OUT,
  &ffi_type_sint16,     // FFI_INT,
  &ffi_type_uint16,     // FFI_UINT,
  &ffi_type_sint32,     // FFI_LONG,
  &ffi_type_uint32,     // FFI_ULONG,
  &ffi_type_double,     // FFI_DOUBLE,
  &ffi_type_longdouble, // FFI_LONGDOUBLE,
  &ffi_type_void,       // FFI_VOID,
  &ffi_type_uint16,     // FFI_WCHAR,
  &ffi_type_pointer,    // FFI_WSTRING,
  &ffi_type_pointer,    // FFI_WSTRING_OUT
  &ffi_type_pointer,    // FFI_COBJECT,
  &ffi_type_pointer     // FFI_SMALLTALK,
};

void initFFISymbols()
{
  ffiSyms = calloc(ffiNumStrs, sizeof(object));
  int i;
  for(i = 0; i < ffiNumStrs; ++i)
    ffiSyms[i] = newSymbol(ffiStrs[i]);
}

void cleanupFFISymbols()
{
  if(NULL != ffiSyms)
    free(ffiSyms);
}


int mapType(object symbol)
{
  int i;
  for(i = 0; i < ffiNumStrs; ++i)
  {
    if(ffiSyms[i] == symbol)
      return i;
  }
  return FFI_VOID;
}

void* valueOut(int argMap, object value, FFI_DataType* data)
{
  void* ptr;
  object realValue;

  object sclass = globalSymbol("Symbol");
  object vclass = getClass(value);
  if(vclass == sclass)
    realValue = globalSymbol(charPtr(value));
  else
    realValue = value;
  switch(argMap)
  {
    case FFI_CHAR_OUT:
      data->outInteger.pointer = &data->outInteger.integer;
      data->outInteger.integer = intValue(realValue);
      ptr = &data->outInteger.pointer;
      break;
    case FFI_CHAR:
      data->integer = intValue(realValue);
      ptr = &data->integer;
      break;
    case FFI_STRING:
    case FFI_STRING_OUT:
    case FFI_SYMBOL:
    case FFI_SYMBOL_OUT:
      data->charPtr = charPtr(realValue);
      ptr = &data->charPtr;
      break;
    case FFI_INT:
    case FFI_UINT:
    case FFI_LONG:
    case FFI_ULONG:
      if(isInteger(realValue))
        data->integer = intValue(realValue);
      ptr = &data->integer;
      break;
    case FFI_DOUBLE:
    case FFI_LONGDOUBLE:
      data->_float = floatValue(realValue);
      ptr = &data->_float;
      break;
    case FFI_VOID:
      break;
    case FFI_WCHAR:
      break;
    case FFI_WSTRING:
      break;
    case FFI_WSTRING_OUT:
      break;
    case FFI_COBJECT:
      break;
    case FFI_SMALLTALK:
      break;
  }
  return ptr;
}


object valueIn(int retMap, FFI_DataType* data)
{
  switch(retMap)
  {
    case FFI_CHAR_OUT:
      return newInteger(data->outInteger.integer);
      break;

    case FFI_CHAR:
      return newChar(data->integer);
      break;

    case FFI_STRING:
      return newStString(data->charPtr);
      break;

    case FFI_INT:
      return newInteger(data->integer);
      break;

    case FFI_VOID:
      return nilobj;
      break;
  }
}

static int addLibrary(FFI_LibraryHandle handle)
{
  if(NULL == gLibraries)
  {
    gLibraries = calloc(1, sizeof(FFI_Lib));
    gLibraries[0].handle = handle;
    gLibraries[0].functions = NULL;
    gLibraries[0].next = NULL;

    return 0;
  }
  else
  {
    FFI_Lib* newLib = calloc(1, sizeof(FFI_Lib));
    newLib->handle = handle;
    newLib->numFunctions = 0;
    newLib->functions = NULL;
    newLib->next = NULL;
    FFI_Lib* last = gLibraries;
    int id = 0;
    while(last->next != NULL)
      last = last->next;
    last->next = newLib;

    return id + 1;
  }
}

static int addFunction(FFI_Lib* lib, FFI_FunctionHandle handle)
{
  assert(NULL != lib && NULL != handle);
  if(NULL == lib->functions)
  {
    lib->functions = calloc(1, sizeof(FFI_FunctionHandle));
    lib->functions[0] = handle;
    lib->numFunctions = 1;
    return 0;
  }
  else
  {
    lib->functions = realloc(lib->functions, (lib->numFunctions+1 * sizeof(FFI_FunctionHandle)));
    lib->functions[lib->numFunctions] = handle;
    lib->numFunctions++;
    return lib->numFunctions-1;
  }
}

static FFI_Lib* getLibrary(int id)
{
  int i = id;
  FFI_Lib* lib = gLibraries;
  while(i-- > 0)
  {
    if(NULL == lib->next)
      return NULL;
    lib = lib->next;
  }

  return lib;
}

object ffiPrimitive(int number, object* arguments)
{	
  object returnedObject = nilobj;
  char libName[MAX_PATH];

  switch(number - 180) {
    case 0: /* dlopen */
      {
        char* p = charPtr(arguments[0]);
        sprintf(libName, "%s.%s", p, SO_EXT);
        void* handle = dlopen(libName, RTLD_LAZY);
        if(NULL != handle)
        {
          int id = addLibrary(handle);
          returnedObject = newInteger(id);
        }
        else 
        {
          sysWarn("library not found","ffiPrimitive");
          returnedObject = newInteger(-1);
        }
      }
      break;

    case 1: /* dlsym */
      {
        if(isInteger(arguments[0]))
        {
          int id = intValue(arguments[0]);
          char* p = charPtr(arguments[1]);
          FFI_Lib* lib = getLibrary(id);
          if(NULL != lib)
          {
            FFI_FunctionHandle func = dlsym(lib->handle, p);
            if(NULL != func)
            {
              int fid = addFunction(lib, func);
              returnedObject = newInteger(fid);
            }
            else
            {
              sysWarn("function not found in library", "ffiPrimitive");
              returnedObject = newInteger(-1);
            }
          }
        }
        else
          sysError("invalid library handle", "ffiPrimitive"); 
      }
      break;

     /* 
      *  cCall <lib_id> <func_id> <#return_type> <#(#arg_type ...)> <#(args ...)>
      */
    case 2: /* cCall */
      {
        int lib_id = intValue(arguments[0]);
        int func_id = intValue(arguments[1]);
        object rtype = arguments[2];
        int retMap = mapType(rtype);
        int cargTypes = sizeField(arguments[3]);
        int cargs = sizeField(arguments[4]);
        int cOutArgs = 0;

        assert(cargTypes == cargs);

        FFI_Lib* lib = getLibrary(lib_id);
        if(NULL != lib)
        {
          if(func_id < lib->numFunctions)
          {
            ffi_type** args = NULL;
            void** values = NULL;
            FFI_DataType* dataValues = NULL; 
            if(cargs > 0)
            {
              args = calloc(cargTypes, sizeof(ffi_type));
              values = calloc(cargs, sizeof(void*));
              dataValues = calloc(cargs, sizeof(FFI_DataType));

              int i;
              for(i = 0; i < cargTypes; ++i)
              {
                object argType = basicAt(arguments[3], i+1);
                int argMap = mapType(argType);
                args[i] = ffiLSTTypes[argMap];
                values[i] = valueOut(argMap, basicAt(arguments[4], i+1), &dataValues[i]);
              }
            }

            ffi_type* ret;
            ret = ffiLSTTypes[retMap]; 
            ffi_type retVal;
            void* retData = &retVal;

            ffi_cif cif;
            if(ffi_prep_cif(&cif, FFI_DEFAULT_ABI, cargs, ret, args) == FFI_OK)
            {
              returnedObject = newArray(cargs + 1);

              ffi_call(&cif, lib->functions[func_id], retData, values);
              basicAtPut(returnedObject, 1, valueIn(retMap, retData));
            }
            // Now fill in the rest of the result array with the arguments
            // thus getting any 'out' values back to the system in a controlled 
            // way.
            // Note: All arguments are passed back, irrespective of if they 
            // are out or not, they will just be unchanged if not. This makes it
            // simpler to index them on the other end.
            if(cargs > 0)
            {
              int i;
              for(i = 0; i < cargs; ++i)
              {
                object argType = basicAt(arguments[3], i+1);
                int argMap = mapType(argType);
                // Get a new value out of the arguments array.
                object newVal = valueIn(argMap, values[i]);
                basicAtPut(returnedObject, i+2, newVal); 
              }
            }
          }
        }
      }
      break;

    default:
      sysError("unknown primitive","ffiPrimitive");
  }
  return(returnedObject);
}

