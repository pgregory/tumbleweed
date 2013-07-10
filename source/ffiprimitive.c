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
#include <string.h>
#if !defined WIN32
#include <dlfcn.h>
#include <memory.h>
#else
#include <Windows.h>
#endif
#include <assert.h>
#include <ffi.h>
#include <limits.h>

#include "env.h"
#include "memory.h"
#include "names.h"
#include "interp.h"

extern object sendMessageToObject(object receiver, const char* message, object* args, int cargs);

extern object processStack;
extern int linkPointer;

typedef void* FFI_LibraryHandle;
typedef void* FFI_FunctionHandle;

typedef struct FFI_DataType_U
{
  int typemap;
  ffi_type* type;
  void* ptr;
  union {
    char* charPtr;
    int   integer;
    void* cPointer;
    float _float;
    double _double;
    struct 
    {
      void* pointer;
      int integer;
    } outInteger;
    struct 
    {
      void* pointer;
      float _float;
    } outFloat;
    struct 
    {
      void* pointer;
      double _double;
    } outDouble;
    struct
    {
      ffi_type* type;
      object _class;
      struct FFI_DataType_U* members;
      int numMembers;
      size_t size;
      void* pointer;
    } externalData;
  };
} FFI_DataType;

typedef enum FFI_Symbols_E 
{
  FFI_CHAR,
  FFI_CHAR_OUT,
  FFI_STRING,
  FFI_STRING_OUT,
  FFI_SYMBOL,
  FFI_SYMBOL_OUT,
  FFI_INT,
  FFI_INT_OUT,
  FFI_UINT,
  FFI_UINT_OUT,
  FFI_LONG,
  FFI_LONG_OUT,
  FFI_ULONG,
  FFI_ULONG_OUT,
  FFI_FLOAT,
  FFI_FLOAT_OUT,
  FFI_DOUBLE,
  FFI_DOUBLE_OUT,
  FFI_LONGDOUBLE,
  FFI_LONGDOUBLE_OUT,
  FFI_VOID,
  FFI_WCHAR,
  FFI_WCHAR_OUT,
  FFI_WSTRING,
  FFI_WSTRING_OUT,
  FFI_COBJECT,
  FFI_SMALLTALK,
  FFI_EXTERNALDATA,
  FFI_EXTERNALDATAOUT,
} FFI_Symbols;


static const char* ffiStrs[] = 
{
  "char",               // FFI_CHAR,       
  "charOut",            // FFI_CHAR_OUT,
  "string",             // FFI_STRING,
  "stringOut",          // FFI_STRING_OUT,
  "symbol",             // FFI_SYMBOL,
  "symbolOut",          // FFI_SYMBOL_OUT,
  "int",                // FFI_INT,
  "intOut",             // FFI_INT_OUT,
  "uInt",               // FFI_UINT,
  "uIntOut",            // FFI_UINT_OUT,
  "long",               // FFI_LONG,
  "longOut",            // FFI_LONG_OUT,
  "uLong",              // FFI_ULONG,
  "uLongOut",           // FFI_ULONG_OUT,
  "float",              // FFI_FLOAT,
  "floatOut",           // FFI_FLOAT_OUT,
  "double",             // FFI_DOUBLE,
  "doubleOut",          // FFI_DOUBLE_OUT,
  "longDouble",         // FFI_LONGDOUBLE,
  "longDoubleOut",      // FFI_LONGDOUBLE_OUT,
  "void",               // FFI_VOID,
  "wchar",              // FFI_WCHAR,
  "wcharOut",           // FFI_WCHAR_OUT,
  "wstring",            // FFI_WSTRING,
  "wstringOut",         // FFI_WSTRING_OUT
  "cObject",            // FFI_COBJECT,
  "smalltalk",          // FFI_SMALLTALK,
  "external",           // FFI_EXTERNALDATA,
  "externalOut",        // FFI_EXTERNALDATAOUT,
};

