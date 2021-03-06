/*
   Little Smalltalk, version 3
   Written by Tim Budd, January 1989

   tty interface routines
   this is used by those systems that have a bare tty interface
   systems using another interface, such as the stdwin interface
   will replace this file with another.
   */

#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#if !defined WIN32
extern "C" {
#include <dlfcn.h>
#include <memory.h>
}
#else
#include <Windows.h>
#endif
#include <assert.h>
#include <ffi.h>
#include <limits.h>

#include "env.h"
#include "objmemory.h"
#include "names.h"
#include "interp.h"

extern ObjectHandle sendMessageToObject(ObjectHandle receiver, const char* message, ObjectHandle* args, int cargs);

extern ObjectHandle processStack;
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
  ffiSyms = static_cast<object*>(calloc(ffiNumStrs, sizeof(object)));
  int i;
  for(i = 0; i < ffiNumStrs; ++i)
    ffiSyms[i] = createSymbol(ffiStrs[i]);
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
    realValue = globalSymbol(objectRef(value).charPtr());
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
      data->outFloat._float = objectRef(realValue).floatValue();
      data->ptr = &data->outFloat.pointer;
      data->type = &ffi_type_pointer;
      break;
    case FFI_DOUBLE_OUT:
    case FFI_LONGDOUBLE_OUT:
      data->outDouble.pointer = &data->outDouble._double;
      data->outDouble._double = objectRef(realValue).floatValue();
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
      data->charPtr = objectRef(realValue).charPtr();
      data->ptr = &data->charPtr;
      data->type = &ffi_type_pointer;
      break;
    case FFI_UINT:
    case FFI_ULONG:
      if(getClass(realValue) == globalSymbol("Integer"))
        data->integer = getInteger(realValue);
      else if(getClass(realValue) == globalSymbol("Float"))
        data->integer = (int)objectRef(realValue).floatValue();
      data->ptr = &data->integer;
      data->type = &ffi_type_uint32;
    case FFI_INT:
    case FFI_LONG:
      if(getClass(realValue) == globalSymbol("Integer"))
        data->integer = getInteger(realValue);
      else if(getClass(realValue) == globalSymbol("Float"))
        data->integer = (int)objectRef(realValue).floatValue();
      data->ptr = &data->integer;
      data->type = &ffi_type_sint32;
      break;
    case FFI_FLOAT:
      // \todo: How to check type.
      data->_float = objectRef(realValue).floatValue();
      data->ptr = &data->_float;
      data->type = &ffi_type_float;
      break;
    case FFI_DOUBLE:
      // \todo: How to check type.
      data->_double = objectRef(realValue).floatValue();
      data->ptr = &data->_double;
      data->type = &ffi_type_double;
      break;
    case FFI_LONGDOUBLE:
      // \todo: How to check type.
      data->_float = objectRef(realValue).floatValue();
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
        void* f = objectRef(realValue).cPointerValue();
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
        ObjectHandle args[1];
        args[0] = createSymbol("fields");
        ObjectHandle result = sendMessageToObject(value, "respondsTo:", args, 1);
        if(result == booleanSyms[booleanTrue])
        {
          result = sendMessageToObject(value, "fields", NULL, 0);
          // \todo: type check return.
          // Now process the returned array
          int ctypes = objectRef(result).size;
          // Create an ffi_type to represent this structure.
          ffi_type* ed_type = new ffi_type;
          // \todo: check leakage.
          ffi_type** ed_type_elements = new ffi_type*[ctypes + 1];
          FFI_DataType* members = new FFI_DataType[ctypes];
          size_t size = 0;
          for(int i = 0; i < ctypes; ++i)
          {
            // Read the types from the returned array.
            ObjectHandle type = result->basicAt(i+1);
            ObjectHandle name = type->basicAt(1);
            ObjectHandle basetype = type->basicAt(2);
            int argmap = mapType(basetype);
            ed_type_elements[i] = static_cast<ffi_type*>(ffiLSTTypes[argmap]);
            size += ffiLSTTypeSizes[argmap];
          }
          // leak:
          void* f = calloc(1, size);
          char* pf = static_cast<char*>(f);
          for(int i = 0; i < ctypes; ++i)
          {
            // Read the types from the returned array.
            ObjectHandle type = result->basicAt(i+1);
            ObjectHandle name = type->basicAt(1);
            ObjectHandle basetype = type->basicAt(2);
            ObjectHandle args[1];
            args[0] = newInteger(i+1);
            ObjectHandle memberval = sendMessageToObject(value, "member:", args, 1); 
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
        ObjectHandle edclass = data->externalData._class;
        int csize = edclass->basicAt(sizeInClass);
        ObjectHandle ed = MemoryManager::Instance()->allocObject(csize);
        ed->basicAtPut(1, newArray(data->externalData.numMembers));
        ed->_class = edclass;
        // Now fill in the values using recursive marshalling.
        ObjectHandle args[2];
        for(int i = 0; i < data->externalData.numMembers; ++i)
        {
          args[0] = newInteger(i+1);
          args[1] = valueIn(data->externalData.members[i].typemap, &(data->externalData.members[i]));
          sendMessageToObject(ed, "setMember:to:", args, 2);
        }
        return ed;
      }
      break;

    case FFI_EXTERNALDATAOUT:
      {
        ObjectHandle edclass = data->externalData._class;
        int csize = edclass->basicAt(sizeInClass);
        ObjectHandle ed = MemoryManager::Instance()->allocObject(csize);
        ed->basicAtPut(1, newArray(data->externalData.numMembers));
        ed->_class = edclass;
        // Now fill in the values using recursive marshalling.
        ObjectHandle args[2];
        char* structStorage = static_cast<char*>(data->externalData.pointer);
        for(int i = 0; i < data->externalData.numMembers; ++i)
        {
          args[0] = newInteger(i+1);
          structStorage += readFromStorage(structStorage, &(data->externalData.members[i]));
          args[1] = valueIn(data->externalData.members[i].typemap, &(data->externalData.members[i]));
          sendMessageToObject(ed, "setMember:to:", args, 2);
        }
        return ed;
      }
      break;
  }
}

