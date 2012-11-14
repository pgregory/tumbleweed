/*
    Little Smalltalk, version 2
    Written by Tim Budd, Oregon State University, July 1987
*/
/*
    values returned by the lexical analyzer
*/

#ifndef LEX_H_INCLUDED
#define LEX_H_INCLUDED

#include <string>
#include <deque>

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


class Lexer
{
    public:
        //! Default constructor
        /*! The lexer is in a default state after this, and will produce no tokens */ 
        Lexer();
        //! Constructor from a source string
        /*! The lexer will be reset to the start of the provided source string.
         *  The first token will be read, ready for processing.
         */
        Lexer(const char* source);
        //! Copy constructor
        /*! The lexer will clone the entire state of the provided lexer.
         *  It will point to the same source, and at the same point in the source as the 
         *  previous lexer, with the same pushback and current token.
         *  The first call to nextToken on the new lexer will result in the same token
         *  as calling nextToken on the donor lexer.
         */
        Lexer(const Lexer& from);
        ~Lexer() {}

        void reset(const char* source);

        tokentype nextToken();
        char peek();
        tokentype currentToken() const;
        const std::string& strToken() const;
        int intToken() const;
        double floatToken() const;
        const char* source() const;


    private:
        void pushBack(char c);
        char nextChar();
        bool isClosing(char c);
        bool isSymbolChar(char c);
        bool singleBinary(char c);
        bool binarySecond(char c);

        tokentype   m_currentToken;
        int m_tokenInteger;
        double m_tokenFloat;
        std::string m_tokenString;
        const char* m_source;
        const char *m_cp;        /* character pointer */
        char m_cc;         /* current character */
        std::deque<char> m_pushBuffer; /* pushed back buffer */
};


// Implementation.

inline tokentype Lexer::currentToken() const
{
    return m_currentToken;
}

inline const std::string& Lexer::strToken() const
{
    return m_tokenString;
}

inline int Lexer::intToken() const
{
    return m_tokenInteger;
}

inline double Lexer::floatToken() const
{
    return m_tokenFloat;
}

inline const char* Lexer::source() const
{
    return m_source;
}
#endif
