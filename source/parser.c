/*
   Little Smalltalk, version 2
   Written by Tim Budd, Oregon State University, July 1987

   Method parser - parses the textual description of a method,
   generating bytecodes and literals.

   This parser is based around a simple minded recursive descent
   parser.
   It is used both by the module that builds the initial virtual image,
   and by a primitive when invoked from a running Smalltalk system.

   The latter case could, if the bytecode interpreter were fast enough,
   be replaced by a parser written in Smalltalk.  This would be preferable,
   but not if it slowed down the system too terribly.

   To use the parser the routine setInstanceVariables must first be
   called with a class object.  This places the appropriate instance
   variables into the memory buffers, so that references to them
   can be correctly encoded.

   As this is recursive descent, you should read it SDRAWKCAB !
   (from bottom to top)
   */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "env.h"
#include "interp.h"
#include "parser.h"
#include "primitive.h"

int _parseok;            /* parse still ok? */
int _codeTop;            /* top position filled in code array */
byte _codeArray[codeLimit];  /* bytecode array */
int _literalTop;
object _literalArray[literalLimit];
int _temporaryTop;
char *_temporaryName[temporaryLimit];
int _argumentTop;
char *_argumentName[argumentLimit];
int _instanceTop;
char *_instanceName[instanceLimit];

int _maxTemporary;      /* highest temporary see so far */
char _selector[80];     /* message _selector */

enum blockstatus 
{
    NotInBlock, 
    InBlock, 
    OptimizedBlock
} _blocksat;

void compilWarn(const char* _selector, const char* str1, const char* str2)
{
    fprintf(stderr,"compiler warning: Method %s : %s %s\n", 
            _selector, str1, str2);
}


void compilError(const char* _selector, const char* str1, const char* str2)
{
    fprintf(stderr,"compiler error: Method %s : %s %s\n", 
            _selector, str1, str2);
    //_snprintf(gLastError, 1024, "compiler error: Method %s : %s %s", _selector, str1, str2);
    _parseok = FALSE;
}



void setInstanceVariables(object aClass)
{   
    int i, limit;
    object vars;

    if (aClass == nilobj)
        _instanceTop = 0;
    else 
    {
        setInstanceVariables(basicAt(aClass,superClassInClass));
        vars = basicAt(aClass,variablesInClass);
        if (vars != nilobj) 
        {
            limit = vars->size;
            for (i = 1; i <= limit; i++)
                _instanceName[++_instanceTop] = charPtr(basicAt(vars,i));
        }
    }
}

void genCode(int value)
{
    if (_codeTop >= codeLimit)
        compilError(_selector,"too many bytecode instructions in method","");
    else
        _codeArray[_codeTop++] = value;
}

void genInstruction(enum EOpCodes high, int low)
{
    if (low >= 16) 
    {
        genInstruction(Op_Extended, high);
        genCode(low);
    }
    else
        genCode(high * 16 + low);
}

int genLiteral(object aLiteral)
{
    if (_literalTop >= literalLimit)
        compilError(_selector,"too many literals in method","");
    else 
    {
        _literalArray[++_literalTop] = aLiteral;
    }
    return(_literalTop - 1);
}

void genInteger(int val)    /* generate an integer push */
{
    if (val == -1)
        genInstruction(Op_PushConstant, minusOne);
    else if ((val >= 0) && (val <= 2))
        genInstruction(Op_PushConstant, val);
    else
        genInstruction(Op_PushLiteral,
                genLiteral(newInteger(val)));
}

const char *glbsyms[] = 
{
    "currentInterpreter", "nil", "true", "false",
    0 
};

