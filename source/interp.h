/*
    Little Smalltalk, version 2
    Written by Tim Budd, Oregon State University, July 1987
*/
/*
    symbolic definitions for the bytecodes
*/

#ifndef INTERP_H_INCLUDED
#define INTERP_H_INCLUDED

#include "memory.h"

enum EOpCodes 
{
  Op_Extended = 0,
  Op_PushInstance = 1,
  Op_PushArgument = 2,
  Op_PushTemporary = 3,
  Op_PushLiteral = 4,
  Op_PushConstant = 5,
  Op_AssignInstance = 6,
  Op_AssignTemporary = 7,
  Op_MarkArguments = 8,
  Op_SendMessage = 9,
  Op_SendUnary = 10,
  Op_SendBinary = 11,
  Op_DoPrimitive2 = 12,
  Op_DoPrimitive = 13,
  Op_DoSpecial = 15,
};

/* a few constants that can be pushed by PushConstant */
# define minusOne 3     /* the value -1 */
# define contextConst 4     /* the current context */
# define nilConst 5     /* the constant nil */
# define trueConst 6        /* the constant true */
# define falseConst 7       /* the constant false */

/* types of special instructions (opcode 15) */

# define SelfReturn 1
# define StackReturn 2
# define Duplicate 4
# define PopTop 5
# define Branch 6
# define BranchIfTrue 7
# define BranchIfFalse 8
# define AndBranch 9
# define OrBranch 10
# define SendToSuper 11

# define cacheSize 211

extern int execute(object aProcess, int maxsteps);
extern void flushCache(object messageToSend, object _class);
extern void runCode(const char * text);
extern void initialiseInterpreter();
extern void shutdownInterpreter();

#endif