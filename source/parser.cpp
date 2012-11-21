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
#include "parser.h"

bool _parseok;            /* parse still ok? */
int _codeTop;            /* top position filled in code array */
byte _codeArray[codeLimit];  /* bytecode array */
std::vector<object> _literalArray;
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
    _parseok = false;
}



void setInstanceVariables(object aClass)
{   
    int i, limit;
    object vars;

    if (aClass == nilobj)
        _instanceTop = 0;
    else 
    {
        setInstanceVariables(aClass->basicAt(superClassInClass));
        vars = aClass->basicAt(variablesInClass);
        if (vars != nilobj) 
        {
            limit = vars->size;
            for (i = 1; i <= limit; i++)
                _instanceName[++_instanceTop] = vars->basicAt(i)->charPtr();
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

void genInstruction(int high, int low)
{
    if (low >= 16) 
    {
        genInstruction(Extended, high);
        genCode(low);
    }
    else
        genCode(high * 16 + low);
}

int genLiteral(object aLiteral)
{
    _literalArray.push_back(aLiteral);
    return(_literalArray.size()-1);
}

void genInteger(int val)    /* generate an integer push */
{
    if (val == -1)
        genInstruction(PushConstant, minusOne);
    else if ((val >= 0) && (val <= 2))
        genInstruction(PushConstant, val);
    else
        genInstruction(PushLiteral,
                genLiteral(newInteger(val)));
}

const char *glbsyms[] = 
{
    "currentInterpreter", "nil", "true", "false",
    0 
};

bool nameTerm(const char *name)
{   
    int i;
    bool done = false;
    bool isSuper = false;

    /* it might be self or super */
    if (streq(name, "self") || streq(name, "super")) 
    {
        genInstruction(PushArgument, 0);
        done = true;
        if (streq(name,"super")) 
            isSuper = true;
    }

    /* or it might be a temporary (reverse this to get most recent first)*/
    if (! done)
    {
        for (i = _temporaryTop; (! done) && ( i >= 1 ) ; i--)
        {
            if (streq(name, _temporaryName[i])) 
            {
                genInstruction(PushTemporary, i-1);
                done = true;
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
                genInstruction(PushArgument, i);
                done = true;
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
                genInstruction(PushInstance, i-1);
                done = true;
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
                genInstruction(PushConstant, i+4);
                done = true;
            }
        }
    }

    /* not anything else, it must be a global */
    /* must look it up at run time */
    if (! done) 
    {
        genInstruction(PushLiteral, genLiteral(newSymbol(name)));
        genMessage(false, 0, newSymbol("value"));
    }

    return(isSuper);
}

int parseArray()
{   
    int i, size, base;
    object newLit, obj;

    base = _literalArray.size();
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
                genLiteral(newSymbol(strToken().c_str()));
                nextToken();
                break;

            case binary:
                if (strToken().compare("(") == 0) 
                {
                    parseArray();
                    break;
                }
                if (strToken().compare("-") == 0 && isdigit(peek())) 
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
                genLiteral(newSymbol(strToken().c_str()));
                nextToken();
                break;

            case charconst:
                genLiteral(newChar( intToken()));
                nextToken();
                break;

            case strconst:
                genLiteral(newStString(strToken().c_str()));
                nextToken();
                break;

            default:
                compilError(_selector,"illegal text in literal array",
                        strToken().c_str());
                nextToken();
                break;
        }
    }

    if (_parseok)
    {
        if (strToken().compare(")"))
            compilError(_selector,"array not terminated by right parenthesis",
                    strToken().c_str());
        else
            nextToken();
    }

    size = _literalArray.size() - base;
    newLit = newArray(size);
    for (i = size; i >= 1; i--) 
    {
        obj = _literalArray.back();
        newLit->basicAtPut(i, obj);
        _literalArray.pop_back();
    }
    return(genLiteral(newLit));
}

bool term()
{   
    bool superTerm = false;  /* true if term is pseudo var super */
    tokentype token = currentToken();

    if (token == nameconst) 
    {
        superTerm = nameTerm(strToken().c_str());
        nextToken();
    }
    else if (token == intconst) 
    {
        genInteger(intToken());
        nextToken();
    }
    else if (token == floatconst) 
    {
        genInstruction(PushLiteral, genLiteral(newFloat(floatToken())));
        nextToken();
    }
    else if ((token == binary) && strToken().compare("-") == 0) 
    {
        token = nextToken();
        if (token == intconst)
            genInteger(- intToken());
        else if (token == floatconst) 
        {
            genInstruction(PushLiteral,
                    genLiteral(newFloat(-floatToken())));
        }
        else
            compilError(_selector,"negation not followed",
                    "by number");
        nextToken();
    }
    else if (token == charconst) 
    {
        genInstruction(PushLiteral,
                genLiteral(newChar(intToken())));
        nextToken();
    }
    else if (token == symconst) 
    {
        genInstruction(PushLiteral,
                genLiteral(newSymbol(strToken().c_str())));
        nextToken();
    }
    else if (token == strconst) 
    {
        genInstruction(PushLiteral,
                genLiteral(newStString(strToken().c_str())));
        nextToken();
    }
    else if (token == arraybegin) 
    {
        genInstruction(PushLiteral, parseArray());
    }
    else if ((token == binary) && strToken().compare("(") == 0) 
    {
        nextToken();
        expression();
        if (_parseok)
            if ((currentToken() != closing) || strToken().compare(")"))
                compilError(_selector,"Missing Right Parenthesis","");
            else
                nextToken();
    }
    else if ((token == binary) && strToken().compare("<") == 0)
        parsePrimitive();
    else if ((token == binary) && strToken().compare("[") == 0)
        block();
    else
        compilError(_selector,"invalid expression start", strToken().c_str());

    return(superTerm);
}

void parsePrimitive()
{   
    int primitiveNumber, argumentCount;

    if (nextToken() != intconst)
        compilError(_selector,"primitive number missing","");
    primitiveNumber = intToken();
    nextToken();
    argumentCount = 0;
    while (_parseok && ! ((currentToken() == binary) && strToken().compare(">") == 0)) 
    {
        term();
        argumentCount++;
    }
    genInstruction(DoPrimitive, argumentCount);
    genCode(primitiveNumber);
    nextToken();
}

void genMessage(bool toSuper, int argumentCount, object messagesym)
{   
    bool sent = false;
    int i;

    if ((! toSuper) && (argumentCount == 0))
    {
        for (i = 0; (! sent) && i < unSyms.size() ; i++)
        {
            if (messagesym == unSyms[i]) {
                genInstruction(SendUnary, i);
                sent = true;
            }
        }
    }

    if ((! toSuper) && (argumentCount == 1))
    {
        for (i = 0; (! sent) && i < binSyms.size(); i++)
        {
            if (messagesym == binSyms[i]) 
            {
                genInstruction(SendBinary, i);
                sent = true;
            }
        }
    }

    if (! sent) 
    {
        genInstruction(MarkArguments, 1 + argumentCount);
        if (toSuper) 
        {
            genInstruction(DoSpecial, SendToSuper);
            genCode(genLiteral(messagesym));
        }
        else
            genInstruction(SendMessage, genLiteral(messagesym));
    }
}

bool unaryContinuation(bool superReceiver)
{   
    int i;
    bool sent;

    while (_parseok && (currentToken() == nameconst)) 
    {
        /* first check to see if it could be a temp by mistake */
        for (i=1; i < _temporaryTop; i++)
        {
            if (strToken().compare(_temporaryName[i]) == 0)
                compilWarn(_selector,"message same as temporary:",
                        strToken().c_str());
        }
        for (i=1; i < _argumentTop; i++)
        {
            if (strToken().compare(_argumentName[i]) == 0)
                compilWarn(_selector,"message same as argument:",
                        strToken().c_str());
        }
        /* the next generates too many spurious messages */
        /* for (i=1; i < _instanceTop; i++)
           if (streq(tokenString, _instanceName[i]))
           compilWarn(_selector,"message same as instance",
           tokenString); */

        sent = false;

        if (! sent) 
        {
            genMessage(superReceiver, 0, newSymbol(strToken().c_str()));
        }
        /* once a message is sent to super, reciever is not super */
        superReceiver = false;
        nextToken();
    }
    return(superReceiver);
}

bool binaryContinuation(bool superReceiver)
{   
    bool superTerm;
    object messagesym;
    SObjectHandle* lock_messageSym = 0;

    superReceiver = unaryContinuation(superReceiver);
    while (_parseok && (currentToken() == binary)) 
    {
        messagesym = newSymbol(strToken().c_str());
        lock_messageSym = new_SObjectHandle_from_object(messagesym);
        nextToken();
        superTerm = term();
        unaryContinuation(superTerm);
        genMessage(superReceiver, 1, messagesym);
        superReceiver = false;
        free_SObjectHandle(lock_messageSym);
    }
    return(superReceiver);
}

int optimizeBlock(int instruction, bool dopop)
{   
    int location;
    enum blockstatus savebstat;

    savebstat = _blocksat;
    genInstruction(DoSpecial, instruction);
    location = _codeTop;
    genCode(0);
    if (dopop)
        genInstruction(DoSpecial, PopTop);
    nextToken();
    if (strToken().compare("[") == 0) 
    {
        nextToken();
        if (_blocksat == NotInBlock)
            _blocksat = OptimizedBlock;
        body();
        if (strToken().compare("]"))
            compilError(_selector,"missing close","after block");
        nextToken();
    }
    else 
    {
        binaryContinuation(term());
        genMessage(false, 0, newSymbol("value"));
    }
    _codeArray[location] = _codeTop+1;
    _blocksat = savebstat;
    return(location);
}

bool keyContinuation(bool superReceiver)
{   
    int i, j, argumentCount;
    bool sent, superTerm;
    object messagesym;
    char pattern[80];

    superReceiver = binaryContinuation(superReceiver);
    if (currentToken() == namecolon) 
    {
        if (strToken().compare("ifTrue:") == 0) 
        {
            i = optimizeBlock(BranchIfFalse, false);
            if (strToken().compare("ifFalse:") == 0) 
            {
                _codeArray[i] = _codeTop + 3;
                optimizeBlock(Branch, true);
            }
        }
        else if (strToken().compare("ifFalse:") == 0) 
        {
            i = optimizeBlock(BranchIfTrue, false);
            if (strToken().compare("ifTrue:") == 0) 
            {
                _codeArray[i] = _codeTop + 3;
                optimizeBlock(Branch, true);
            }
        }
        else if (strToken().compare("whileTrue:") == 0) 
        {
            j = _codeTop;
            genInstruction(DoSpecial, Duplicate);
            genMessage(false, 0, newSymbol("value"));
            i = optimizeBlock(BranchIfFalse, false);
            genInstruction(DoSpecial, PopTop);
            genInstruction(DoSpecial, Branch);
            genCode(j+1);
            _codeArray[i] = _codeTop+1;
            genInstruction(DoSpecial, PopTop);
        }
        else if (strToken().compare("and:") == 0)
            optimizeBlock(AndBranch, false);
        else if (strToken().compare("or:") == 0)
            optimizeBlock(OrBranch, false);
        else 
        {
            pattern[0] = '\0';
            argumentCount = 0;
            while (_parseok && (currentToken() == namecolon)) 
            {
                strcat(pattern, strToken().c_str());
                argumentCount++;
                nextToken();
                superTerm = term();
                binaryContinuation(superTerm);
            }
            sent = false;

            /* check for predefined messages */
            messagesym = newSymbol(pattern);

            if (! sent) 
            {
                genMessage(superReceiver, argumentCount, messagesym);
            }
        }
        superReceiver = false;
    }
    return(superReceiver);
}

void continuation(bool superReceiver)
{
    superReceiver = keyContinuation(superReceiver);

    while (_parseok && (currentToken() == closing) && strToken().compare(";") == 0) 
    {
        genInstruction(DoSpecial, Duplicate);
        nextToken();
        keyContinuation(superReceiver);
        genInstruction(DoSpecial, PopTop);
    }
}

void expression()
{   
    bool superTerm;
    char assignname[60];

    if (currentToken() == nameconst) 
    {   /* possible assignment */
        strcpy(assignname, strToken().c_str());
        nextToken();
        if ((currentToken() == binary) && strToken().compare("<-") == 0) 
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
    bool done;

    done = false;

    /* it might be a temporary */
    for (i = _temporaryTop; (! done) && (i > 0); i--)
    {
        if (streq(name, _temporaryName[i])) 
        {
            expression();
            genInstruction(AssignTemporary, i-1);
            done = true;
        }
    }

    /* or it might be an instance variable */
    for (i = 1; (! done) && (i <= _instanceTop); i++)
    {
        if (streq(name, _instanceName[i])) 
        {
            expression();
            genInstruction(AssignInstance, i-1);
            done = true;
        }
    }

    if (! done) 
    {   /* not known, handle at run time */
        genInstruction(PushArgument, 0);
        genInstruction(PushLiteral, genLiteral(newSymbol(name)));
        expression();
        genMessage(false, 2, newSymbol("assign:value:"));
    }
}

void statement()
{

    if ((currentToken() == binary) && strToken().compare("^") == 0) 
    {
        nextToken();
        expression();
        if (_blocksat == InBlock) 
        {
            /* change return point before returning */
            genInstruction(PushConstant, contextConst);
            genMessage(false, 0, newSymbol("blockReturn"));
            genInstruction(DoSpecial, PopTop);
        }
        genInstruction(DoSpecial, StackReturn);
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
        if ((currentToken() == closing) && strToken().compare("]") == 0) 
        {
            genInstruction(PushConstant, nilConst);
            return;
        }
    }

    while(_parseok) 
    {
        statement();
        if (currentToken() == closing)
        {
            if (strToken().compare(".") == 0) 
            {
                if (nextToken() == inputend)
                    break;
                else  /* pop result, go to next statement */
                    genInstruction(DoSpecial, PopTop);
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
                        strToken().c_str());
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

    saveTemporary = _temporaryTop;
    savebstat = _blocksat;
    argumentCount = 0;
    if ((nextToken() == binary) && strToken().compare(":") == 0) 
    {
        while (_parseok && (currentToken() == binary) && strToken().compare(":") == 0) 
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
                tempsym = newSymbol(strToken().c_str());
                _temporaryName[_temporaryTop] = tempsym->charPtr();
            }
            nextToken();
        }
        if ((currentToken() != binary) || strToken().compare("|"))
            compilError(_selector,"block argument list must be terminated",
                    "by |");
        nextToken();
    }
    newBlk = newBlock();
    lock_newBlk = new_SObjectHandle_from_object(newBlk);
    newBlk->basicAtPut(argumentCountInBlock, newInteger(argumentCount));
    newBlk->basicAtPut(argumentLocationInBlock, 
            newInteger(saveTemporary + 1));
    genInstruction(PushLiteral, genLiteral(newBlk));
    genInstruction(PushConstant, contextConst);
    genInstruction(DoPrimitive, 2);
    genCode(29);
    genInstruction(DoSpecial, Branch);
    fixLocation = _codeTop;
    genCode(0);
    /*genInstruction(DoSpecial, PopTop);*/
    newBlk->basicAtPut(bytecountPositionInBlock, newInteger(_codeTop+1));
    _blocksat = InBlock;
    body();
    if ((currentToken() == closing) && strToken().compare("]") == 0)
        nextToken();
    else
        compilError(_selector,"block not terminated by ]","");
    genInstruction(DoSpecial, StackReturn);
    _codeArray[fixLocation] = _codeTop+1;
    _temporaryTop = saveTemporary;
    _blocksat = savebstat;

    free_SObjectHandle(lock_newBlk);
}

void temporaries()
{   
    object tempsym;

    _temporaryTop = 0;
    if ((currentToken() == binary) && strToken().compare("|") == 0) 
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
                tempsym = newSymbol(strToken().c_str());
                _temporaryName[_temporaryTop] = tempsym->charPtr();
            }
            nextToken();
        }
        if ((currentToken() != binary) || strToken().compare("|"))
            compilError(_selector,"temporary list not terminated by bar","");
        else
            nextToken();
    }
}

