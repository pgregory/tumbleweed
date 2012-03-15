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
extern "C" {
#include <dlfcn.h>
}
#include <assert.h>
#include <ffi.h>
#include <limits.h>

#include "env.h"
#include "memory.h"
#include "names.h"
#include "interp.h"

extern ObjectHandle processStack;
extern int linkPointer;

typedef void* FFI_LibraryHandle;
typedef void* FFI_FunctionHandle;

typedef union FFI_DataType_U
{
  char* charPtr;
  int   integer;
  void* cPointer;
  float _float;
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
  "smalltalk"           // FFI_SMALLTALK,
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
  &ffi_type_pointer     // FFI_SMALLTALK,
};

void initFFISymbols()
{
  ffiSyms = static_cast<object*>(calloc(ffiNumStrs, sizeof(object)));
  int i;
  for(i = 0; i < ffiNumStrs; ++i)
    ffiSyms[i] = MemoryManager::Instance()->newSymbol(ffiStrs[i]);
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
  object vclass = objectRef(value)._class;
  if(vclass == sclass)
    realValue = globalSymbol(objectRef(value).charPtr());
  else
    realValue = value;
  switch(argMap)
  {
    case FFI_INT_OUT:
    case FFI_UINT_OUT:
    case FFI_LONG_OUT:
    case FFI_ULONG_OUT:
    case FFI_CHAR_OUT:
    case FFI_WCHAR_OUT:
      data->outInteger.pointer = &data->outInteger.integer;
      data->outInteger.integer = objectRef(realValue).intValue();
      ptr = &data->outInteger.pointer;
      break;
    case FFI_DOUBLE_OUT:
    case FFI_LONGDOUBLE_OUT:
      data->outFloat.pointer = &data->outFloat._float;
      data->outFloat._float = objectRef(realValue).floatValue();
      ptr = &data->outFloat.pointer;
      break;
    case FFI_CHAR:
      data->integer = objectRef(realValue).intValue();
      ptr = &data->integer;
      break;
    case FFI_STRING:
    case FFI_STRING_OUT:
    case FFI_SYMBOL:
    case FFI_SYMBOL_OUT:
      data->charPtr = objectRef(realValue).charPtr();
      ptr = &data->charPtr;
      break;
    case FFI_INT:
    case FFI_UINT:
    case FFI_LONG:
    case FFI_ULONG:
      if(objectRef(realValue)._class == globalSymbol("Integer"))
        data->integer = objectRef(realValue).intValue();
      else if(objectRef(realValue)._class == globalSymbol("Float"))
        data->integer = (int)objectRef(realValue).floatValue();
      ptr = &data->integer;
      break;
    case FFI_DOUBLE:
    case FFI_LONGDOUBLE:
      // \todo: How to check type.
      data->_float = objectRef(realValue).floatValue();
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
      {
        // \todo: How to check type.
        void* f = objectRef(realValue).cPointerValue();
        int s = sizeof(f);
        data->cPointer = f;
        ptr = &data->cPointer;
      }
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
    case FFI_INT_OUT:
    case FFI_UINT_OUT:
    case FFI_LONG_OUT:
    case FFI_ULONG_OUT:
    case FFI_CHAR_OUT:
    case FFI_WCHAR_OUT:
      return MemoryManager::Instance()->newInteger(data->outInteger.integer);
      break;

    case FFI_CHAR:
      return MemoryManager::Instance()->newChar(data->integer);
      break;

    case FFI_STRING:
      return MemoryManager::Instance()->newStString(data->charPtr);
      break;

    case FFI_INT:
    case FFI_UINT:
    case FFI_LONG:
    case FFI_ULONG:
      return MemoryManager::Instance()->newInteger(data->integer);
      break;

    case FFI_COBJECT:
      return MemoryManager::Instance()->newCPointer(data->cPointer);
      break;

    case FFI_VOID:
      return nilobj;
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
  FFI_CallbackData* data = static_cast<FFI_CallbackData*>(calloc(1, sizeof(FFI_CallbackData)));
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
  ObjectHandle stack = MemoryManager::Instance()->newArray(50);
  process->basicAtPut(stackInProcess, stack);
  process->basicAtPut(stackTopInProcess, MemoryManager::Instance()->newInteger(10));
  process->basicAtPut(linkPtrInProcess, MemoryManager::Instance()->newInteger(2));
  stack->basicAtPut(contextInStack, context);
  stack->basicAtPut(returnpointInStack, MemoryManager::Instance()->newInteger(1));
  stack->basicAtPut(bytepointerInStack, bytePointer);

  /* change context and byte pointer */
  int argLoc = objectRef(block->basicAt(argumentLocationInBlock)).intValue();
  object temps = context->basicAt(temporariesInContext);
  for(arg = 0; arg < data->numArgs; ++arg)
    objectRef(temps).basicAtPut(argLoc+arg, valueIn(data->argTypeArray[arg], (FFI_DataType*)args[arg]));

  ObjectHandle saveProcessStack = processStack;
  int saveLinkPointer = linkPointer;
  while(execute(process, 15000));
  // Re-read the stack object, in case it had to grow during execution and 
  // was replaced.
  stack = process->basicAt(stackInProcess);
  object ro = stack->basicAt(1);
  valueOut(data->retType, ro, static_cast<FFI_DataType*>(ret)); 
  processStack = saveProcessStack;
  linkPointer = saveLinkPointer;
}

object ffiPrimitive(int number, object* arguments)
{   
  ObjectHandle returnedObject = nilobj;
  char libName[MAX_PATH];

  switch(number - 180) {
    case 0: /* dlopen */
      {
        char* p = objectRef(arguments[0]).charPtr();
        sprintf(libName, "%s.%s", p, SO_EXT);
        FFI_LibraryHandle handle = dlopen(libName, RTLD_LAZY);
        if(NULL != handle)
          returnedObject = MemoryManager::Instance()->newCPointer(handle);
        else 
        {
          const char* msg = dlerror();
          sysWarn("library not found",msg);
          returnedObject = nilobj;
        }
      }
      break;

    case 1: /* dlsym */
      {
        // \todo: Check type.
        FFI_LibraryHandle lib = objectRef(arguments[0]).cPointerValue();
        char* p = objectRef(arguments[1]).charPtr();
        if(NULL != lib)
        {
          FFI_FunctionHandle func = dlsym(lib, p);
          if(NULL != func)
            returnedObject = MemoryManager::Instance()->newCPointer(func);
          else
          {
            sysWarn("function not found in library", "ffiPrimitive");
            returnedObject = nilobj;
          }
        }
      }
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
            args = static_cast<ffi_type**>(calloc(cargTypes, sizeof(ffi_type)));
            values = static_cast<void**>(calloc(cargs, sizeof(void*)));
            dataValues = static_cast<FFI_DataType*>(calloc(cargs, sizeof(FFI_DataType)));

            int i;
            for(i = 0; i < cargTypes; ++i)
            {
              object argType = objectRef(arguments[2]).basicAt(i+1);
              int argMap = mapType(argType);
              args[i] = static_cast<ffi_type*>(ffiLSTTypes[argMap]);
              values[i] = valueOut(argMap, objectRef(arguments[3]).basicAt(i+1), &dataValues[i]);
            }
          }

          ffi_type* ret;
          ret = static_cast<ffi_type*>(ffiLSTTypes[retMap]); 
          ffi_type retVal;
          void* retData = &retVal;

          ffi_cif cif;
          if(ffi_prep_cif(&cif, FFI_DEFAULT_ABI, cargTypes, ret, args) == FFI_OK)
          {
            returnedObject = MemoryManager::Instance()->newArray(cargTypes + 1);

            ffi_call(&cif, reinterpret_cast<void(*)()>(func), retData, values);
            returnedObject->basicAtPut(1, valueIn(retMap, static_cast<FFI_DataType*>(retData)));
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
              object newVal = valueIn(argMap, static_cast<FFI_DataType*>(values[i]));
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
              returnedObject = MemoryManager::Instance()->newCPointer(callback);
            }
          }
        }

        /* Deallocate both closure, and bound_puts */
        //ffi_closure_free(closure);
      }
      break;

    case 4: // dlclose
      {
        // \todo: Check type.
        FFI_LibraryHandle lib = objectRef(arguments[0]).cPointerValue();
        char* p = objectRef(arguments[1]).charPtr();
        if(NULL != lib)
        {
          int result = dlclose(lib);
          returnedObject = MemoryManager::Instance()->newInteger(result);
        }
      }
      break;


    default:
      sysError("unknown primitive","ffiPrimitive");
  }
  return(returnedObject);
}

