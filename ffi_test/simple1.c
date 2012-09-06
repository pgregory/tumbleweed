#include "stdio.h"

#if defined WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

typedef struct {
  char* string;
  int   number;
} STest;

EXPORT int my_print(const char* string, int* outval, STest* test)
{
  printf("%s - %s\n", string, test->string);
  *outval = test->number;
  test->number = 199;
  test->string = "Changed in native";
  return 47;
}