void messagePattern()
{   
    object argsym;

    _argumentTop = 0;
    strcpy(_selector, strToken().c_str());
    if (currentToken() == nameconst)     /* unary message pattern */
        nextToken();
    else if (currentToken() == binary) 
    { /* binary message pattern */
        nextToken();
        if (currentToken() != nameconst) 
            compilError(_selector,"binary message pattern not followed by name",_selector);
        argsym = newSymbol(strToken().c_str());
        _argumentName[++_argumentTop] = argsym->charPtr();
        nextToken();
    }
    else if (currentToken() == namecolon) 
    {  /* keyword message pattern */
        _selector[0] = '\0';
        while (_parseok && (currentToken() == namecolon)) 
        {
            strcat(_selector, strToken().c_str());
            nextToken();
            if (currentToken() != nameconst)
                compilError(_selector,"keyword message pattern",
                        "not followed by a name");
            if (++_argumentTop > argumentLimit)
                compilError(_selector,"too many arguments in method","");
            argsym = newSymbol(strToken().c_str());
            _argumentName[_argumentTop] = argsym->charPtr();
            nextToken();
        }
    }
    else
        compilError(_selector,"illegal message _selector", strToken().c_str());
}

bool parseCode(object method, bool savetext)
{   
    _literalArray.clear();
    _parseok = true;

    _blocksat = NotInBlock;
    _codeTop = 0;
    _temporaryTop = _argumentTop =0;
    _maxTemporary = 0;

    temporaries();
    if (_parseok)
        body();
    return(recordMethodBytecode(method, savetext));
}

