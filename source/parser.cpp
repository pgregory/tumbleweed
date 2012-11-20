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

void Parser::compilWarn(const char* selector, const char* str1, const char* str2)
{
    fprintf(stderr,"compiler warning: Method %s : %s %s\n", 
            selector, str1, str2);
}


void Parser::compilError(const char* selector, const char* str1, const char* str2)
{
    fprintf(stderr,"compiler error: Method %s : %s %s\n", 
            selector, str1, str2);
    //_snprintf(gLastError, 1024, "compiler error: Method %s : %s %s", selector, str1, str2);
    parseok = false;
}



void Parser::setInstanceVariables(object aClass)
{   
    int i, limit;
    object vars;

    if (aClass == nilobj)
        instanceTop = 0;
    else 
    {
        setInstanceVariables(aClass->basicAt(superClassInClass));
        vars = aClass->basicAt(variablesInClass);
        if (vars != nilobj) 
        {
            limit = vars->size;
            for (i = 1; i <= limit; i++)
                instanceName[++instanceTop] = vars->basicAt(i)->charPtr();
        }
    }
}

void Parser::genCode(int value)
{
    if (codeTop >= codeLimit)
        compilError(selector,"too many bytecode instructions in method","");
    else
        codeArray[codeTop++] = value;
}

void Parser::genInstruction(int high, int low)
{
    if (low >= 16) 
    {
        genInstruction(Extended, high);
        genCode(low);
    }
    else
        genCode(high * 16 + low);
}

int Parser::genLiteral(object aLiteral)
{
    literalArray.push_back(aLiteral);
    return(literalArray.size()-1);
}

void Parser::genInteger(int val)    /* generate an integer push */
{
    if (val == -1)
        genInstruction(PushConstant, minusOne);
    else if ((val >= 0) && (val <= 2))
        genInstruction(PushConstant, val);
    else
        genInstruction(PushLiteral,
                genLiteral(MemoryManager::Instance()->newInteger(val)));
}

const char *glbsyms[] = 
{
    "currentInterpreter", "nil", "true", "false",
    0 
};

bool Parser::nameTerm(const char *name)
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
        for (i = temporaryTop; (! done) && ( i >= 1 ) ; i--)
        {
            if (streq(name, temporaryName[i])) 
            {
                genInstruction(PushTemporary, i-1);
                done = true;
            }
        }
    }

    /* or it might be an argument */
    if (! done)
    {
        for (i = 1; (! done) && (i <= argumentTop ) ; i++)
        {
            if (streq(name, argumentName[i])) 
            {
                genInstruction(PushArgument, i);
                done = true;
            }
        }
    }

    /* or it might be an instance variable */
    if (! done)
    {
        for (i = 1; (! done) && (i <= instanceTop); i++) 
        {
            if (streq(name, instanceName[i])) 
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
        genInstruction(PushLiteral, genLiteral(MemoryManager::Instance()->newSymbol(name)));
        genMessage(false, 0, MemoryManager::Instance()->newSymbol("value"));
    }

    return(isSuper);
}

