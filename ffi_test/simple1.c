#include "stdio.h"

typedef struct {
  char* string;
  int   number;
} STest;

int my_print(const char* string, int* outval, STest* test)
{
  printf("%s - %s\n", string, test->string);
  *outval = test->number;
  test->number = 199;
  test->string = "Changed in native";
  return 47;
}

