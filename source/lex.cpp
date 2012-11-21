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

static tokentype   _currentToken;
static int _tokenInteger;
static double _tokenFloat;
static char _tokenString[80];
static const char* _source;
static const char *_cp;
static char _cc;
static int _pbTop;
static char _pushBuffer[20];


void resetLexer(const char* src)
{
    _pbTop = 0;
    _source = src;
    _cp = _source;
    /* get first token */
    nextToken();
}

/* pushBack - push one character back into the input */
static void pushBack(char c)
{
    _pushBuffer[_pbTop++] = c;
}

/* nextChar - retrieve the next char, from buffer or input */
static char nextChar()
{
    if (_pbTop > 0) 
        _cc = _pushBuffer[--_pbTop];
    else if (*_cp) 
        _cc = *_cp++;
    else 
        _cc = '\0';
    return(_cc);
}

/* peek - take a peek at the next character */
char peek()
{
    pushBack(nextChar());
    return (_cc);
}

/* isClosing - characters which can close an expression */
static bool isClosing(char c)
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
static bool isSymbolChar(char c)
{
    if (isdigit(c) || isalpha(c)) 
        return(true);
    if (isspace(c) || isClosing(c)) 
        return(false);
    return(true);
}

/* singleBinary - binary characters that cannot be continued */
static bool singleBinary(char c)
{
    switch(c) 
    {
        case '[': 
        case '(': 
        case ')': 
        case ']':
            return(true);
    }
    return(false);
}

/* binarySecond - return true if char can be second char in binary symbol */
static bool binarySecond(char c)
{
    if (isalpha(c) || isdigit(c) || isspace(c) || isClosing(c) || singleBinary(c))
        return(false);
    return(true);
}

tokentype nextToken()
{   
    bool sign;
    char* tp;

    /* skip over blanks and comments */
    while(nextChar() && (isspace(_cc) || (_cc == '"')))
    {
        if (_cc == '"') {
            /* read comment */
            while (nextChar() && (_cc != '"')) ;
            if (! _cc) break;    /* break if we run into eof */
        }
    }

    tp = _tokenString;
    *tp++ = _cc;

    if (! _cc)           /* end of input */
        _currentToken = inputend;
    else if (isalpha(_cc)) 
    {     /* identifier */
        while (nextChar() && isalnum(_cc))
            *tp++ = _cc;
        if (_cc == ':') 
        {
            *tp++ = _cc;
            _currentToken = namecolon;
        }
        else 
        {
            pushBack(_cc);
            _currentToken = nameconst;
        }
    }
    else if (isdigit(_cc)) 
    {     /* number */
        long longresult = _cc - '0';
        while (nextChar() && isdigit(_cc)) 
        {
            *tp++ = _cc;
            longresult = (longresult * 10) + (_cc - '0');
        }
        if (longCanBeInt(longresult)) 
        {
            _tokenInteger = longresult;
            _currentToken = intconst;
        }
        else 
        {
            _currentToken = floatconst;
            _tokenFloat = (double) longresult;
        }
        if (_cc == '.') 
        {    /* possible float */
            if (nextChar() && isdigit(_cc)) 
            {
                *tp++ = '.';
                do
                    *tp++ = _cc;
                while (nextChar() && isdigit(_cc));
                if (_cc) 
                    pushBack(_cc);
                _currentToken = floatconst;
                *tp = '\0';
                _tokenFloat = atof(_tokenString);
            }
            else 
            {
                /* nope, just an ordinary period */
                if (_cc) pushBack(_cc);
                pushBack('.');
            }
        }
        else
            pushBack(_cc);

        if (nextChar() && _cc == 'e') 
        {  /* possible float */
            if (nextChar() && _cc == '-') 
            {
                sign = true;
                nextChar();
            }
            else
                sign = false;
            if (_cc && isdigit(_cc)) 
            { /* yep, its a float */
                *tp++ = 'e';
                if (sign) 
                    *tp++ = '-';
                while (_cc && isdigit(_cc)) 
                {
                    *tp++ = _cc;
                    nextChar();
                }
                if (_cc) 
                    pushBack(_cc);
                _currentToken = floatconst;
                _tokenFloat = atof(_tokenString);
            }
            else 
            {  /* nope, wrong again */
                if (_cc) pushBack(_cc);
                if (sign) pushBack('-');
                pushBack('e');
            }
        }
        else
            if (_cc) pushBack(_cc);
    }
    else if (_cc == '$') 
    {       /* character constant */
        _tokenInteger = (int) nextChar();
        _currentToken = charconst;
    }
    else if (_cc == '#') 
    {       /* symbol */
        tp--;
        if (nextChar() == '(')
            _currentToken = arraybegin;
        else 
        {
            pushBack(_cc);
            while (nextChar() && isSymbolChar(_cc))
                *tp++ = _cc;
            pushBack(_cc);
            _currentToken = symconst;
        }
    }
    else if (_cc == '\'') 
    {      /* string constant */
        tp--;
strloop:
        while (nextChar() && (_cc != '\''))
            *tp++ = _cc;
        /* check for nested quote marks */
        if (_cc && nextChar() && (_cc == '\'')) 
        {
            *tp++ = _cc;
            goto strloop;
        }
        pushBack(_cc);
        _currentToken = strconst;
    }
    else if (isClosing(_cc))     /* closing expressions */
        _currentToken = closing;
    else if (singleBinary(_cc)) 
    {    /* single binary expressions */
        _currentToken = binary;
    }
    else 
    {              /* anything else is binary */
        if (nextChar() && binarySecond(_cc))
            *tp++ = _cc;
        else
            pushBack(_cc);
        _currentToken = binary;
    }

    *tp++ = '\0';
    return(_currentToken);
}


tokentype currentToken()
{
    return _currentToken;
}

const char* strToken()
{
    return _tokenString;
}

int intToken()
{
    return _tokenInteger;
}

double floatToken()
{
    return _tokenFloat;
}

const char* source()
{
    return _source;
}