int Parser::parseArray()
{   
    int i, size, base;
    object newLit, obj;

    base = literalArray.size();
    m_lexer.nextToken();
    while (parseok && (m_lexer.currentToken() != closing)) 
    {
        switch(m_lexer.currentToken()) 
        {
            case arraybegin:
                parseArray();
                break;

            case intconst:
                genLiteral(MemoryManager::Instance()->newInteger(m_lexer.intToken()));
                m_lexer.nextToken();
                break;

            case floatconst:
                genLiteral(MemoryManager::Instance()->newFloat(m_lexer.floatToken()));
                m_lexer.nextToken();
                break;

            case nameconst: case namecolon: case symconst:
                genLiteral(MemoryManager::Instance()->newSymbol(m_lexer.strToken().c_str()));
                m_lexer.nextToken();
                break;

            case binary:
                if (m_lexer.strToken().compare("(") == 0) 
                {
                    parseArray();
                    break;
                }
                if (m_lexer.strToken().compare("-") == 0 && isdigit(m_lexer.peek())) 
                {
                    m_lexer.nextToken();
                    if (m_lexer.currentToken() == intconst)
                        genLiteral(MemoryManager::Instance()->newInteger(-m_lexer.intToken()));
                    else if (m_lexer.currentToken() == floatconst) 
                    {
                        genLiteral(MemoryManager::Instance()->newFloat(-m_lexer.floatToken()));
                    }
                    else
                        compilError(selector,"negation not followed", "by number");
                    m_lexer.nextToken();
                    break;
                }
                genLiteral(MemoryManager::Instance()->newSymbol(m_lexer.strToken().c_str()));
                m_lexer.nextToken();
                break;

            case charconst:
                genLiteral(MemoryManager::Instance()->newChar( m_lexer.intToken()));
                m_lexer.nextToken();
                break;

            case strconst:
                genLiteral(MemoryManager::Instance()->newStString(m_lexer.strToken().c_str()));
                m_lexer.nextToken();
                break;

            default:
                compilError(selector,"illegal text in literal array",
                        m_lexer.strToken().c_str());
                m_lexer.nextToken();
                break;
        }
    }

    if (parseok)
    {
        if (m_lexer.strToken().compare(")"))
            compilError(selector,"array not terminated by right parenthesis",
                    m_lexer.strToken().c_str());
        else
            m_lexer.nextToken();
    }

    size = literalArray.size() - base;
    newLit = MemoryManager::Instance()->newArray(size);
    for (i = size; i >= 1; i--) 
    {
        obj = literalArray.back();
        newLit->basicAtPut(i, obj);
        literalArray.pop_back();
    }
    return(genLiteral(newLit));
}

bool Parser::term()
{   
    bool superTerm = false;  /* true if term is pseudo var super */
    tokentype token = m_lexer.currentToken();

    if (token == nameconst) 
    {
        superTerm = nameTerm(m_lexer.strToken().c_str());
        m_lexer.nextToken();
    }
    else if (token == intconst) 
    {
        genInteger(m_lexer.intToken());
        m_lexer.nextToken();
    }
    else if (token == floatconst) 
    {
        genInstruction(PushLiteral, genLiteral(MemoryManager::Instance()->newFloat(m_lexer.floatToken())));
        m_lexer.nextToken();
    }
    else if ((token == binary) && m_lexer.strToken().compare("-") == 0) 
    {
        token = m_lexer.nextToken();
        if (token == intconst)
            genInteger(- m_lexer.intToken());
        else if (token == floatconst) 
        {
            genInstruction(PushLiteral,
                    genLiteral(MemoryManager::Instance()->newFloat(-m_lexer.floatToken())));
        }
        else
            compilError(selector,"negation not followed",
                    "by number");
        m_lexer.nextToken();
    }
    else if (token == charconst) 
    {
        genInstruction(PushLiteral,
                genLiteral(MemoryManager::Instance()->newChar(m_lexer.intToken())));
        m_lexer.nextToken();
    }
    else if (token == symconst) 
    {
        genInstruction(PushLiteral,
                genLiteral(MemoryManager::Instance()->newSymbol(m_lexer.strToken().c_str())));
        m_lexer.nextToken();
    }
    else if (token == strconst) 
    {
        genInstruction(PushLiteral,
                genLiteral(MemoryManager::Instance()->newStString(m_lexer.strToken().c_str())));
        m_lexer.nextToken();
    }
    else if (token == arraybegin) 
    {
        genInstruction(PushLiteral, parseArray());
    }
    else if ((token == binary) && m_lexer.strToken().compare("(") == 0) 
    {
        m_lexer.nextToken();
        expression();
        if (parseok)
            if ((m_lexer.currentToken() != closing) || m_lexer.strToken().compare(")"))
                compilError(selector,"Missing Right Parenthesis","");
            else
                m_lexer.nextToken();
    }
    else if ((token == binary) && m_lexer.strToken().compare("<") == 0)
        parsePrimitive();
    else if ((token == binary) && m_lexer.strToken().compare("[") == 0)
        block();
    else
        compilError(selector,"invalid expression start", m_lexer.strToken().c_str());

    return(superTerm);
}

