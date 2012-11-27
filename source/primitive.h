#ifndef PRIMITIVE_H_INCLUDED
#define PRIMITIVE_H_INCLUDED

#include "memory.h"


typedef object(*PrimFunction)(int primitiveNumber, object* args, int argc);

typedef struct _PrimitiveTableEntry
{
  const char* name;
  PrimFunction  fn;
} PrimitiveTableEntry;

typedef struct _PrimitiveTable
{
  PrimitiveTableEntry* primitives;
  struct _PrimitiveTable* next;
} PrimitiveTable;

extern PrimitiveTable* PrimitiveTableRoot;
extern PrimitiveTable* PrimitivesTableAddresses;
extern int PrimitiveTableCount;

void addPrimitiveTable(PrimitiveTableEntry* primitives);
int findPrimitiveByName(const char* name, int* tableIndex);

void initialiseDebugPrims();

#endif
