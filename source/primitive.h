#ifndef PRIMITIVE_H_INCLUDED
#define PRIMITIVE_H_INCLUDED

#include "memory.h"

/* Format example: 5b667e1c-a22d-4974-b969-d1058632a1c5 */
typedef struct _UUID
{
  unsigned long data1;
  unsigned short data2;
  unsigned short data3;
  unsigned short data4;
  unsigned char data5[6];
} UUID;
#define UUID_STRING_LENGTH 8+1+4+1+4+1+4+1+12+1

UUID stringToUUID(const char* strUUID);

typedef object(*PrimFunction)(int primitiveNumber, object* args, int argc);

typedef struct _PrimitiveTableEntry
{
  const char* name;
  PrimFunction  fn;
} PrimitiveTableEntry;

typedef struct _PrimitiveTable
{
  UUID id;
  const char* name;
  PrimitiveTableEntry* primitives;
  int numPrimitives;
  struct _PrimitiveTable* next;
} PrimitiveTable;

extern PrimitiveTable* PrimitiveTableRoot;
extern PrimitiveTable** PrimitiveTableAddresses;
extern int PrimitiveTableCount;

void addPrimitiveTable(const char* name, UUID id, PrimitiveTableEntry* primitives);
int findPrimitiveByName(const char* name, int* tableIndex);
object executePrimitive(int tableIndex, int index, object* args, int argc);
void writePrimitiveTables(FILE* fp);
void readPrimitiveTables(FILE* fp);

void initialiseDebugPrims();

#endif