int nameTerm(const char *name)
{   
    int i;
    int done = FALSE;
    int isSuper = FALSE;

    /* it might be self or super */
    if (streq(name, "self") || streq(name, "super")) 
    {
        genInstruction(Op_PushArgument, 0);
        done = TRUE;
        if (streq(name,"super")) 
            isSuper = TRUE;
    }

    /* or it might be a temporary (reverse this to get most recent first)*/
    if (! done)
    {
        for (i = _temporaryTop; (! done) && ( i >= 1 ) ; i--)
        {
            if (streq(name, _temporaryName[i])) 
            {
                genInstruction(Op_PushTemporary, i-1);
                done = TRUE;
            }
        }
    }

    /* or it might be an argument */
    if (! done)
    {
        for (i = 1; (! done) && (i <= _argumentTop ) ; i++)
        {
            if (streq(name, _argumentName[i])) 
            {
                genInstruction(Op_PushArgument, i);
                done = TRUE;
            }
        }
    }

    /* or it might be an instance variable */
    if (! done)
    {
        for (i = 1; (! done) && (i <= _instanceTop); i++) 
        {
            if (streq(name, _instanceName[i])) 
            {
                genInstruction(Op_PushInstance, i-1);
                done = TRUE;
            }
        }
    }

    /* or it might be a global constant */
    if (! done)
    {
        for (i = 0; (! done) && glbsyms[i]; i++)
        {
            if (streq(name, glbsyms[i])) 
            {
                genInstruction(Op_PushConstant, i+4);
                done = TRUE;
            }
        }
    }

    /* not anything else, it must be a global */
    /* must look it up at run time */
    if (! done) 
    {
        genInstruction(Op_PushLiteral, genLiteral(newSymbol(name)));
        genMessage(0, 0, newSymbol("value"));
    }

    return(isSuper);
}

int parseArray()
{   
    int i, size, base;
    object newLit, obj;

    base = _literalTop;
    nextToken();
    while (_parseok && (currentToken() != closing)) 
    {
        switch(currentToken()) 
        {
            case arraybegin:
                parseArray();
                break;

            case intconst:
                genLiteral(newInteger(intToken()));
                nextToken();
                break;

            case floatconst:
                genLiteral(newFloat(floatToken()));
                nextToken();
                break;

            case nameconst: case namecolon: case symconst:
                genLiteral(newSymbol(strToken()));
                nextToken();
                break;

            case binary:
                if (strcmp(strToken(), "(") == 0) 
                {
                    parseArray();
                    break;
                }
                if (strcmp(strToken(), "-") == 0 && isdigit(peek())) 
                {
                    nextToken();
                    if (currentToken() == intconst)
                        genLiteral(newInteger(-intToken()));
                    else if (currentToken() == floatconst) 
                    {
                        genLiteral(newFloat(-floatToken()));
                    }
                    else
                        compilError(_selector,"negation not followed", "by number");
                    nextToken();
                    break;
                }
                genLiteral(newSymbol(strToken()));
                nextToken();
                break;

            case charconst:
                genLiteral(newChar( intToken()));
                nextToken();
                break;

            case strconst:
                genLiteral(newStString(strToken()));
                nextToken();
                break;

            default:
                compilError(_selector,"illegal text in literal array",
                        strToken());
                nextToken();
                break;
        }
    }

    if (_parseok)
    {
        if (strcmp(strToken(), ")"))
            compilError(_selector,"array not terminated by right parenthesis",
                    strToken());
        else
            nextToken();
    }

    size = _literalTop - base;
    newLit = newArray(size);
    for (i = size; i >= 1; i--) 
    {
        obj = _literalArray[_literalTop];
        basicAtPut(newLit,i, obj);
        _literalArray[_literalTop] = nilobj;
        _literalTop = _literalTop - 1;
    }
    return(genLiteral(newLit));
}

int term()
{   
    int superTerm = 0;  /* 1 if term is pseudo var super */
    tokentype token = currentToken();

    if (token == nameconst) 
    {
        superTerm = nameTerm(strToken());
        nextToken();
    }
    else if (token == intconst) 
    {
        genInteger(intToken());
        nextToken();
    }
    else if (token == floatconst) 
    {
        genInstruction(Op_PushLiteral, genLiteral(newFloat(floatToken())));
        nextToken();
    }
    else if ((token == binary) && strcmp(strToken(), "-") == 0) 
    {
        token = nextToken();
        if (token == intconst)
            genInteger(- intToken());
        else if (token == floatconst) 
        {
            genInstruction(Op_PushLiteral,
                    genLiteral(newFloat(-floatToken())));
        }
        else
            compilError(_selector,"negation not followed",
                    "by number");
        nextToken();
    }
    else if (token == charconst) 
    {
        genInstruction(Op_PushLiteral,
                genLiteral(newChar(intToken())));
        nextToken();
    }
    else if (token == symconst) 
    {
        genInstruction(Op_PushLiteral,
                genLiteral(newSymbol(strToken())));
        nextToken();
    }
    else if (token == strconst) 
    {
        genInstruction(Op_PushLiteral,
                genLiteral(newStString(strToken())));
        nextToken();
    }
    else if (token == arraybegin) 
    {
        genInstruction(Op_PushLiteral, parseArray());
    }
    else if ((token == binary) && strcmp(strToken(), "(") == 0) 
    {
        nextToken();
        expression();
        if (_parseok)
            if ((currentToken() != closing) || strcmp(strToken(), ")"))
                compilError(_selector,"Missing Right Parenthesis","");
            else
                nextToken();
    }
    else if ((token == binary) && strcmp(strToken(), "<") == 0)
        parsePrimitive();
    else if ((token == binary) && strcmp(strToken(), "[") == 0)
        block();
    else
        compilError(_selector,"invalid expression start", strToken());

    return(superTerm);
}

