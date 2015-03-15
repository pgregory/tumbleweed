#include "memory.h"
#include "names.h"
#include "interp.h"
#include "lex.h"


/* all of the following limits could be increased (up to
   256) without any trouble.  They are kept low 
   to keep memory utilization down */

# define codeLimit 16384     /* maximum number of bytecodes permitted */
# define literalLimit 128   /* maximum number of literals permitted */
# define temporaryLimit 32  /* maximum number of temporaries permitted */
# define argumentLimit 32   /* maximum number of arguments permitted */
# define instanceLimit 32   /* maximum number of instance vars permitted */
# define methodLimit 64     /* maximum number of methods permitted */

/*! \brief The parser
 *
 * This class provides source code parsing functionality.
 * It is combined with the lexer to process Tumbleweed source
 * and produce VM bytecodes.
 */
class Parser
{
    public:
        Parser() {}
        Parser(const Lexer& lex) : m_lexer(lex)
        {}
        ~Parser() {}

        bool parseMessageHandler(object method, bool savetext);
        bool parseCode(object method, bool savetext);

        void setInstanceVariables(object aClass);
        void setLexer(const Lexer& lex)
        {
            m_lexer = lex;
        }

    private:
        enum blockstatus 
        {
            NotInBlock, 
            InBlock, 
            OptimizedBlock
        } blockstat;

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

        bool parseok;            /* parse still ok? */
        int codeTop;            /* top position filled in code array */
        byte codeArray[codeLimit];  /* bytecode array */
        std::vector<ObjectHandle> literalArray;
        int temporaryTop;
        char *temporaryName[temporaryLimit];
        int argumentTop;
        char *argumentName[argumentLimit];
        int instanceTop;
        char *instanceName[instanceLimit];

        int maxTemporary;      /* highest temporary see so far */
        char selector[80];     /* message selector */

        Lexer  m_lexer;
};
