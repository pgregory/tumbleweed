/*
    Little Smalltalk, version 2
    Written by Tim Budd, Oregon State University, July 1987
*/

#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED

#include "env.h"
#include "stdint.h"

#if defined TW_UNIT_TESTS
#include "gtest/gtest_prod.h"
#endif

/*! \brief Object structure
 *
 * The default structure that represents all objects int he system.
 */
typedef struct _ObjectStruct 
{
    //! ID of the object that defines the class of this object.
    struct _ObjectStruct* _class;
    //! A reference counter, used only during mark/sweep GC.
    byte referenceCount;
    byte _padding;
    //! An identity hash key, used for hash tables and dictionaries.
    unsigned short identityHash;
    //! The size of the data area, in object ID's
    long size;
    //! A pointer to the data area of the object.
    struct _ObjectStruct** memory;
} ObjectStruct;

typedef ObjectStruct* object;

typedef struct _SObjectHandle
{
    object m_handle;
    struct _SObjectHandle* m_next;
    struct _SObjectHandle* m_prev;
} SObjectHandle;

extern SObjectHandle* head;
extern SObjectHandle* tail;

void appendToList(SObjectHandle* h);
void removeFromList(SObjectHandle* h);
SObjectHandle* new_SObjectHandle();
SObjectHandle* copy_SObjectHandle(const SObjectHandle* from);
SObjectHandle* new_SObjectHandle_from_object(object from);
void free_SObjectHandle(SObjectHandle* h);
int hash_SObjectHandle(SObjectHandle* h);

unsigned short hashObject(object o);



//! Data pointer getter
object* sysMemPtr(object _this);
//! Get the data area as a byte pointer
byte* bytePtr(object _this);
//! Get the data area as a char pointer
char* charPtr(object _this);
/*! Basic indexed object getter.
 *
 * Get an object ID from the given index in the data area.
 *
 * \param index The index in the data area to access.
 * \return The object ID at the given index.
 */
object basicAt(object _this, int index);
/*! Basic indexed object insertion.
 *
 * Assigns the given object to the index if it is range.
 *
 * \param index The index to assign the value to.
 * \param o The object to assign.
 */
void basicAtPut(object _this, int index, object o);
/*! Byte accessor 
 *
 * Get the byte at the given index in the data area.
 *
 * \param index The index of the byte to get, if in range.
 * \return The byte value at the index.
 */
int byteAt(object _this, int index);
/*! Byte setter
 *
 * Set a value in the data area addressed as byte.
 *
 * \param index The index in the data area to update.
 * \param value The value to put into the data area.
 */
void byteAtPut(object _this, int index, int value);

/*! Float getter
 *
 * Get the value stored in the data area as a float.
 *
 * \return The float value this object holds.
 */
double floatValue(object _this);
/*! Int getter
 *
 * Get the value stored in the data area as an integer.
 *
 * \return The int value this object holds.
 */
int intValue(object _this);
/*! CPointer getter
 *
 * Get the value stored in the data area as a native C pointer.
 *
 * \return The pointer value this object holds.
 */
void* cPointerValue(object _this);

extern ObjectStruct _nilobj;
extern object nilobj;

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
extern int debugging;
void givepause();


/*! Get the count of live, referenced objects.
 *
 * \return The number of active objects.
 */
size_t objectCount();

/*! Get the count of free object slots.
 *
 * \return The number of free slots.
 */
size_t freeSlotsCount();


size_t storageSize();

/*! Copy data from an object into a new array.
 *
 * Copy the object ID's from the given object's data area
 * starting at the given index, into a newly created array 
 * type object.
 *
 * \param obj The object to copy data from.
 * \param start The start index to copy from.
 * \param size The amount of data to copy into the array.
 * \return The new object containing the copied data.
 */
object copyFrom(object obj, int start, int size);

/*! Allocate a new object.
 *
 * Create a new object, and prepare data area of the given size.
 *
 * \param memorySize The size to allocate in the data area, a 
 *          count of object ids.
 * \return The new object.
 */
object allocObject(size_t memorySize);

/*! Allocate a new byte object.
 *
 * Create a new object, and prepare data area of the given size.
 *
 * \param size The size in bytes to allocate in data area. This will
 *          will be rounded to multiples of object id size.
 * \return The new object.
 */
object allocByte(size_t size);

/*! Allocate a new string object.
 *
 * Create a new object, and prepare data area to contain the given
 * string. Enough data area will be allocated, and the string copied.
 * Where necessary, rounded space will be filled with '\0'.
 *
 * \param str The string to copy.
 * \return The new object.
 */
object allocStr(register const char* str);

/*! Destroy an object.
 *
 * The given object is added to the free list in the 
 * appropriate place for it's size, so that it can
 * be reused.
 *
 * \param z The object ID to destroy.
 * \return True if the object is added to the free list.
 */
int destroyObject(object z);

/*! Create a new Array object.
 *
 * The newly created object is assigned the appropriate "Array" class
 * and allocated a data area big enough to hold size objects.
 *
 * \param size The desired size of the array in objects.
 * \return The ID of the new object.
 */