void parsePrimitive()
{   
    int primitiveNumber, argumentCount;
    int primitiveTableNumber, primitiveIndex;

    if (nextToken() == intconst)
    {
        primitiveNumber = intToken();
        nextToken();
        argumentCount = 0;
        while (_parseok && ! ((currentToken() == binary) && strcmp(strToken(), ">") == 0)) 
        {
            term();
            argumentCount++;
        }
        genInstruction(Op_DoPrimitive, argumentCount);
        genCode(primitiveNumber);
        nextToken();
    }
    else if(currentToken() == nameconst)
    {
        primitiveIndex = findPrimitiveByName(strToken(), &primitiveTableNumber);
        if(primitiveIndex >= 0)
        {
            nextToken();
            argumentCount = 0;
            while (_parseok && ! ((currentToken() == binary) && strcmp(strToken(), ">") == 0)) 
            {
                term();
                argumentCount++;
            }
            genInstruction(Op_DoPrimitive2, argumentCount);
            genCode(primitiveTableNumber);
            genCode(primitiveIndex);
            nextToken();
        }
        else
            compilError(_selector, "primitive error", "cannot find primitive");
    }
    else
    {
        compilError(_selector,"primitive error", "malformed primitive");
    }
}

void genMessage(int toSuper, int argumentCount, object messagesym)
{   
    int sent = 0;
    int i;

    if ((! toSuper) && (argumentCount == 0))
    {
        for (i = 0; (! sent) && i < num_unSyms ; i++)
        {
            if (messagesym == unSyms[i]) {
                genInstruction(Op_SendUnary, i);
                sent = TRUE;
            }
        }
    }

    if ((! toSuper) && (argumentCount == 1))
    {
        for (i = 0; (! sent) && i < num_binSyms; i++)
        {
            if (messagesym == binSyms[i]) 
            {
                genInstruction(Op_SendBinary, i);
                sent = TRUE;
            }
        }
    }

    if (! sent) 
    {
        genInstruction(Op_MarkArguments, 1 + argumentCount);
        if (toSuper) 
        {
            genInstruction(Op_DoSpecial, SendToSuper);
            genCode(genLiteral(messagesym));
        }
        else
            genInstruction(Op_SendMessage, genLiteral(messagesym));
    }
}

int unaryContinuation(int superReceiver)
{   
    int i;
    int sent;

    while (_parseok && (currentToken() == nameconst)) 
    {
        /* first check to see if it could be a temp by mistake */
        for (i=1; i < _temporaryTop; i++)
        {
            if (strcmp(strToken(), _temporaryName[i]) == 0)
                compilWarn(_selector,"message same as temporary:",
                        strToken());
        }
        for (i=1; i < _argumentTop; i++)
        {
            if (strcmp(strToken(), _argumentName[i]) == 0)
                compilWarn(_selector,"message same as argument:",
                        strToken());
        }
        /* the next generates too many spurious messages */
        /* for (i=1; i < _instanceTop; i++)
           if (streq(tokenString, _instanceName[i]))
           compilWarn(_selector,"message same as instance",
           tokenString); */

        sent = 0;

        if (! sent) 
        {
            genMessage(superReceiver, 0, newSymbol(strToken()));
        }
        /* once a message is sent to super, reciever is not super */
        superReceiver = 0;
        nextToken();
    }
    return(superReceiver);
}

int binaryContinuation(int superReceiver)
{   
    int superTerm;
    object messagesym;
    SObjectHandle* lock_messageSym = 0;

    superReceiver = unaryContinuation(superReceiver);
    while (_parseok && (currentToken() == binary)) 
    {
        messagesym = newSymbol(strToken());
        lock_messageSym = new_SObjectHandle_from_object(messagesym);
        nextToken();
        superTerm = term();
        unaryContinuation(superTerm);
        genMessage(superReceiver, 1, messagesym);
        superReceiver = 0;
        free_SObjectHandle(lock_messageSym);
    }
    return(superReceiver);
}

