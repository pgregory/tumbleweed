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

class MemoryManager;
struct ObjectStruct;

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

inline int hashObject(object o)
{
    uintptr_t intval = (uintptr_t)o;
    return 55;
}


/*! \brief Object structure
 *
 * The default structure that represents all objects int he system.
 */
struct ObjectStruct 
{
    //! ID of the object that defines the class of this object.
    object _class;
    //! A reference counter, used only during mark/sweep GC.
    long referenceCount;
    //! The size of the data area, in object ID's
    long size;
    //! A pointer to the data area of the object.
    object*memory;

    //! Data pointer getter
    object* sysMemPtr();
    //! Get the data area as a byte pointer
    byte* bytePtr();
    //! Get the data area as a char pointer
    char* charPtr();

    /*! Basic indexed object getter.
     *
     * Get an object ID from the given index in the data area.
     *
     * \param index The index in the data area to access.
     * \return The object ID at the given index.
     */
    object basicAt(int index);
    /*! Basic indexed object insertion.
     *
     * Assigns the given object to the index if it is range.
     *
     * \param index The index to assign the value to.
     * \param o The object to assign.
     */
    void basicAtPut(int index, object o);
    /*! Byte accessor 
     *
     * Get the byte at the given index in the data area.
     *
     * \param index The index of the byte to get, if in range.
     * \return The byte value at the index.
     */
    int byteAt(int index);
    /*! Byte setter
     *
     * Set a value in the data area addressed as byte.
     *
     * \param index The index in the data area to update.
     * \param value The value to put into the data area.
     */
    void byteAtPut(int index, int value);

    /*! Float getter
     *
     * Get the value stored in the data area as a float.
     *
     * \return The float value this object holds.
     */
    double floatValue();
    /*! Int getter
     *
     * Get the value stored in the data area as an integer.
     *
     * \return The int value this object holds.
     */
    int intValue();
    /*! CPointer getter
     *
     * Get the value stored in the data area as a native C pointer.
     *
     * \return The pointer value this object holds.
     */
    void* cPointerValue();

};

# define nilobj (object)0
# define mBlockAlloc(size) (object *) calloc((unsigned) size, sizeof(object))
extern ObjectStruct _nilobj;

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
extern bool debugging;
void givepause();


#include <map>
#include <vector>
#include <string>

/*! \brief The memory manager
 *
 * The class that manages all objects in the system.
 * Objects are allocated into an array, the memory pointer then
 * directs to a malloced area.
 */
class MemoryManager
{
    static const size_t m_defaultInitialSize = 6500;
    static const size_t m_defaultGrowCount = 5000;
    private:
        //! Default constructor.
        /*! The default constructor is private as the memory manager is implemented
         *  as a singleton, all access is via the Instance() function.
         */
        MemoryManager(size_t initialSize = m_defaultInitialSize, size_t growCount = m_defaultGrowCount);
    public:
        //! Destructor
        ~MemoryManager();

        //! Singleton accessor
        /*! 
         * The memory manager is implemented as a singleton, all access to the
         * functionality is via this function.
         */
        static MemoryManager* Instance();

        static void Initialise(size_t initialSize = m_defaultInitialSize, size_t growCount = m_defaultGrowCount);

        long objectToIndex(object o);

        //! Transfer all unreferenced objects into the free list for reuse.
        void setFreeLists(); 

        //! Run a full mark/sweep garbage collect.
        /*! When GC is enabled, calling this function will force a complete
         *  pass over all objects accessible from the 'symbols' dictionar.
         *  Anything that can't be reached is collected as dead for reuse.
         *
         *  \return The number of objects freed.
         */
        int garbageCollect();

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


        size_t storageSize() const;

        /*! Dump object memory stats into a string.
         *
         * Get various stats about the status of the memory
         * manager into a string for display.
         *
         * \return A string showing the current memory manager status.
         */
        std::string statsString();

        /*! Dereference an object ID.
         *
         * Given an object ID, return the ObjectStruct that it represents.
         *
         * \param id The object ID to dereference.
         * \return A reference to the ObjectStruct
         */
        ObjectStruct& objectFromID(object id);

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
        bool destroyObject(object z);

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

        /*! Switch garbage collection on or off.
         *
         * Passing true will disable all GC, false will re-enable it. Calls to disableGC
         * are not recorded recursively, that is multiple calls to disable will be
         * countered by a single call to enable it.
         *
         * \param disable True to disable, false to enable.
         */
        void disableGC(bool disable);

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

        void setGrowAmount(size_t amount);

    private:
        bool            noGC;
        static MemoryManager* m_pInstance;

#if defined TW_UNIT_TESTS
        FRIEND_TEST(MemoryManagerTest, AllocateFromFree);
#endif
};

inline MemoryManager* MemoryManager::Instance()
{
    if(NULL == m_pInstance)
        m_pInstance = new MemoryManager();

    return m_pInstance;
}

inline ObjectStruct& MemoryManager::objectFromID(object id)
{
    if(id)
      return *id;
    else
      return _nilobj;
}


inline object* ObjectStruct::sysMemPtr()
{
    return memory;
}

inline byte* ObjectStruct::bytePtr()
{
    return (byte *)sysMemPtr();
}

inline char* ObjectStruct::charPtr()
{
    return (char *)sysMemPtr();
}


inline void ObjectStruct::basicAtPut(int i, object v)
{
    if((i <= 0) || (i > size))
        sysError("index out of range", "basicAtPut");
    else
        sysMemPtr()[i-1] = v;
}



//#define TW_SMALLINTEGER_AS_OBJECT
// TODO: Need to deal with SmallIntegers here
#define objectRef(x) (MemoryManager::Instance()->objectFromID((x)))
#if defined TW_SMALLINTEGER_AS_OBJECT

#define getInteger(x) (objectRef((x)).intValue())
#define getClass(x) (objectRef((x))._class)

#else

extern object g_intClass;
#define getInteger(x) ((x) >> 1)
#define getClass(x) (((x)&1)? ((classSyms[kInteger] == 0)? (globalSymbol("Integer")) : classSyms[kInteger].handle()) : (objectRef((x))._class))

#endif


#endif