object newArray(int size);

/*! Create a new Block object.
 *
 * The newly created object is assigned the appropriate "Object" class.
 *
 * \return The ID of the new object.
 */
object newBlock();

/*! Create a new ByteArray object.
 *
 * The newly created object is assigned the appropriate "ByteArray" class
 * and allocated a data area big enough to hold size bytes.
 *
 * \param size The desired size of the array in bytes.
 * \return The ID of the new object.
 */
object newByteArray(int size);

/*! Create a new Class object.
 *
 * The newly created object is assigned the appropriate "Class" class.
 *
 * \param name The name of the new class.
 * \return The ID of the new object.
 */
object newClass(const char* name);

/*! Create a new Char object.
 *
 * The newly created object is assigned the appropriate "Char" class.
 *
 * \param value The char value to assign to the object.
 * \return The ID of the new object.
 */
object newChar(int value);

/*! Create a new Context object.
 *
 * The newly created object is assigned the appropriate "Context" class.
 *
 * \param link The link offset within the context, where the execution begins from.
 * \param method The object containing the bytecodes for the method.
 * \param args An Array object containing the arguments.
 * \param temp An Array object for storing temporary variables during execution.
 * \return The ID of the new object.
 */
object newContext(int link, object method, object args, object temp);

/*! Create a new Dictionary object.
 *
 * The newly created object is assigned the appropriate "Dictionary" class.
 * A new Array is created of the appropriate size, and assigned as the only
 * indexed member to be used as the hash table.
 *
 * \param size The initial size of the hash table.
 * \return The ID of the new object.
 */
object newDictionary(int size);

/*! Create a new Integer object.
 *
 * The newly created object is assigned the appropriate "Integer" class.
 *
 * \param value The integer value to assign to the object.
 * \return The ID of the new object.
 */
object newInteger(int value);

/*! Create a new Float object.
 *
 * The newly created object is assigned the appropriate "Float" class.
 * The data area will contain a binary representation of the float value
 * specified.
 *
 * \param value The integer value to assign to the object.
 * \return The ID of the new object.
 */
object newFloat(double value);

/*! Create a new Method object.
 *
 * The newly created object is assigned the appropriate "Method" class.
 *
 * \return The ID of the new object.
 */
object newMethod();

/*! Create a new Link object.
 *
 * A Link object is used to hold linked lists of hash values in a hash table.
 * The Link contains the key and value it represents, and a link to the next
 * in the list or nilobj if it is the tail.
 *
 * \param key The hash key for this entry.
 * \param value The value associated with the key.
 * \return The ID of the new object.
 */
object newLink(object key, object value);

/*! Create a new String object.
 *
 * The newly created object is assigned the appropriate "String" class. The
 * data area is allocated with enough space to hold the specified string, 
 * and the string data is copied into the area.
 *
 * \param str The character string to assign to the object.
 * \return The ID of the new object.
 */
object newStString(const char* str);

/*! Create a new Symbol object.
 *
 * Symbols are universally unique in Tumbleweed, so this function will first
 * check the existence of a matching symbol, and simply return that if found.
 * Otherwise, a new object is created, it's class set to "Symbol", the data
 * area is allocated and filled with the specified name. Finally, the symbol
 * is registered with the global symbol table automatically.
 *
 * \param name The name of the symbol to find or create.
 * \return The symbol object representing the given name.
 */
object newSymbol(const char* name);

/*! Create a new CPointer object.
 *
 * A CPointer object is used when communicating with the native side of 
 * Tumbleweed, primarily through the FFI interface. It is internally guaranteed
 * to be of the appropriate size for the architecture on which Tumbleweed is
 * running.
 *
 * \param l The pointer to store in the object data area.
 * \return The new CPointer object containing the pointer provided.
 */
object newCPointer(void* l);

/*! Read an image file into object storage.
 *
 * The current object storage is wiped completely, and replaced with the contents
 * of the specified image file. Object memory is grown sufficiently to contain all
 * object ID's specified in the image file.
 *
 * \param fp The opened file to read from.
 */
void imageRead(FILE* fp);

/*! Write the current object memory to an image file.
 *
 * The current memory is first garbage collected to eliminate dead objects, then
 * the whole object table is written to the specified file.
 * The first entry is the global symbols table.
 *
 * \param fp A file opened for write to copy the object table to.
 */
void imageWrite(FILE* fp);



//#define TW_SMALLINTEGER_AS_OBJECT
// TODO: Need to deal with SmallIntegers here
#if defined TW_SMALLINTEGER_AS_OBJECT

#define getInteger(x) (intValue((x)))
#define getClass(x) ((x)->_class)

#else

extern object g_intClass;
#define getInteger(x) ((x) >> 1)
#define getClass(x) (((x)&1)? ((classSyms[kInteger] == 0)? (globalSymbol("Integer")) : classSyms[kInteger].handle()) : (objectRef((x))._class))

#endif


#endif

