/*
   Little Smalltalk, version 2
   Written by Tim Budd, Oregon State University, July 1987

   lexical analysis routines for method parser
   should be called only by parser 
   */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
# include "env.h"
# include "memory.h"
# include "lex.h"


Lexer::Lexer() : 
    m_currentToken(nothing),
    m_tokenInteger(0),
    m_tokenFloat(0.0),
    m_tokenString(""),
    m_source(NULL),
    m_cp(NULL),
    m_cc('\0')
{}

Lexer::Lexer(const char* source) :
    m_currentToken(nothing),
    m_tokenInteger(0),
    m_tokenFloat(0.0),
    m_tokenString(""),
    m_source(NULL),
    m_cp(NULL),
    m_cc('\0')
{
    reset(source);
}


Lexer::Lexer(const Lexer& from)
{
    // Duplicate entire state, this new lexer will continue
    // from the same place as the donor.
    m_source = from.m_source;
    m_cp = from.m_cp;

    m_pushBuffer.assign(from.m_pushBuffer.begin(), from.m_pushBuffer.end());
    m_cc = from.m_cc;
    m_currentToken = from.m_currentToken;
    m_tokenString = from.m_tokenString;
    m_tokenInteger = from.m_tokenInteger;
    m_tokenFloat = from.m_tokenFloat;
}


void Lexer::reset(const char* src)
{
    m_pushBuffer.clear();
    m_source = src;
    m_cp = m_source;
    /* get first token */
    nextToken();
}

/* pushBack - push one character back into the input */
void Lexer::pushBack(char c)
{
    m_pushBuffer.push_back(c);
}

/* nextChar - retrieve the next char, from buffer or input */
char Lexer::nextChar()
{
    if (!m_pushBuffer.empty()) 
    {
        m_cc = m_pushBuffer.back();
        m_pushBuffer.pop_back();
    }
    else if (*m_cp) 
        m_cc = *m_cp++;
    else 
        m_cc = '\0';
    return(m_cc);
}

/* peek - take a peek at the next character */
char Lexer::peek()
{
    pushBack(nextChar());
    return (m_cc);
}

/* isClosing - characters which can close an expression */
bool Lexer::isClosing(char c)
{
    switch(c) 
    {
        case '.': 
        case ']': 
        case ')': 
        case ';':
            case '\"': 
        case '\'':
                return(true);
    }
    return(false);
}

/* isSymbolChar - characters which can be part of symbols */
bool Lexer::isSymbolChar(char c)
{
    if (isdigit(c) || isalpha(c)) 
        return(true);
    if (isspace(c) || isClosing(c)) 
        return(false);
    return(true);
}

/* singleBinary - binary characters that cannot be continued */
bool Lexer::singleBinary(char c)
{
    switch(c) {
        case '[': case '(': case ')': case ']':
            return(true);
    }
    return(false);
}

/* binarySecond - return true if char can be second char in binary symbol */
bool Lexer::binarySecond(char c)
{
    if (isalpha(c) || isdigit(c) || isspace(c) || isClosing(c) || singleBinary(c))
        return(false);
    return(true);
}

tokentype Lexer::nextToken()
{   
    bool sign;
    std::string strToken;

    /* skip over blanks and comments */
    while(nextChar() && (isspace(m_cc) || (m_cc == '"')))
    {
        if (m_cc == '"') {
            /* read comment */
            while (nextChar() && (m_cc != '"')) ;
            if (! m_cc) break;    /* break if we run into eof */
        }
    }

    strToken.clear();
    strToken.push_back(m_cc);

    if (! m_cc)           /* end of input */
        m_currentToken = inputend;
    else if (isalpha(m_cc)) 
    {     /* identifier */
        while (nextChar() && isalnum(m_cc))
            strToken.push_back(m_cc);
        if (m_cc == ':') 
        {
            strToken.push_back(m_cc);
            m_currentToken = namecolon;
        }
        else 
        {
            pushBack(m_cc);
            m_currentToken = nameconst;
        }
    }
    else if (isdigit(m_cc)) 
    {     /* number */
        long longresult = m_cc - '0';
        while (nextChar() && isdigit(m_cc)) 
        {
            strToken.push_back(m_cc);
            longresult = (longresult * 10) + (m_cc - '0');
        }
        if (longCanBeInt(longresult)) 
        {
            m_tokenInteger = longresult;
            m_currentToken = intconst;
        }
        else 
        {
            m_currentToken = floatconst;
            m_tokenFloat = (double) longresult;
        }
        if (m_cc == '.') 
        {    /* possible float */
            if (nextChar() && isdigit(m_cc)) 
            {
                strToken.push_back('.');
                do
                    strToken.push_back(m_cc);
                while (nextChar() && isdigit(m_cc));
                if (m_cc) pushBack(m_cc);
                m_currentToken = floatconst;
                strToken.push_back('\0');
                m_tokenFloat = atof(m_tokenString.c_str());
            }
            else 
            {
                /* nope, just an ordinary period */
                if (m_cc) pushBack(m_cc);
                pushBack('.');
            }
        }
        else
            pushBack(m_cc);

        if (nextChar() && m_cc == 'e') 
        {  /* possible float */
            if (nextChar() && m_cc == '-') 
            {
                sign = true;
                nextChar();
            }
            else
                sign = false;
            if (m_cc && isdigit(m_cc)) 
            { /* yep, its a float */
                strToken.push_back('e');
                if (sign) 
                    strToken.push_back('-');
                while (m_cc && isdigit(m_cc)) 
                {
                    strToken.push_back(m_cc);
                    nextChar();
                }
                if (m_cc) 
                    pushBack(m_cc);
                m_currentToken = floatconst;
                m_tokenFloat = atof(m_tokenString.c_str());
            }
            else 
            {  /* nope, wrong again */
                if (m_cc) pushBack(m_cc);
                if (sign) pushBack('-');
                pushBack('e');
            }
        }
        else
            if (m_cc) pushBack(m_cc);
    }
    else if (m_cc == '$') 
    {       /* character constant */
        m_tokenInteger = (int) nextChar();
        m_currentToken = charconst;
    }
    else if (m_cc == '#') 
    {       /* symbol */
        strToken.resize(strToken.size()-1); // erase pound sign
        if (nextChar() == '(')
            m_currentToken = arraybegin;
        else 
        {
            pushBack(m_cc);
            while (nextChar() && isSymbolChar(m_cc))
                strToken.push_back(m_cc);
            pushBack(m_cc);
            m_currentToken = symconst;
        }
    }
    else if (m_cc == '\'') 
    {      /* string constant */
        strToken.resize(strToken.size()-1);
strloop:
        while (nextChar() && (m_cc != '\''))
            strToken.push_back(m_cc);
        /* check for nested quote marks */
        if (m_cc && nextChar() && (m_cc == '\'')) 
        {
            strToken.push_back(m_cc);
            goto strloop;
        }
        pushBack(m_cc);
        m_currentToken = strconst;
    }
    else if (isClosing(m_cc))     /* closing expressions */
        m_currentToken = closing;
    else if (singleBinary(m_cc)) 
    {    /* single binary expressions */
        m_currentToken = binary;
    }
    else 
    {              /* anything else is binary */
        if (nextChar() && binarySecond(m_cc))
            strToken.push_back(m_cc);
        else
            pushBack(m_cc);
        m_currentToken = binary;
    }

    m_tokenString = strToken;
    return(m_currentToken);
}
