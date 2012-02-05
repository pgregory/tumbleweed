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

struct objectStruct {
    object _class;
    short referenceCount;
    short size;
    object *memory;
    };
# define ObjectTableMax 32500

extern struct objectStruct *objectTable;

/*
    The most basic routines to the memory manager are incr and decr,
  which increment and decrement reference counts in objects.  By separating
  decrement from memory freeing, we could replace these as procedure calls
  by using the following macros (thereby saving procedure calls):
*/

/*
  notice that the argument x is first assigned to a global variable; this is
  in case evaluation of x results in side effects (such as assignment) which
  should not be repeated. 
*/

/*
    The next most basic routines in the memory module are those that
  allocate blocks of storage.  There are three routines:
    allocObject(size) - allocate an array of objects
    allocByte(size) - allocate an array of bytes
    allocStr(str) - allocate a string and fill it in
  again, these may be macros, or they may be actual procedure calls
*/

extern object allocObject( INT );
extern object allocByte( INT );
extern object allocStr( CSTR );

/*
    integer objects are (but need not be) treated specially.
  In this memory manager, negative integers are just left as is, but
  positive integers are changed to x*2+1.  Either a negative or an odd
  number is therefore an integer, while a nonzero even number is an
  object pointer (multiplied by two).  Zero is reserved for the object ``nil''
  Since newInteger does not fill in the _class field, it can be given here.
  If it was required to use the _class field, it would have to be deferred
  until names.h
*/

/*
    there are four routines used to access fields within an object.
  Again, some of these could be replaced by macros, for efficiency
    basicAt(x, i) - ith field (start at 1) of object x
    basicAtPut(x, i, v) - put value v in object x
    byteAt(x, i) - ith field (start at 0) of object x
    byteAtPut(x, i, v) - put value v in object x*/

extern object newCPointer(void* l);
extern void* cPointerValue(object);
/*
    Finally, a few routines (or macros) are used to access or set
  _class fields and size fields of objects
*/

# define classField(x) objectTable[x>>1]._class
# define setClass(x,y) incr(classField(x)=y)
# define sizeField(x) objectTable[x>>1].size

# define sysMemPtr(x) objectTable[x>>1].memory
extern object sysobj;
# define memoryPtr(x) sysMemPtr(x)
# define bytePtr(x) ((byte *) memoryPtr(x))
# define charPtr(x) ((char *) memoryPtr(x))

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
# ifndef mBlockAlloc
extern object *mBlockAlloc(INT);
# endif

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
extern void sysDecr(object, int);
extern boolean debugging;
void setInstanceVariables(object aClass);
boolean parse(object method, const char* text, boolean savetext);
void givepause();
int objectCount();
void setFreeLists();
int garbageCollect(int verbose);


# ifndef basicAtPut
extern void basicAtPut(object, int, object);
# endif
# ifndef simpleAtPut
extern void simpleAtPut(object, int, object);
# endif
# ifndef incr
extern void incr(object);
# endif
# ifndef decr
extern void decr(object);
# endif
# ifdef fieldAtPut
extern int f_i;
#else
extern void fieldAtPut(object, int, object);
# endif
# ifndef byteAt
extern int byteAt(object, int);
# endif
# ifndef byteAtPut
extern void byteAtPut(object, int, int);
# endif

# ifndef basicAt
extern object basicAt(object o, int i);
# endif