typedef struct FFI_CallbackData_U
{
  ObjectHandle  block;
  int     numArgs;
  FFI_Symbols *argTypeArray;
  FFI_Symbols retType;
} FFI_CallbackData;

FFI_CallbackData* newCallbackData(int numArgs)
{
  FFI_CallbackData* data = new FFI_CallbackData();
  data->argTypeArray = static_cast<FFI_Symbols*>(calloc(numArgs, sizeof(FFI_Symbols)));
  data->numArgs = numArgs;

  return data;
}

void deleteCallbackData(FFI_CallbackData *data)
{
  free(data->argTypeArray);
  free(data);
}

void callBack(ffi_cif* cif, void* ret, void* args[], void* ud)
{
  void* handle = *(void**)args[0];
  int arg;

  FFI_CallbackData* data = (FFI_CallbackData*)ud;

  ObjectHandle block = data->block;
  ObjectHandle context = block->basicAt(contextInBlock);
  ObjectHandle bytePointer = block->basicAt(bytecountPositionInBlock);

  object processClass = globalSymbol("Process");
  ObjectHandle process = MemoryManager::Instance()->allocObject(processSize);
  ObjectHandle stack = newArray(50);
  process->basicAtPut(stackInProcess, stack);
  process->basicAtPut(stackTopInProcess, newInteger(10));
  process->basicAtPut(linkPtrInProcess, newInteger(2));
  stack->basicAtPut(contextInStack, context);
  stack->basicAtPut(returnpointInStack, newInteger(1));
  stack->basicAtPut(bytepointerInStack, bytePointer);

  /* change context and byte pointer */
  int argLoc = getInteger(block->basicAt(argumentLocationInBlock));
  object temps = context->basicAt(temporariesInContext);
  for(arg = 0; arg < data->numArgs; ++arg)
  {
    FFI_DataType value;
    value.typemap = data->argTypeArray[arg];
    readFromStorage(args[arg], &value);
    objectRef(temps).basicAtPut(argLoc+arg, valueIn(data->argTypeArray[arg], &value));
    // \todo: Memory leak
  }

  ObjectHandle saveProcessStack = processStack;
  int saveLinkPointer = linkPointer;
  while(execute(process, 15000));
  // Re-read the stack object, in case it had to grow during execution and 
  // was replaced.
  stack = process->basicAt(stackInProcess);
  object ro = stack->basicAt(1);
  FFI_DataType temp;
  temp.typemap = data->retType;
  valueOut(ro, &temp); 
  // \todo: Support struct returns
  memcpy(ret, temp.ptr, ffiLSTTypeSizes[data->retType]);
  processStack = saveProcessStack;
  linkPointer = saveLinkPointer;
}

object ffiPrimitive(int number, object* arguments)
{   
  ObjectHandle returnedObject = nilobj;
  std::stringstream libName;