bool parseMessageHandler(object method, bool savetext)
{   
    _literalArray.clear();
    _parseok = true;

    _blocksat = NotInBlock;
    _codeTop = 0;
    _temporaryTop = _argumentTop =0;
    _maxTemporary = 0;

    messagePattern();
    if (_parseok)
        temporaries();
    if (_parseok)
        body();
    return(recordMethodBytecode(method, savetext));
}


bool recordMethodBytecode(object method, bool savetext)
{
    int i;
    object bytecodes, theLiterals;
    byte *bp;

    if (_parseok) 
    {
        genInstruction(DoSpecial, PopTop);
        genInstruction(DoSpecial, SelfReturn);
    }

    if (! _parseok) 
    {
        method->basicAtPut(bytecodesInMethod, nilobj);
    }
    else 
    {
        bytecodes = newByteArray(_codeTop);
        bp = bytecodes->bytePtr();
        for (i = 0; i < _codeTop; i++) 
        {
            bp[i] = _codeArray[i];
        }
        method->basicAtPut(messageInMethod, newSymbol(_selector));
        method->basicAtPut(bytecodesInMethod, bytecodes);
        if (!_literalArray.empty()) 
        {
            theLiterals = newArray(_literalArray.size());
            for (i = 1; i <= _literalArray.size(); i++) 
            {
                theLiterals->basicAtPut(i, _literalArray[i-1]);
            }
            method->basicAtPut(literalsInMethod, theLiterals);
        }
        else 
        {
            method->basicAtPut(literalsInMethod, nilobj);
        }
        method->basicAtPut(stackSizeInMethod, newInteger(6));
        method->basicAtPut(temporarySizeInMethod,
                newInteger(1 + _maxTemporary));
        if (savetext) 
        {
            method->basicAtPut(textInMethod, newStString(source()));
        }
        return(true);
    }
    return(false);
}
