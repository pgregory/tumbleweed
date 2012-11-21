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

bool parseMessageHandler(object method, bool savetext);
bool parseCode(object method, bool savetext);

void setInstanceVariables(object aClass);


void genMessage(bool toSuper, int argumentCount, object messagesym);
void genInstruction(int high, int low);
void genCode(int value);
int genLiteral(object aLiteral);
void genInteger(int val);
void expression();
bool unaryContinuation(bool superReceiver);
bool binaryContinuation(bool superReceiver);
int optimizeBlock(int instruction, bool dopop);
bool keyContinuation(bool superReceiver);
void continuation(bool superReceiver);
void parsePrimitive();
int parseArray();
void block();
void body();
void statement();
void assignment(char* name);
void temporaries();
void messagePattern();
bool nameTerm(const char *name);
bool term();
bool recordMethodBytecode(object method, bool savetext);

void compilWarn(const char* selector, const char* str1, const char* str2);
void compilError(const char* selector, const char* str1, const char* str2);