int optimizeBlock(int instruction, int dopop)
{   
    int location;
    enum blockstatus savebstat;

    savebstat = _blocksat;
    genInstruction(Op_DoSpecial, instruction);
    location = _codeTop;
    genCode(0);
    if (dopop)
        genInstruction(Op_DoSpecial, PopTop);
    nextToken();
    if (strcmp(strToken(), "[") == 0) 
    {
        nextToken();
        if (_blocksat == NotInBlock)
            _blocksat = OptimizedBlock;
        body();
        if (strcmp(strToken(), "]"))
            compilError(_selector,"missing close","after block");
        nextToken();
    }
    else 
    {
        binaryContinuation(term());
        genMessage(0, 0, newSymbol("value"));
    }
    _codeArray[location] = _codeTop+1;
    _blocksat = savebstat;
    return(location);
}

int keyContinuation(int superReceiver)
{   
    int i, j, argumentCount;
    int sent, superTerm;
    object messagesym;
    char pattern[80];

    superReceiver = binaryContinuation(superReceiver);
    if (currentToken() == namecolon) 
    {
        if (strcmp(strToken(), "ifTrue:") == 0) 
        {
            i = optimizeBlock(BranchIfFalse, 0);
            if (strcmp(strToken(), "ifFalse:") == 0) 
            {
                _codeArray[i] = _codeTop + 3;
                optimizeBlock(Branch, 1);
            }
        }
        else if (strcmp(strToken(), "ifFalse:") == 0) 
        {
            i = optimizeBlock(BranchIfTrue, 0);
            if (strcmp(strToken(), "ifTrue:") == 0) 
            {
                _codeArray[i] = _codeTop + 3;
                optimizeBlock(Branch, 1);
            }
        }
        else if (strcmp(strToken(), "whileTrue:") == 0) 
        {
            j = _codeTop;
            genInstruction(Op_DoSpecial, Duplicate);
            genMessage(0, 0, newSymbol("value"));
            i = optimizeBlock(BranchIfFalse, 0);
            genInstruction(Op_DoSpecial, PopTop);
            genInstruction(Op_DoSpecial, Branch);
            genCode(j+1);
            _codeArray[i] = _codeTop+1;
            genInstruction(Op_DoSpecial, PopTop);
        }
        else if (strcmp(strToken(), "and:") == 0)
            optimizeBlock(AndBranch, 0);
        else if (strcmp(strToken(), "or:") == 0)
            optimizeBlock(OrBranch, 0);
        else 
        {
            pattern[0] = '\0';
            argumentCount = 0;
            while (_parseok && (currentToken() == namecolon)) 
            {
                strcat(pattern, strToken());
                argumentCount++;
                nextToken();
                superTerm = term();
                binaryContinuation(superTerm);
            }
            sent = 0;

            /* check for predefined messages */
            messagesym = newSymbol(pattern);

            if (! sent) 
            {
                genMessage(superReceiver, argumentCount, messagesym);
            }
        }
        superReceiver = 0;
    }
    return(superReceiver);
}

void continuation(int superReceiver)
{
    superReceiver = keyContinuation(superReceiver);

    while (_parseok && (currentToken() == closing) && strcmp(strToken(), ";") == 0) 
    {
        genInstruction(Op_DoSpecial, Duplicate);
        nextToken();
        keyContinuation(superReceiver);
        genInstruction(Op_DoSpecial, PopTop);
    }
}

void expression()
{   
    int superTerm;
    char assignname[60];

    if (currentToken() == nameconst) 
    {   /* possible assignment */
        strcpy(assignname, strToken());
        nextToken();
        if ((currentToken() == binary) && strcmp(strToken(), "<-") == 0) 
        {
            nextToken();
            assignment(assignname);
        }
        else 
        {      /* not an assignment after all */
            superTerm = nameTerm(assignname);
            continuation(superTerm);
        }
    }
    else 
    {
        superTerm = term();
        if (_parseok)
            continuation(superTerm);
    }
}