void Parser::parsePrimitive()
{   
    int primitiveNumber, argumentCount;

    if (m_lexer.nextToken() != intconst)
        compilError(selector,"primitive number missing","");
    primitiveNumber = m_lexer.intToken();
    m_lexer.nextToken();
    argumentCount = 0;
    while (parseok && ! ((m_lexer.currentToken() == binary) && m_lexer.strToken().compare(">") == 0)) 
    {
        term();
        argumentCount++;
    }
    genInstruction(DoPrimitive, argumentCount);
    genCode(primitiveNumber);
    m_lexer.nextToken();
}

void Parser::genMessage(bool toSuper, int argumentCount, object messagesym)
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

bool Parser::unaryContinuation(bool superReceiver)
{   
    int i;
    bool sent;

    while (parseok && (m_lexer.currentToken() == nameconst)) 
    {
        /* first check to see if it could be a temp by mistake */
        for (i=1; i < temporaryTop; i++)
        {
            if (m_lexer.strToken().compare(temporaryName[i]) == 0)
                compilWarn(selector,"message same as temporary:",
                        m_lexer.strToken().c_str());
        }
        for (i=1; i < argumentTop; i++)
        {
            if (m_lexer.strToken().compare(argumentName[i]) == 0)
                compilWarn(selector,"message same as argument:",
                        m_lexer.strToken().c_str());
        }
        /* the next generates too many spurious messages */
        /* for (i=1; i < instanceTop; i++)
           if (streq(m_lexer.tokenString, instanceName[i]))
           compilWarn(selector,"message same as instance",
           m_lexer.tokenString); */

        sent = false;

        if (! sent) 
        {
            genMessage(superReceiver, 0, MemoryManager::Instance()->newSymbol(m_lexer.strToken().c_str()));
        }
        /* once a message is sent to super, reciever is not super */
        superReceiver = false;
        m_lexer.nextToken();
    }
    return(superReceiver);
}

bool Parser::binaryContinuation(bool superReceiver)
{   
    bool superTerm;
    object messagesym;
    SObjectHandle* lock_messageSym = 0;

    superReceiver = unaryContinuation(superReceiver);
    while (parseok && (m_lexer.currentToken() == binary)) 
    {
        messagesym = MemoryManager::Instance()->newSymbol(m_lexer.strToken().c_str());
        lock_messageSym = new_SObjectHandle_from_object(messagesym);
        m_lexer.nextToken();
        superTerm = term();
        unaryContinuation(superTerm);
        genMessage(superReceiver, 1, messagesym);
        superReceiver = false;
        free_SObjectHandle(lock_messageSym);
    }
    return(superReceiver);
}

int Parser::optimizeBlock(int instruction, bool dopop)
{   
    int location;
    enum blockstatus savebstat;

    savebstat = blockstat;
    genInstruction(DoSpecial, instruction);
    location = codeTop;
    genCode(0);
    if (dopop)
        genInstruction(DoSpecial, PopTop);
    m_lexer.nextToken();
    if (m_lexer.strToken().compare("[") == 0) 
    {
        m_lexer.nextToken();
        if (blockstat == NotInBlock)
            blockstat = OptimizedBlock;
        body();
        if (m_lexer.strToken().compare("]"))
            compilError(selector,"missing close","after block");
        m_lexer.nextToken();
    }
    else 
    {
        binaryContinuation(term());
        genMessage(false, 0, MemoryManager::Instance()->newSymbol("value"));
    }
    codeArray[location] = codeTop+1;
    blockstat = savebstat;
    return(location);
}

