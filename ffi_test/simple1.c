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

typedef int(*callback)(const char*, int);

EXPORT int my_print(const char* string, int* outval, STest* test, callback cb)
{
  printf("%s - %s\n", string, test->string);
  *outval = test->number;
  test->number = 199;
  test->string = "Changed in native";

  if(cb)
  {
    int cbresult = (*cb)("Called from native", 69);
    printf("Callback responded with: %d\n", cbresult); 
  }

  return 47;
}