void assignment(char* name)
{   
    int i;
    int done;

    done = 0;

    /* it might be a temporary */
    for (i = _temporaryTop; (! done) && (i > 0); i--)
    {
        if (streq(name, _temporaryName[i])) 
        {
            expression();
            genInstruction(Op_AssignTemporary, i-1);
            done = 1;
        }
    }

    /* or it might be an instance variable */
    for (i = 1; (! done) && (i <= _instanceTop); i++)
    {
        if (streq(name, _instanceName[i])) 
        {
            expression();
            genInstruction(Op_AssignInstance, i-1);
            done = 1;
        }
    }

    if (! done) 
    {   /* not known, handle at run time */
        genInstruction(Op_PushArgument, 0);
        genInstruction(Op_PushLiteral, genLiteral(newSymbol(name)));
        expression();
        genMessage(0, 2, newSymbol("assign:value:"));
    }
}

void statement()
{

    if ((currentToken() == binary) && strcmp(strToken(), "^") == 0) 
    {
        nextToken();
        expression();
        if (_blocksat == InBlock) 
        {
            /* change return point before returning */
            genInstruction(Op_PushConstant, contextConst);
            genMessage(0, 0, newSymbol("blockReturn"));
            genInstruction(Op_DoSpecial, PopTop);
        }
        genInstruction(Op_DoSpecial, StackReturn);
    }
    else 
    {
        expression();
    }
}

void body()
{
    /* empty blocks are same as nil */
    if ((_blocksat == InBlock) || (_blocksat == OptimizedBlock))
    {
        if ((currentToken() == closing) && strcmp(strToken(), "]") == 0) 
        {
            genInstruction(Op_PushConstant, nilConst);
            return;
        }
    }

    while(_parseok) 
    {
        statement();
        if (currentToken() == closing)
        {
            if (strcmp(strToken(), ".") == 0) 
            {
                if (nextToken() == inputend)
                    break;
                else  /* pop result, go to next statement */
                    genInstruction(Op_DoSpecial, PopTop);
            }
            else
                break;  /* leaving result on stack */
        }
        else
        {
            if (currentToken() == inputend)
                break;  /* leaving result on stack */
            else 
            {
                compilError(_selector,"invalid statement ending; token is ",
                        strToken());
            }
        }
    }
}

void block()
{   
    int saveTemporary, argumentCount, fixLocation;
    object tempsym; 
    object newBlk;
    SObjectHandle* lock_newBlk = 0;
    enum blockstatus savebstat;
    int primitiveTableNumber, primitiveIndex;

    saveTemporary = _temporaryTop;
    savebstat = _blocksat;
    argumentCount = 0;
    if ((nextToken() == binary) && strcmp(strToken(), ":") == 0) 
    {
        while (_parseok && (currentToken() == binary) && strcmp(strToken(), ":") == 0) 
        {
            if (nextToken() != nameconst)
                compilError(_selector,"name must follow colon",
                        "in block argument list");
            if (++_temporaryTop > _maxTemporary)
                _maxTemporary = _temporaryTop;
            argumentCount++;
            if (_temporaryTop > temporaryLimit)
                compilError(_selector,"too many temporaries in method","");
            else 
            {
                tempsym = newSymbol(strToken());
                _temporaryName[_temporaryTop] = charPtr(tempsym);
            }
            nextToken();
        }
        if ((currentToken() != binary) || strcmp(strToken(), "|"))
            compilError(_selector,"block argument list must be terminated",
                    "by |");
        nextToken();
    }
    newBlk = newBlock();
    lock_newBlk = new_SObjectHandle_from_object(newBlk);
    basicAtPut(newBlk,argumentCountInBlock, newInteger(argumentCount));
    basicAtPut(newBlk,argumentLocationInBlock, 
            newInteger(saveTemporary + 1));
    genInstruction(Op_PushLiteral, genLiteral(newBlk));
    genInstruction(Op_PushConstant, contextConst);
    genInstruction(Op_DoPrimitive2, 2);
    primitiveIndex = findPrimitiveByName("blockCreateContext", &primitiveTableNumber);
    genCode(primitiveTableNumber);
    genCode(primitiveIndex);
    genInstruction(Op_DoSpecial, Branch);
    fixLocation = _codeTop;
    genCode(0);
    /*genInstruction(DoSpecial, PopTop);*/
    basicAtPut(newBlk,bytecountPositionInBlock, newInteger(_codeTop+1));
    _blocksat = InBlock;
    body();
    if ((currentToken() == closing) && strcmp(strToken(), "]") == 0)
        nextToken();
    else
        compilError(_selector,"block not terminated by ]","");
    genInstruction(Op_DoSpecial, StackReturn);
    _codeArray[fixLocation] = _codeTop+1;
    _temporaryTop = saveTemporary;
    _blocksat = savebstat;

    free_SObjectHandle(lock_newBlk);
}

