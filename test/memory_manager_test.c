#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include <string.h>
#include <cmockery.h>

#include "memory.h"

object firstProcess, processStack;
object classSyms[1];

void readPrimitiveTables(FILE *fp)
{
}

void writePrimitiveTables(FILE *fp)
{
}

void nameTableInsert(object dict, int hash, object key, object value)
{
}

void sysError(const char* s1, const char* s2)
{
}

object globalKey( const char* str )
{
  return (object)mock();
}

object nameTableLookup ( object dict, const char* str )
{
  return nilobj;
}

int strHash ( const char* str)
{
  return 0;
}

/* Test allocObject creates an object with correctly
 * recorded object size
 */
void allocObject_size_test(void **state)
{
  object result;
  const char* testStr = "Hello World!";
  const char* symbolName = "NewSymbol";

  result = allocObject(10);
  assert_int_equal(result->size, 10);

  result = allocByte(20);
  assert_int_equal(result->size, -20);

  result = allocStr(testStr);
  assert_int_equal(result->size, -(strlen(testStr)+1));

  result = newArray(5);
  assert_int_equal(result->size, 5);

  result = newBlock();
  assert_int_equal(result->size, 6);

  result = newByteArray(15);
  assert_int_equal(result->size, -15);

  will_return(globalKey, (object)nilobj);
  result = newClass("NewClass");
  assert_int_equal(result->size, 5);

  result = newChar('a');
  assert_int_equal(result->size, 1);

  result = newContext(0, nilobj, nilobj, nilobj);
  assert_int_equal(result->size, 8);

  result = newDictionary(25);
  assert_int_equal(result->size, 1);

  result = newInteger(1);
  assert_int_equal(result->size, -(sizeof(int)));

  result = newFloat(1.0);
  assert_int_equal(result->size, -(sizeof(double)));

  result = newMethod();
  assert_int_equal(result->size, 9);

  result = newLink(nilobj, nilobj);
  assert_int_equal(result->size, 3);

  result = newStString(testStr);
  assert_int_equal(result->size, -(strlen(testStr)+1));

  will_return(globalKey, (object)nilobj);
  result = newSymbol(symbolName);
  assert_int_equal(result->size, -(strlen(symbolName)+1));

  result = newCPointer(NULL);
  assert_int_equal(result->size, -(sizeof(void*)));
}

int main(int argc, char* argv[]) 
{
    const UnitTest tests[] = 
    {
        unit_test(allocObject_size_test),
    };
    return run_tests(tests);
}

