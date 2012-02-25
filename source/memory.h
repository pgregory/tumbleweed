/*
    Little Smalltalk, version 2
    Written by Tim Budd, Oregon State University, July 1987
*/

/*
    The first major decision to be made in the memory manager is what
an entity of type object really is.  Two obvious choices are a pointer (to 
the actual object memory) or an index into an object table.  We decided to
use the latter, although either would work.
    Similarly, one can either define the token object using a typedef,
or using a define statement.  Either one will work (check this?)
*/

typedef long object;

/*
    The memory module itself is defined by over a dozen routines.
All of these could be defined by procedures, and indeed this was originally
done.  However, for efficiency reasons, many of these procedures can be
replaced by macros generating in-line code.  For the latter approach
to work, the structure of the object table must be known.  For this reason,
it is given here.  Note, however, that outside of the files memory.c and
unixio.c (or macio.c on the macintosh) ONLY the macros described in this
file make use of this structure: therefore modifications or even complete
replacement is possible as long as the interface remains consistent
*/

struct objectStruct 
{
    object _class;
    long referenceCount;
    long size;
    object *memory;

    object classField();
    void setClass(object y);
    long sizeField();
    void setSizeField(long size);
    object* sysMemPtr();
    object* memoryPtr();
    byte* bytePtr();
    char* charPtr();

    void basicAtPut(int, object);
    void simpleAtPut(int, object);
    void fieldAtPut(int, object);
    int byteAt(int);
    void byteAtPut(int, int);
    object basicAt(int i);

    double floatValue();
    int intValue();
    void* cPointerValue();

};


# define nilobj (object) 0

/*
    There is a large amount of differences in the qualities of malloc
    procedures in the Unix world.  Some perform very badly when asked
    to allocate thousands of very small memory blocks, while others
    take this without any difficulty.  The routine mBlockAlloc is used
    to allocate a small bit of memory; the version given below
    allocates a large block and then chops it up as needed; if desired,
    for versions of malloc that can handle small blocks with ease
    this can be replaced using the following macro: 

# define mBlockAlloc(size) (object *) calloc((unsigned) size, sizeof(object))

    This can, and should, be replaced by a better memory management
    algorithm.
*/
# define mBlockAlloc(size) (object *) calloc((unsigned) size, sizeof(object))

/*
    the dictionary symbols is the source of all symbols in the system
*/
extern object symbols;

/*
    finally some external declarations with prototypes
*/
extern void sysError(const char*, const char*);
extern void dspMethod(char*, char*);
extern void initMemoryManager();
extern void imageWrite(FILE*);
extern void imageRead(FILE*);
extern boolean debugging;
void setInstanceVariables(object aClass);
boolean parse(object method, const char* text, boolean savetext);
void givepause();


#include <map>
#include <vector>
#include <string>

typedef std::vector<objectStruct>     TObjectTable;
typedef TObjectTable::iterator  TObjectTableIterator;
typedef std::multimap<size_t, object>    TObjectFreeList;
typedef std::map<object, size_t>    TObjectFreeListInv;
typedef TObjectFreeList::iterator   TObjectFreeListIterator;
typedef TObjectFreeListInv::iterator   TObjectFreeListInvIterator;
typedef TObjectFreeList::reverse_iterator   TObjectFreeListRevIterator;

class MemoryManager
{
    public:
        MemoryManager();
        ~MemoryManager();

        static MemoryManager* Instance();

        void setFreeLists(); 
        int garbageCollect();
        void visit(register object x);
        void visitMethodCache();
        size_t objectCount();

        std::string statsString();

        objectStruct& objectFromID(object id);
        object copyFrom(object obj, int start, int size);

        object allocObject(size_t memorySize);
        object allocByte(size_t size);
        object allocStr(register const char* str);
        bool destroyObject(object z);

        object newArray(int);
        object newBlock();
        object newByteArray(int);
        object newClass(const char*);
        object newChar(int);
        object newContext(int, object, object, object);
        object newDictionary(int);
        object newInteger(int);
        object newFloat(double);
        object newMethod();
        object newLink(object, object);
        object newStString(const char*);
        object newSymbol(const char*);
        object newCPointer(void* l);

        void disableGC(bool disable);

        void imageRead(FILE* fp);
        void imageWrite(FILE* fp);

    private:
        TObjectFreeList objectFreeList;
        TObjectFreeListInv objectFreeListInv;
        TObjectTable    objectTable;
        bool            noGC;

        static MemoryManager* m_pInstance;

        size_t growObjectStore(size_t amount);
};

extern MemoryManager* theMemoryManager;

#define objectRef(x) (MemoryManager::Instance()->objectFromID(x))

