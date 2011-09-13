#include "stdio.h"

int my_print(const char* string, int* outval)
{
  printf("%s\n", string);
  *outval = 99;
  return 47;
}