bool Parser::keyContinuation(bool superReceiver)
{   
    int i, j, argumentCount;
    bool sent, superTerm;
    object messagesym;
    char pattern[80];

    superReceiver = binaryContinuation(superReceiver);
    if (m_lexer.currentToken() == namecolon) 
    {
        if (m_lexer.strToken().compare("ifTrue:") == 0) 
        {
            i = optimizeBlock(BranchIfFalse, false);
            if (m_lexer.strToken().compare("ifFalse:") == 0) 
            {
                codeArray[i] = codeTop + 3;
                optimizeBlock(Branch, true);
            }
        }
        else if (m_lexer.strToken().compare("ifFalse:") == 0) 
        {
            i = optimizeBlock(BranchIfTrue, false);
            if (m_lexer.strToken().compare("ifTrue:") == 0) 
            {
                codeArray[i] = codeTop + 3;
                optimizeBlock(Branch, true);
            }
        }
        else if (m_lexer.strToken().compare("whileTrue:") == 0) 
        {
            j = codeTop;
            genInstruction(DoSpecial, Duplicate);
            genMessage(false, 0, MemoryManager::Instance()->newSymbol("value"));
            i = optimizeBlock(BranchIfFalse, false);
            genInstruction(DoSpecial, PopTop);
            genInstruction(DoSpecial, Branch);
            genCode(j+1);
            codeArray[i] = codeTop+1;
            genInstruction(DoSpecial, PopTop);
        }
        else if (m_lexer.strToken().compare("and:") == 0)
            optimizeBlock(AndBranch, false);
        else if (m_lexer.strToken().compare("or:") == 0)
            optimizeBlock(OrBranch, false);
        else 
        {
            pattern[0] = '\0';
            argumentCount = 0;
            while (parseok && (m_lexer.currentToken() == namecolon)) 
            {
                strcat(pattern, m_lexer.strToken().c_str());
                argumentCount++;
                m_lexer.nextToken();
                superTerm = term();
                binaryContinuation(superTerm);
            }
            sent = false;

            /* check for predefined messages */
            messagesym = MemoryManager::Instance()->newSymbol(pattern);

            if (! sent) 
            {
                genMessage(superReceiver, argumentCount, messagesym);
            }
        }
        superReceiver = false;
    }
    return(superReceiver);
}

void Parser::continuation(bool superReceiver)
{
    superReceiver = keyContinuation(superReceiver);

    while (parseok && (m_lexer.currentToken() == closing) && m_lexer.strToken().compare(";") == 0) 
    {
        genInstruction(DoSpecial, Duplicate);
        m_lexer.nextToken();
        keyContinuation(superReceiver);
        genInstruction(DoSpecial, PopTop);
    }
}