static int ffiNumStrs = sizeof(ffiStrs)/sizeof(ffiStrs[0]);
static object *ffiSyms;
static void* ffiLSTTypes[] = 
{
  &ffi_type_uchar,      // FFI_CHAR,
  &ffi_type_pointer,    // FFI_CHAR_OUT,
  &ffi_type_pointer,    // FFI_STRING,
  &ffi_type_pointer,    // FFI_STRING_OUT,
  &ffi_type_pointer,    // FFI_SYMBOL,
  &ffi_type_pointer,    // FFI_SYMBOL_OUT,
  &ffi_type_sint32,     // FFI_INT,
  &ffi_type_pointer,    // FFI_INT_OUT,
  &ffi_type_uint32,     // FFI_UINT,
  &ffi_type_pointer,    // FFI_UINT_OUT,
  &ffi_type_sint32,     // FFI_LONG,
  &ffi_type_pointer,    // FFI_LONG_OUT
  &ffi_type_uint32,     // FFI_ULONG,
  &ffi_type_pointer,    // FFI_ULONG_OUT,
  &ffi_type_float,      // FFI_FLOAT,
  &ffi_type_pointer,    // FFI_FLOAT_OUT,
  &ffi_type_double,     // FFI_DOUBLE,
  &ffi_type_pointer,    // FFI_DOUBLE_OUT,
  &ffi_type_longdouble, // FFI_LONGDOUBLE,
  &ffi_type_pointer,    // FFI_LONG_DOUBLE_OUT,
  &ffi_type_void,       // FFI_VOID,
  &ffi_type_uint16,     // FFI_WCHAR,
  &ffi_type_pointer,    // FFI_WCHAR_OUT,
  &ffi_type_pointer,    // FFI_WSTRING,
  &ffi_type_pointer,    // FFI_WSTRING_OUT
  &ffi_type_pointer,    // FFI_COBJECT,
  &ffi_type_pointer,    // FFI_SMALLTALK,
  &ffi_type_pointer,    // FFI_EXTERNALDATA,
  &ffi_type_pointer,    // FFI_EXTERNALDATAOUT,
};

static size_t ffiLSTTypeSizes[] = 
{
  sizeof(unsigned char),  // FFI_CHAR,
  sizeof(void*),          // FFI_CHAR_OUT,
  sizeof(void*),          // FFI_STRING,
  sizeof(void*),          // FFI_STRING_OUT,
  sizeof(void*),          // FFI_SYMBOL,
  sizeof(void*),          // FFI_SYMBOL_OUT,
  sizeof(int),            // FFI_INT,
  sizeof(void*),          // FFI_INT_OUT,
  sizeof(unsigned int),   // FFI_UINT,
  sizeof(void*),          // FFI_UINT_OUT,
  sizeof(long),           // FFI_LONG,
  sizeof(void*),          // FFI_LONG_OUT
  sizeof(unsigned int),   // FFI_ULONG,
  sizeof(void*),          // FFI_ULONG_OUT,
  sizeof(float),          // FFI_FLOAT,
  sizeof(void*),          // FFI_FLOAT_OUT,
  sizeof(double),         // FFI_DOUBLE,
  sizeof(void*),          // FFI_DOUBLE_OUT,
  sizeof(long double),    // FFI_LONGDOUBLE,
  sizeof(void*),          // FFI_LONG_DOUBLE_OUT,
  0,                      // FFI_VOID,
  sizeof(short),          // FFI_WCHAR,
  sizeof(void*),          // FFI_WCHAR_OUT,
  sizeof(void*),          // FFI_WSTRING,
  sizeof(void*),          // FFI_WSTRING_OUT
  sizeof(void*),          // FFI_COBJECT,
  sizeof(void*),          // FFI_SMALLTALK,
  sizeof(void*),          // FFI_EXTERNALDATA,
  sizeof(void*),          // FFI_EXTERNALDATAOUT,
};

