#include "memory.h"
#include "names.h"
#include "interp.h"
#include "lex.h"


/* all of the following limits could be increased (up to
   256) without any trouble.  They are kept low 
   to keep memory utilization down */

# define codeLimit 1024     /* maximum number of bytecodes permitted */
# define literalLimit 128   /* maximum number of literals permitted */
# define temporaryLimit 32  /* maximum number of temporaries permitted */
# define argumentLimit 32   /* maximum number of arguments permitted */
# define instanceLimit 32   /* maximum number of instance vars permitted */
# define methodLimit 64     /* maximum number of methods permitted */

int parseMessageHandler(object method, int savetext);
int parseCode(object method, int savetext);

void setInstanceVariables(object aClass);


void genMessage(int toSuper, int argumentCount, object messagesym);
void genInstruction(int high, int low);
void genCode(int value);
int genLiteral(object aLiteral);
void genInteger(int val);
void expression();
int unaryContinuation(int superReceiver);
int binaryContinuation(int superReceiver);
int optimizeBlock(int instruction, int dopop);
int keyContinuation(int superReceiver);
void continuation(int superReceiver);
void parsePrimitive();
int parseArray();
void block();
void body();
void statement();
void assignment(char* name);
void temporaries();
void messagePattern();
int nameTerm(const char *name);
int term();
int recordMethodBytecode(object method, int savetext);

void compilWarn(const char* selector, const char* str1, const char* str2);
void compilError(const char* selector, const char* str1, const char* str2);