void Parser::expression()
{   
    bool superTerm;
    char assignname[60];

    if (m_lexer.currentToken() == nameconst) 
    {   /* possible assignment */
        strcpy(assignname, m_lexer.strToken().c_str());
        m_lexer.nextToken();
        if ((m_lexer.currentToken() == binary) && m_lexer.strToken().compare("<-") == 0) 
        {
            m_lexer.nextToken();
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
        if (parseok)
            continuation(superTerm);
    }
}

void Parser::assignment(char* name)
{   
    int i;
    bool done;

    done = false;

    /* it might be a temporary */
    for (i = temporaryTop; (! done) && (i > 0); i--)
    {
        if (streq(name, temporaryName[i])) 
        {
            expression();
            genInstruction(AssignTemporary, i-1);
            done = true;
        }
    }

    /* or it might be an instance variable */
    for (i = 1; (! done) && (i <= instanceTop); i++)
    {
        if (streq(name, instanceName[i])) 
        {
            expression();
            genInstruction(AssignInstance, i-1);
            done = true;
        }
    }

    if (! done) 
    {   /* not known, handle at run time */
        genInstruction(PushArgument, 0);
        genInstruction(PushLiteral, genLiteral(MemoryManager::Instance()->newSymbol(name)));
        expression();
        genMessage(false, 2, MemoryManager::Instance()->newSymbol("assign:value:"));
    }
}

void Parser::statement()
{

    if ((m_lexer.currentToken() == binary) && m_lexer.strToken().compare("^") == 0) 
    {
        m_lexer.nextToken();
        expression();
        if (blockstat == InBlock) 
        {
            /* change return point before returning */
            genInstruction(PushConstant, contextConst);
            genMessage(false, 0, MemoryManager::Instance()->newSymbol("blockReturn"));
            genInstruction(DoSpecial, PopTop);
        }
        genInstruction(DoSpecial, StackReturn);
    }
    else 
    {
        expression();
    }
}

void Parser::body()
{
    /* empty blocks are same as nil */
    if ((blockstat == InBlock) || (blockstat == OptimizedBlock))
    {
        if ((m_lexer.currentToken() == closing) && m_lexer.strToken().compare("]") == 0) 
        {
            genInstruction(PushConstant, nilConst);
            return;
        }
    }

    while(parseok) 
    {
        statement();
        if (m_lexer.currentToken() == closing)
        {
            if (m_lexer.strToken().compare(".") == 0) 
            {
                if (m_lexer.nextToken() == inputend)
                    break;
                else  /* pop result, go to next statement */
                    genInstruction(DoSpecial, PopTop);
            }
            else
                break;  /* leaving result on stack */
        }
        else
        {
            if (m_lexer.currentToken() == inputend)
                break;  /* leaving result on stack */
            else 
            {
                compilError(selector,"invalid statement ending; token is ",
                        m_lexer.strToken().c_str());
            }
        }
    }
}

void Parser::block()
{   
    int saveTemporary, argumentCount, fixLocation;
    object tempsym; 
    object newBlk;
    SObjectHandle* lock_newBlk = 0;
    enum blockstatus savebstat;

    saveTemporary = temporaryTop;
    savebstat = blockstat;
    argumentCount = 0;
    if ((m_lexer.nextToken() == binary) && m_lexer.strToken().compare(":") == 0) 
    {
        while (parseok && (m_lexer.currentToken() == binary) && m_lexer.strToken().compare(":") == 0) 
        {
            if (m_lexer.nextToken() != nameconst)
                compilError(selector,"name must follow colon",
                        "in block argument list");
            if (++temporaryTop > maxTemporary)
                maxTemporary = temporaryTop;
            argumentCount++;
            if (temporaryTop > temporaryLimit)
                compilError(selector,"too many temporaries in method","");
            else 
            {
                tempsym = MemoryManager::Instance()->newSymbol(m_lexer.strToken().c_str());
                temporaryName[temporaryTop] = tempsym->charPtr();
            }
            m_lexer.nextToken();
        }
        if ((m_lexer.currentToken() != binary) || m_lexer.strToken().compare("|"))
            compilError(selector,"block argument list must be terminated",
                    "by |");
        m_lexer.nextToken();
    }
    newBlk = MemoryManager::Instance()->newBlock();
    lock_newBlk = new_SObjectHandle_from_object(newBlk);
    newBlk->basicAtPut(argumentCountInBlock, MemoryManager::Instance()->newInteger(argumentCount));
    newBlk->basicAtPut(argumentLocationInBlock, 
            MemoryManager::Instance()->newInteger(saveTemporary + 1));
    genInstruction(PushLiteral, genLiteral(newBlk));
    genInstruction(PushConstant, contextConst);
    genInstruction(DoPrimitive, 2);
    genCode(29);
    genInstruction(DoSpecial, Branch);
    fixLocation = codeTop;
    genCode(0);
    /*genInstruction(DoSpecial, PopTop);*/
    newBlk->basicAtPut(bytecountPositionInBlock, MemoryManager::Instance()->newInteger(codeTop+1));
    blockstat = InBlock;
    body();
    if ((m_lexer.currentToken() == closing) && m_lexer.strToken().compare("]") == 0)
        m_lexer.nextToken();
    else
        compilError(selector,"block not terminated by ]","");
    genInstruction(DoSpecial, StackReturn);
    codeArray[fixLocation] = codeTop+1;
    temporaryTop = saveTemporary;
    blockstat = savebstat;

    free_SObjectHandle(lock_newBlk);
}

void Parser::temporaries()
{   
    object tempsym;

    temporaryTop = 0;
    if ((m_lexer.currentToken() == binary) && m_lexer.strToken().compare("|") == 0) 
    {
        m_lexer.nextToken();
        while (m_lexer.currentToken() == nameconst) 
        {
            if (++temporaryTop > maxTemporary)
                maxTemporary = temporaryTop;
            if (temporaryTop > temporaryLimit)
                compilError(selector,"too many temporaries in method","");
            else 
            {
                tempsym = MemoryManager::Instance()->newSymbol(m_lexer.strToken().c_str());
                temporaryName[temporaryTop] = tempsym->charPtr();
            }
            m_lexer.nextToken();
        }
        if ((m_lexer.currentToken() != binary) || m_lexer.strToken().compare("|"))
            compilError(selector,"temporary list not terminated by bar","");
        else
            m_lexer.nextToken();
    }
}

void Parser::messagePattern()
{   
    object argsym;

    argumentTop = 0;
    strcpy(selector, m_lexer.strToken().c_str());
    if (m_lexer.currentToken() == nameconst)     /* unary message pattern */
        m_lexer.nextToken();
    else if (m_lexer.currentToken() == binary) 
    { /* binary message pattern */
        m_lexer.nextToken();
        if (m_lexer.currentToken() != nameconst) 
            compilError(selector,"binary message pattern not followed by name",selector);
        argsym = MemoryManager::Instance()->newSymbol(m_lexer.strToken().c_str());
        argumentName[++argumentTop] = argsym->charPtr();
        m_lexer.nextToken();
    }
    else if (m_lexer.currentToken() == namecolon) 
    {  /* keyword message pattern */
        selector[0] = '\0';
        while (parseok && (m_lexer.currentToken() == namecolon)) 
        {
            strcat(selector, m_lexer.strToken().c_str());
            m_lexer.nextToken();
            if (m_lexer.currentToken() != nameconst)
                compilError(selector,"keyword message pattern",
                        "not followed by a name");
            if (++argumentTop > argumentLimit)
                compilError(selector,"too many arguments in method","");
            argsym = MemoryManager::Instance()->newSymbol(m_lexer.strToken().c_str());
            argumentName[argumentTop] = argsym->charPtr();
            m_lexer.nextToken();
        }
    }
    else
        compilError(selector,"illegal message selector", m_lexer.strToken().c_str());
}

bool Parser::parseCode(object method, bool savetext)
{   
    literalArray.clear();
    parseok = true;

    blockstat = NotInBlock;
    codeTop = 0;
    temporaryTop = argumentTop =0;
    maxTemporary = 0;

    temporaries();
    if (parseok)
        body();
    return(recordMethodBytecode(method, savetext));
}

bool Parser::parseMessageHandler(object method, bool savetext)
{   
    literalArray.clear();
    parseok = true;

    blockstat = NotInBlock;
    codeTop = 0;
    temporaryTop = argumentTop =0;
    maxTemporary = 0;

    messagePattern();
    if (parseok)
        temporaries();
    if (parseok)
        body();
    return(recordMethodBytecode(method, savetext));
}


bool Parser::recordMethodBytecode(object method, bool savetext)
{
    int i;
    object bytecodes, theLiterals;
    byte *bp;

    if (parseok) 
    {
        genInstruction(DoSpecial, PopTop);
        genInstruction(DoSpecial, SelfReturn);
    }

    if (! parseok) 
    {
        method->basicAtPut(bytecodesInMethod, nilobj);
    }
    else 
    {
        bytecodes = MemoryManager::Instance()->newByteArray(codeTop);
        bp = bytecodes->bytePtr();
        for (i = 0; i < codeTop; i++) 
        {
            bp[i] = codeArray[i];
        }
        method->basicAtPut(messageInMethod, MemoryManager::Instance()->newSymbol(selector));
        method->basicAtPut(bytecodesInMethod, bytecodes);
        if (!literalArray.empty()) 
        {
            theLiterals = MemoryManager::Instance()->newArray(literalArray.size());
            for (i = 1; i <= literalArray.size(); i++) 
            {
                theLiterals->basicAtPut(i, literalArray[i-1]);
            }
            method->basicAtPut(literalsInMethod, theLiterals);
        }
        else 
        {
            method->basicAtPut(literalsInMethod, nilobj);
        }
        method->basicAtPut(stackSizeInMethod, MemoryManager::Instance()->newInteger(6));
        method->basicAtPut(temporarySizeInMethod,
                MemoryManager::Instance()->newInteger(1 + maxTemporary));
        if (savetext) 
        {
            method->basicAtPut(textInMethod, MemoryManager::Instance()->newStString(m_lexer.source()));
        }
        return(true);
    }
    return(false);
}
