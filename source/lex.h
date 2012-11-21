/*
    Little Smalltalk, version 2
    Written by Tim Budd, Oregon State University, July 1987
*/
/*
    values returned by the lexical analyzer
*/

#ifndef LEX_H_INCLUDED
#define LEX_H_INCLUDED

typedef enum tokensyms 
{ 
    nothing, 
    nameconst, 
    namecolon, 
    intconst, 
    floatconst, 
    charconst, 
    symconst,
    arraybegin, 
    strconst, 
    binary, 
    closing, 
    inputend
} tokentype;


void resetLexer(const char* source);

tokentype nextToken();
char peek();

tokentype currentToken();
const char* strToken();
int intToken();
double floatToken();
const char* source();

#endif