  switch(number - 180) {
    case 0: /* dlopen */
#if !defined WIN32
          {
        char* p = objectRef(arguments[0]).charPtr();
        libName << p << "." << SO_EXT;
        FFI_LibraryHandle handle = dlopen(libName.str().c_str(), RTLD_LAZY);
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
        char* p = objectRef(arguments[0]).charPtr();
        libName << p << "." << SO_EXT;
        FFI_LibraryHandle handle = LoadLibrary(libName.str().c_str());
        if(NULL != handle)
          returnedObject = newCPointer(handle);
        else 
        {
          // \todo: GetLastError() etc.
          sysWarn("library not found",libName.str().c_str());
          returnedObject = nilobj;
        }
      }
#endif
      break;

    case 1: /* dlsym */
#if !defined WIN32
          {
        // \todo: Check type.
        FFI_LibraryHandle lib = NULL;
        if(arguments[0] != nilobj) {
          lib = objectRef(arguments[0]).cPointerValue();
        }
        char* p = objectRef(arguments[1]).charPtr();
        FFI_FunctionHandle func;
        if(NULL != lib) {
          func = dlsym(lib, p);
        } else {
          func = dlsym(RTLD_SELF, p);
        }
        if(NULL != func)
          returnedObject = newCPointer(func);
        else
        {
          std::stringstream err;
          err << "function " << p << " not found in library";
          sysWarn(err.str().c_str(), "ffiPrimitive");
          returnedObject = nilobj;
        }
      }
#else
      {
        // \todo: Check type.
        FFI_LibraryHandle lib = objectRef(arguments[0]).cPointerValue();
        char* p = objectRef(arguments[1]).charPtr();
        if(NULL != lib)
        {
          FFI_FunctionHandle func = GetProcAddress((HINSTANCE)lib, p);
          if(NULL != func)
            returnedObject = newCPointer(func);
          else
          {
            std::stringstream err;
            err << "function " << p << " not found in library";
            sysWarn(err.str().c_str(), "ffiPrimitive");
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
        FFI_FunctionHandle func = objectRef(arguments[0]).cPointerValue();
        object rtype = arguments[1];
        int retMap = mapType(rtype);
        int cargTypes = objectRef(arguments[2]).size;
        int cargs = objectRef(arguments[3]).size;
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
            args = static_cast<ffi_type**>(calloc(cargTypes, sizeof(ffi_type*)));
            // leak:
            values = static_cast<void**>(calloc(cargs, sizeof(void*)));
            // leak:
            dataValues = static_cast<FFI_DataType*>(calloc(cargs, sizeof(FFI_DataType)));

            int i;
            for(i = 0; i < cargTypes; ++i)
            {
              object argType = objectRef(arguments[2]).basicAt(i+1);
              dataValues[i].typemap = mapType(argType);
              valueOut(objectRef(arguments[3]).basicAt(i+1), &dataValues[i]);
              values[i] = dataValues[i].ptr;
              args[i] = dataValues[i].type;
            }
          }
          ffi_type* ret;
          ret = static_cast<ffi_type*>(ffiLSTTypes[retMap]); 
          ffi_type retVal;
          void* retData = &retVal;

          ffi_cif cif;
          if(ffi_prep_cif(&cif, FFI_DEFAULT_ABI, cargTypes, ret, args) == FFI_OK)
          {
            returnedObject = newArray(cargTypes + 1);

            ffi_call(&cif, reinterpret_cast<void(*)()>(func), retData, values);
            FFI_DataType ret;
            ret.typemap = retMap;
            readFromStorage(retData, &ret);
            returnedObject->basicAtPut(1, valueIn(retMap, &ret));
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
              object argType = objectRef(arguments[2]).basicAt(i+1);
              int argMap = mapType(argType);
              // Get a new value out of the arguments array.
              object newVal = valueIn(argMap, &dataValues[i]);
              returnedObject->basicAtPut(i+2, newVal); 
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
        int cargTypes = objectRef(arguments[1]).size;
        ffi_closure* closure;
        ffi_cif *cif = static_cast<ffi_cif*>(calloc(1, sizeof(ffi_cif)));
        object block = arguments[2];

        /* Allocate closure and bound_puts */
        closure = static_cast<ffi_closure*>(ffi_closure_alloc(sizeof(ffi_closure), &callback));

        if(closure)
        {
          /* Build the callback structure that is passed to the callback handler */
          FFI_CallbackData* data = newCallbackData(cargTypes);

          /* Initialize the argument info vectors */
          ffi_type** args = NULL;
          if(cargTypes > 0)
          {
            args = static_cast<ffi_type**>(calloc(cargTypes, sizeof(ffi_type)));

            int i;
            for(i = 0; i < cargTypes; ++i)
            {
              object argType = objectRef(arguments[1]).basicAt(i+1);
              int argMap = mapType(argType);
              data->argTypeArray[i] = static_cast<FFI_Symbols_E>(argMap);
              args[i] = static_cast<ffi_type*>(ffiLSTTypes[argMap]);
            }
          }
          data->retType = static_cast<FFI_Symbols>(retMap);
          data->block = block;

          ffi_type* ret;
          ret = static_cast<ffi_type*>(ffiLSTTypes[retMap]); 

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
        FFI_LibraryHandle lib = objectRef(arguments[0]).cPointerValue();
        char* p = objectRef(arguments[1]).charPtr();
        if(NULL != lib)
        {
          int result = dlclose(lib);
          returnedObject = newInteger(result);
        }
      }
#else
      {
        // \todo: Check type.
        FFI_LibraryHandle lib = objectRef(arguments[0]).cPointerValue();
        char* p = objectRef(arguments[1]).charPtr();
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