void temporaries()
{   
    object tempsym;

    _temporaryTop = 0;
    if ((currentToken() == binary) && strcmp(strToken(), "|") == 0) 
    {
        nextToken();
        while (currentToken() == nameconst) 
        {
            if (++_temporaryTop > _maxTemporary)
                _maxTemporary = _temporaryTop;
            if (_temporaryTop > temporaryLimit)
                compilError(_selector,"too many temporaries in method","");
            else 
            {
                tempsym = newSymbol(strToken());
                _temporaryName[_temporaryTop] = charPtr(tempsym);
            }
            nextToken();
        }
        if ((currentToken() != binary) || strcmp(strToken(), "|"))
            compilError(_selector,"temporary list not terminated by bar","");
        else
            nextToken();
    }
}

void messagePattern()
{   
    object argsym;

    _argumentTop = 0;
    strcpy(_selector, strToken());
    if (currentToken() == nameconst)     /* unary message pattern */
        nextToken();
    else if (currentToken() == binary) 
    { /* binary message pattern */
        nextToken();
        if (currentToken() != nameconst) 
            compilError(_selector,"binary message pattern not followed by name",_selector);
        argsym = newSymbol(strToken());
        _argumentName[++_argumentTop] = charPtr(argsym);
        nextToken();
    }
    else if (currentToken() == namecolon) 
    {  /* keyword message pattern */
        _selector[0] = '\0';
        while (_parseok && (currentToken() == namecolon)) 
        {
            strcat(_selector, strToken());
            nextToken();
            if (currentToken() != nameconst)
                compilError(_selector,"keyword message pattern",
                        "not followed by a name");
            if (++_argumentTop > argumentLimit)
                compilError(_selector,"too many arguments in method","");
            argsym = newSymbol(strToken());
            _argumentName[_argumentTop] = charPtr(argsym);
            nextToken();
        }
    }
    else
        compilError(_selector,"illegal message _selector", strToken());
}

int parseCode(object method, int savetext)
{   
    int result;

    _parseok = 1;
    g_DisableGC = 1;

    _blocksat = NotInBlock;
    _codeTop = 0;
    _literalTop = _temporaryTop = _argumentTop =0;
    _maxTemporary = 0;

    temporaries();
    if (_parseok)
        body();
    
    result = recordMethodBytecode(method, savetext);
    g_DisableGC = 0;

    return result;
}

int parseMessageHandler(object method, int savetext)
{   
    int result;

    _parseok = 1;
    g_DisableGC = 1;

    _blocksat = NotInBlock;
    _codeTop = 0;
    _literalTop = _temporaryTop = _argumentTop =0;
    _maxTemporary = 0;

    messagePattern();
    if (_parseok)
        temporaries();
    if (_parseok)
        body();
    
    result = recordMethodBytecode(method, savetext);
    g_DisableGC = 0;

    return result;
}


int recordMethodBytecode(object method, int savetext)
{
    int i;
    object bytecodes, theLiterals;
    byte *bp;

    if (_parseok) 
    {
        genInstruction(Op_DoSpecial, PopTop);
        genInstruction(Op_DoSpecial, SelfReturn);
    }

    if (! _parseok) 
    {
        basicAtPut(method,bytecodesInMethod, nilobj);
    }
    else 
    {
        bytecodes = newByteArray(_codeTop);
        bp = bytePtr(bytecodes);
        for (i = 0; i < _codeTop; i++) 
        {
            bp[i] = _codeArray[i];
        }
        basicAtPut(method,messageInMethod, newSymbol(_selector));
        basicAtPut(method,bytecodesInMethod, bytecodes);
        if (_literalTop > 0) 
        {
            theLiterals = newArray(_literalTop);
            for (i = 1; i <= _literalTop; ++i) 
            {
                basicAtPut(theLiterals,i, _literalArray[i]);
            }
            basicAtPut(method,literalsInMethod, theLiterals);
        }
        else 
        {
            basicAtPut(method,literalsInMethod, nilobj);
        }
        basicAtPut(method,stackSizeInMethod, newInteger(6));
        basicAtPut(method,temporarySizeInMethod,
                newInteger(1 + _maxTemporary));
        if (savetext) 
        {
            basicAtPut(method,textInMethod, newStString(source()));
        }
        return(1);
    }
    return(0);
}