void initFFISymbols()
{
  ffiSyms = (object*)(calloc(ffiNumStrs, sizeof(object)));
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


void valueOut(object value, FFI_DataType* data)
{
  object realValue;

  object sclass = globalSymbol("Symbol");
  object vclass = getClass(value);
  if(vclass == sclass)
    realValue = globalSymbol(charPtr(value));
  else
    realValue = value;

  switch(data->typemap)
  {
    case FFI_INT_OUT:
    case FFI_UINT_OUT:
    case FFI_LONG_OUT:
    case FFI_ULONG_OUT:
    case FFI_CHAR_OUT:
    case FFI_WCHAR_OUT:
      data->outInteger.pointer = &data->outInteger.integer;
      data->outInteger.integer = getInteger(realValue);
      data->ptr = &data->outInteger.pointer;
      data->type = &ffi_type_pointer;
      break;
    case FFI_FLOAT_OUT:
      data->outFloat.pointer = &data->outFloat._float;
      if(isInteger(realValue))
      {
        data->outFloat._float = (float)getInteger(realValue);
      }
      else
      {
        data->outFloat._float = floatValue(realValue);
      }
      data->ptr = &data->outFloat.pointer;
      data->type = &ffi_type_pointer;
      break;
    case FFI_DOUBLE_OUT:
    case FFI_LONGDOUBLE_OUT:
      data->outDouble.pointer = &data->outDouble._double;
      if(isInteger(realValue))
      {
        data->outDouble._double = (double)getInteger(realValue);
      }
      else
      {
        data->outDouble._double = floatValue(realValue);
      }
      data->ptr = &data->outDouble.pointer;
      data->type = &ffi_type_pointer;
      break;
    case FFI_CHAR:
      data->integer = getInteger(realValue);
      data->ptr = &data->integer;
      data->type = &ffi_type_uchar;
      break;
    case FFI_STRING:
    case FFI_STRING_OUT:
    case FFI_SYMBOL:
    case FFI_SYMBOL_OUT:
      data->charPtr = charPtr(realValue);
      data->ptr = &data->charPtr;
      data->type = &ffi_type_pointer;
      break;
    case FFI_UINT:
    case FFI_ULONG:
      if(getClass(realValue) == globalSymbol("Integer"))
        data->integer = getInteger(realValue);
      else if(getClass(realValue) == globalSymbol("Float"))
        data->integer = (int)floatValue(realValue);
      data->ptr = &data->integer;
      data->type = &ffi_type_uint32;
    case FFI_INT:
    case FFI_LONG:
      if(getClass(realValue) == globalSymbol("Integer"))
        data->integer = getInteger(realValue);
      else if(getClass(realValue) == globalSymbol("Float"))
        data->integer = (int)floatValue(realValue);
      data->ptr = &data->integer;
      data->type = &ffi_type_sint32;
      break;
    case FFI_FLOAT:
      // \todo: How to check type.
      if(isInteger(realValue))
      {
        data->_float = floatValue(realValue);
      }
      else
      {
        data->_float = floatValue(realValue);
      }
      data->ptr = &data->_float;
      data->type = &ffi_type_float;
      break;
    case FFI_DOUBLE:
      // \todo: How to check type.
      if(isInteger(realValue))
      {
        data->_double = (double)getInteger(realValue);
      }
      else
      {
        data->_double = floatValue(realValue);
      }
      data->ptr = &data->_double;
      data->type = &ffi_type_double;
      break;
    case FFI_LONGDOUBLE:
      // \todo: How to check type.
      if(isInteger(realValue))
      {
        data->_float = (float)getInteger(realValue);
      }
      else
      {
        data->_float = floatValue(realValue);
      }
      data->ptr = &data->_float;
      data->type = &ffi_type_longdouble;
      break;
    case FFI_VOID:
      data->type = &ffi_type_void;
      break;
    case FFI_WCHAR:
      data->type = &ffi_type_uint16;
      break;
    case FFI_WSTRING:
      data->type = &ffi_type_pointer;
      break;
    case FFI_WSTRING_OUT:
      data->type = &ffi_type_pointer;
      break;
    case FFI_COBJECT:
      {
        // \todo: How to check type.
        void* f = cPointerValue(realValue);
        int s = sizeof(f);
        data->cPointer = f;
        data->ptr = &data->cPointer;
        data->type = &ffi_type_pointer;
      }
      break;
    case FFI_SMALLTALK:
      data->type = &ffi_type_pointer;
      break;
    case FFI_EXTERNALDATA:
    case FFI_EXTERNALDATAOUT:
      {
        // Check the argument, see if it's a type of ExternalData  
        // Initially, check just for responding to 'fields'
        object args[1];
        args[0] = newSymbol("fields");
        SObjectHandle* lock_arg = new_SObjectHandle_from_object(args[0]);
        object result = sendMessageToObject(value, "respondsTo:", args, 1);
        free_SObjectHandle(lock_arg);
        if(result == booleanSyms[booleanTrue])
        {
          result = sendMessageToObject(value, "fields", NULL, 0);
          // \todo: type check return.
          // Now process the returned array
          int ctypes = result->size;
          // Create an ffi_type to represent this structure.
          ffi_type* ed_type = (ffi_type*)(calloc(1, sizeof(ffi_type)));
          // \todo: check leakage.
          ffi_type** ed_type_elements = (ffi_type**)(calloc(ctypes + 1, sizeof(ffi_type*)));
          FFI_DataType* members = (FFI_DataType*)(calloc(ctypes, sizeof(FFI_DataType)));
          size_t size = 0;
          int i;
          for(i = 0; i < ctypes; ++i)
          {
            // Read the types from the returned array.
            object type = basicAt(result, i+1);
            object name = basicAt(type,1);
            object basetype = basicAt(type,2);
            int argmap = mapType(basetype);
            ed_type_elements[i] = (ffi_type*)(ffiLSTTypes[argmap]);
            size += ffiLSTTypeSizes[argmap];
          }
          // leak:
          void* f = calloc(1, size);
          char* pf = (char*)(f);
          for(i = 0; i < ctypes; ++i)
          {
            // Read the types from the returned array.
            object type = basicAt(result,i+1);
            object name = basicAt(type,1);
            object basetype = basicAt(type,2);
            object args[1];
            args[0] = newInteger(i+1);
            object memberval = sendMessageToObject(value, "member:", args, 1); 
            int argmap = mapType(basetype);
            members[i].typemap = argmap;
            valueOut(memberval, &members[i]);
            ed_type_elements[i] = members[i].type;
            memcpy(pf, members[i].ptr, ffiLSTTypeSizes[argmap]);
            pf += ffiLSTTypeSizes[argmap];
          }
          ed_type_elements[ctypes] = NULL;
          ed_type->size = ed_type->alignment = 0;
          ed_type->elements = ed_type_elements;
          ed_type->type = FFI_TYPE_STRUCT;

          data->externalData.type = ed_type;
          data->externalData.size = size;
          data->externalData.members = members;
          data->externalData._class = getClass(realValue);
          data->externalData.numMembers = ctypes;
          data->externalData.pointer = f;

          if(data->typemap == FFI_EXTERNALDATA)
          {
            data->ptr = f;
            data->type = ed_type;
          }
          else
          {
            data->ptr = &data->externalData.pointer;
            data->type = &ffi_type_pointer;
          }
        }
        else
        {
          sysError("External data argument","not ExternalData");
        }
      }
      break;
  }
}

size_t readFromStorage(void* storage, FFI_DataType* data)
{
  switch(data->typemap)
  {
    case FFI_INT_OUT:
    case FFI_UINT_OUT:
    case FFI_LONG_OUT:
    case FFI_ULONG_OUT:
    case FFI_CHAR_OUT:
    case FFI_WCHAR_OUT:
      memcpy(&data->outInteger.integer, storage, ffiLSTTypeSizes[data->typemap]);
      return ffiLSTTypeSizes[data->typemap];
      break;

    case FFI_CHAR:
      memcpy(&data->integer, storage, ffiLSTTypeSizes[data->typemap]);
      return ffiLSTTypeSizes[data->typemap];
      break;

    case FFI_STRING:
      memcpy(&data->charPtr, storage, ffiLSTTypeSizes[data->typemap]);
      return ffiLSTTypeSizes[data->typemap];
      break;

    case FFI_INT:
    case FFI_UINT:
    case FFI_LONG:
    case FFI_ULONG:
      memcpy(&data->integer, storage, ffiLSTTypeSizes[data->typemap]);
      return ffiLSTTypeSizes[data->typemap];
      break;

    case FFI_FLOAT:
      memcpy(&data->_float, storage, ffiLSTTypeSizes[data->typemap]);
      return ffiLSTTypeSizes[data->typemap];
      break;

    case FFI_DOUBLE:
      memcpy(&data->_double, storage, ffiLSTTypeSizes[data->typemap]);
      return ffiLSTTypeSizes[data->typemap];
      break;

    case FFI_COBJECT:
      memcpy(&data->cPointer, storage, ffiLSTTypeSizes[data->typemap]);
      return ffiLSTTypeSizes[data->typemap];
      break;

    case FFI_VOID:
      return 0;
      break;

    case FFI_EXTERNALDATA:
      // \todo:
      return 0;
      break;

    case FFI_EXTERNALDATAOUT:
      // \todo:
      return 0;
      break;
  }
  return 0;
}

object valueIn(int retMap, FFI_DataType* data)
{
  switch(retMap)
  {
    case FFI_INT_OUT:
    case FFI_UINT_OUT:
    case FFI_LONG_OUT:
    case FFI_ULONG_OUT:
    case FFI_CHAR_OUT:
    case FFI_WCHAR_OUT:
      return newInteger(data->outInteger.integer);
      break;

    case FFI_CHAR:
      return newChar(data->integer);
      break;

    case FFI_STRING:
      return newStString(data->charPtr);
      break;

    case FFI_INT:
    case FFI_UINT:
    case FFI_LONG:
    case FFI_ULONG:
      return newInteger(data->integer);
      break;

    case FFI_FLOAT:
      return newFloat(data->_float);
      break;

    case FFI_DOUBLE:
      return newFloat(data->_double);
      break;

    case FFI_COBJECT:
      return newCPointer(data->cPointer);
      break;

    case FFI_VOID:
      return nilobj;
      break;

    case FFI_EXTERNALDATA:
      {
        object edclass = data->externalData._class;
        int csize = getInteger(basicAt(edclass,sizeInClass));
        object ed = allocObject(csize);
        SObjectHandle* lock_ed = new_SObjectHandle_from_object(ed);
        basicAtPut(ed,1, newArray(data->externalData.numMembers));
        ed->_class = edclass;
        // Now fill in the values using recursive marshalling.
        object args[2];
        SObjectHandle* lock_args[2];
        int i;
        for(i = 0; i < data->externalData.numMembers; ++i)
        {
          args[0] = newInteger(i+1);
          lock_args[0] = new_SObjectHandle_from_object(args[0]);
          args[1] = valueIn(data->externalData.members[i].typemap, &(data->externalData.members[i]));
          lock_args[1] = new_SObjectHandle_from_object(args[1]);
          sendMessageToObject(ed, "setMember:to:", args, 2);
          free_SObjectHandle(lock_args[0]);
          free_SObjectHandle(lock_args[1]);
        }
        free_SObjectHandle(lock_ed);
        return ed;
      }
      break;

    case FFI_EXTERNALDATAOUT:
      {
        object edclass = data->externalData._class;
        int csize = getInteger(basicAt(edclass,sizeInClass));
        object ed = allocObject(csize);
        SObjectHandle* lock_ed = new_SObjectHandle_from_object(ed);
        basicAtPut(ed,1, newArray(data->externalData.numMembers));
        ed->_class = edclass;
        // Now fill in the values using recursive marshalling.
        object args[2];
        SObjectHandle* lock_args[2];
        char* structStorage = (char*)(data->externalData.pointer);
        int i;
        for(i = 0; i < data->externalData.numMembers; ++i)
        {
          args[0] = newInteger(i+1);
          lock_args[0] = new_SObjectHandle_from_object(args[0]);
          structStorage += readFromStorage(structStorage, &(data->externalData.members[i]));
          args[1] = valueIn(data->externalData.members[i].typemap, &(data->externalData.members[i]));
          lock_args[1] = new_SObjectHandle_from_object(args[1]);
          sendMessageToObject(ed, "setMember:to:", args, 2);
          free_SObjectHandle(lock_args[0]);
          free_SObjectHandle(lock_args[1]);
        }
        free_SObjectHandle(lock_ed);
        return ed;
      }
      break;
  }
  return nilobj;
}

typedef struct FFI_CallbackData_U
{
  SObjectHandle*  block;
  int     numArgs;
  FFI_Symbols *argTypeArray;
  FFI_Symbols retType;
} FFI_CallbackData;

FFI_CallbackData* newCallbackData(int numArgs)
{
  FFI_CallbackData* data = (FFI_CallbackData*)(calloc(1, sizeof(FFI_CallbackData)));
  data->block = new_SObjectHandle();
  data->argTypeArray = (FFI_Symbols*)(calloc(numArgs, sizeof(FFI_Symbols)));
  data->numArgs = numArgs;

  return data;
}

void deleteCallbackData(FFI_CallbackData *data)
{
  free_SObjectHandle(data->block);
  free(data->argTypeArray);
  free(data);
}

void callBack(ffi_cif* cif, void* ret, void* args[], void* ud)
{
  void* handle = *(void**)args[0];
  int arg;

  FFI_CallbackData* data = (FFI_CallbackData*)ud;

  object block = data->block->m_handle;
  object context = basicAt(block,contextInBlock);
  object bytePointer = basicAt(block,bytecountPositionInBlock);

  object processClass = globalSymbol("Process");
  object process = allocObject(processSize);
  SObjectHandle* lock_process = new_SObjectHandle_from_object(process);
  object stack = newArray(50);
  SObjectHandle* lock_stack = new_SObjectHandle_from_object(stack);
  basicAtPut(process,stackInProcess, stack);
  basicAtPut(process,stackTopInProcess, newInteger(10));
  basicAtPut(process,linkPtrInProcess, newInteger(2));
  basicAtPut(stack,contextInStack, context);
  basicAtPut(stack,returnpointInStack, newInteger(1));
  basicAtPut(stack,bytepointerInStack, bytePointer);

  /* change context and byte pointer */
  int argLoc = getInteger(basicAt(block,argumentLocationInBlock));
  object temps = basicAt(context,temporariesInContext);
  for(arg = 0; arg < data->numArgs; ++arg)
  {
    FFI_DataType value;
    value.typemap = data->argTypeArray[arg];
    readFromStorage(args[arg], &value);
    basicAtPut(temps,argLoc+arg, valueIn(data->argTypeArray[arg], &value));
    // \todo: Memory leak
  }

  object saveProcessStack = processStack;
  SObjectHandle* lock_saveProcessStack = new_SObjectHandle_from_object(saveProcessStack);
  int saveLinkPointer = linkPointer;
  while(execute(process, 15000));
  // Re-read the stack object, in case it had to grow during execution and 
  // was replaced.
  stack = basicAt(process,stackInProcess);
  object ro = basicAt(stack,1);
  FFI_DataType temp;
  temp.typemap = data->retType;
  valueOut(ro, &temp); 
  // \todo: Support struct returns
  memcpy(ret, temp.ptr, ffiLSTTypeSizes[data->retType]);
  processStack = saveProcessStack;
  linkPointer = saveLinkPointer;

  free_SObjectHandle(lock_saveProcessStack);
  free_SObjectHandle(lock_process);
  free_SObjectHandle(lock_stack);
}

object ffiPrimitive(int number, object* arguments)
{   
  object returnedObject = nilobj;
  char libName[100];
  char err[100];

  switch(number - 180) {
    case 0: /* dlopen */
#if !defined WIN32
          {
        char* p = charPtr(arguments[0]);
        snprintf(libName, 100, "%s.%s", p, SO_EXT);
        FFI_LibraryHandle handle = dlopen(libName, RTLD_LAZY);
        if(NULL != handle)
          returnedObject = newCPointer(handle);
        else 
        {
          const char* msg = dlerror();
          sysWarn("library not found",msg);
          returnedObject = nilobj;
        }
      }
#else
          {
        char* p = charPtr(arguments[0]);
        snprintf(libName, 100, "%s.%s", p, SO_EXT);
        FFI_LibraryHandle handle = LoadLibrary(libName);
        if(NULL != handle)
          returnedObject = newCPointer(handle);
        else 
        {
          // \todo: GetLastError() etc.
          sysWarn("library not found",libName);
          returnedObject = nilobj;
        }
      }
#endif
      break;

    case 1: /* dlsym */
#if !defined WIN32
          {
        // \todo: Check type.
        FFI_LibraryHandle lib = cPointerValue(arguments[0]);
        char* p = charPtr(arguments[1]);
        if(NULL != lib)
        {
          FFI_FunctionHandle func = dlsym(lib, p);
          if(NULL != func)
            returnedObject = newCPointer(func);
          else
          {
            snprintf(err, 100, "function %s not found in library", p);
            sysWarn(err, "ffiPrimitive");
            returnedObject = nilobj;
          }
        }
      }
#else
      {
        // \todo: Check type.
        FFI_LibraryHandle lib = cPointerValue(arguments[0]);
        char* p = charPtr(arguments[1]);
        if(NULL != lib)
        {
          FFI_FunctionHandle func = GetProcAddress((HINSTANCE)lib, p);
          if(NULL != func)
            returnedObject = newCPointer(func);
          else
          {
            snprintf(err, 100, "function %s not found in library", p);
            sysWarn(err, "ffiPrimitive");
            returnedObject = nilobj;
          }
        }
      }
#endif
      break;

     /* 
      *  cCall <func_id> <#return_type> <#(#arg_type ...)> <#(args ...)>
      */
    case 2: /* cCall */
      {
        // \todo: Check types.
        FFI_FunctionHandle func = cPointerValue(arguments[0]);
        object rtype = arguments[1];
        int retMap = mapType(rtype);
        int cargTypes = arguments[2]->size;
        int cargs = arguments[3]->size;
        int cOutArgs = 0;

        assert(cargTypes <= cargs);

        if(NULL != func)
        {
          ffi_type** args = NULL;
          void** values = NULL;
          FFI_DataType* dataValues = NULL; 
          if(cargs > 0)
          {
            // leak:
            args = (ffi_type**)(calloc(cargTypes, sizeof(ffi_type*)));
            // leak:
            values = (void**)(calloc(cargs, sizeof(void*)));
            // leak:
            dataValues = (FFI_DataType*)(calloc(cargs, sizeof(FFI_DataType)));

            int i;
            for(i = 0; i < cargTypes; ++i)
            {
              object argType = basicAt(arguments[2], i+1);
              dataValues[i].typemap = mapType(argType);
              valueOut(basicAt(arguments[3], i+1), &dataValues[i]);
              values[i] = dataValues[i].ptr;
              args[i] = dataValues[i].type;
            }
          }

          ffi_type* ret;
          ret = (ffi_type*)(ffiLSTTypes[retMap]); 
          ffi_type retVal;
          void* retData = &retVal;

          ffi_cif cif;
          if(ffi_prep_cif(&cif, FFI_DEFAULT_ABI, cargTypes, ret, args) == FFI_OK)
          {
            returnedObject = newArray(cargTypes + 1);

            ffi_call(&cif, (void(*)())(func), retData, values);
            FFI_DataType ret;
            ret.typemap = retMap;
            readFromStorage(retData, &ret);
            basicAtPut(returnedObject,1, valueIn(retMap, &ret));
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
            for(i = 0; i < cargTypes; ++i)
            {
              object argType = basicAt(arguments[2], i+1);
              int argMap = mapType(argType);
              // Get a new value out of the arguments array.
              object newVal = valueIn(argMap, &dataValues[i]);
              basicAtPut(returnedObject,i+2, newVal); 
            }
          }
        }
      }
      break;

     /* 
      *  cCallback <#return_type> <#(#arg_type ...)> aBlock
      */
    case 3: /* cCallback */
      {
        void *callback;

        object rtype = arguments[0];
        int retMap = mapType(rtype);
        int cargTypes = arguments[1]->size;
        ffi_closure* closure;
        ffi_cif *cif = (ffi_cif*)(calloc(1, sizeof(ffi_cif)));
        object block = arguments[2];

        /* Allocate closure and bound_puts */
        closure = (ffi_closure*)(ffi_closure_alloc(sizeof(ffi_closure), &callback));

        if(closure)
        {
          /* Build the callback structure that is passed to the callback handler */
          FFI_CallbackData* data = newCallbackData(cargTypes);

          /* Initialize the argument info vectors */
          ffi_type** args = NULL;
          if(cargTypes > 0)
          {
            args = (ffi_type**)(calloc(cargTypes, sizeof(ffi_type)));

            int i;
            for(i = 0; i < cargTypes; ++i)
            {
              object argType = basicAt(arguments[1], i+1);
              int argMap = mapType(argType);
              data->argTypeArray[i] = argMap;
              args[i] = (ffi_type*)(ffiLSTTypes[argMap]);
            }
          }
          data->retType = (FFI_Symbols)(retMap);
          data->block = new_SObjectHandle_from_object(block);

          ffi_type* ret;
          ret = (ffi_type*)(ffiLSTTypes[retMap]); 

          /* Initialize the cif */
          if(ffi_prep_cif(cif, FFI_DEFAULT_ABI, cargTypes, ret, args) == FFI_OK)
          {
            /* Initialize the closure, setting stream to stdout */
            if(ffi_prep_closure_loc(closure, cif, callBack, data, callback) == FFI_OK)
            {
              returnedObject = newCPointer(callback);
            }
          }
        }

        /* Deallocate both closure, and bound_puts */
        //ffi_closure_free(closure);
      }
      break;

    case 4: // dlclose
#if !defined WIN32
      {
        // \todo: Check type.
        FFI_LibraryHandle lib = cPointerValue(arguments[0]);
        char* p = charPtr(arguments[1]);
        if(NULL != lib)
        {
          int result = dlclose(lib);
          returnedObject = newInteger(result);
        }
      }
#else
      {
        // \todo: Check type.
        FFI_LibraryHandle lib = cPointerValue(arguments[0]);
        char* p = charPtr(arguments[1]);
        if(NULL != lib)
        {
          int result = FreeLibrary((HINSTANCE)lib);
          returnedObject = newInteger(result);
        }
      }
#endif
      break;


    default:
      sysError("unknown primitive","ffiPrimitive");
  }
  return(returnedObject);
}

