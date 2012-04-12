/*
    Little Smalltalk, version 2
    Written by Tim Budd, Oregon State University, July 1987
*/

#include "env.h"

#include "gtest/gtest_prod.h"

class MemoryManager;
struct objectStruct;
/*! \brief Auto referencing object handle class.
 *
 * Use this class to hold a reference to a newly created object so
 * that it won't be collected by the garbage collector before it is
 * assigned to a place in the object storage. 
 * The handle automatically records the reference on creation, and 
 * releases it when destroyed, so it just needs to be properly 
 * scoped
 */
class ObjectHandle
{
    public:
        //! Default constructor
        /*! By default, the handle refers to the nilobj. */
        ObjectHandle();
        //! Constructor
        /*! Takes an object ID, automatically records the reference 
         *
         * \param object The object to reference, it will be managed by the
         *        default singleton memory manager
         *
         */
        //! Copy constructor
        /*! Ensures that the copy correctly references the object.
         *
         * \param from The ObjectHandle to duplicate.
         */
        ObjectHandle(const ObjectHandle& from);
        ObjectHandle(long object);
        //! Constructor
        /*! Takes an object ID and manager to reference against, 
         *  automatically records the reference 
         *
         * \param object The object to reference, it will be managed by the
         *        default singleton memory manager
         * \param manager Pointer to the manager to reference with.
         *
         */
        ObjectHandle(long object, MemoryManager* manager);
        //! Destructor
        /*! Releases any reference, allowing the object to be collected,
         *  unless it has been assigned to a slot in the object storage
         *  reachable from the Symbols list.
         */
        ~ObjectHandle();

        //! Cast to object struct
        operator objectStruct&() const;

        operator long() const;

        objectStruct* operator->() const;

        /*! Accessor for the object ID referenced
         *
         * \return The object handle
         */
        long handle() const
        {
            return m_handle;
        }

        int hash() const;

        //! Assignement operator
        /*!
         *  Assign a new handle to be referenced.
         *  If an existing handle is referenced, it will be 
         *  released first.
         *
         *  \param o An object reference.
         *  \return A reference to this object.
         */
        ObjectHandle& operator=(long o);

    private:
        long    m_handle;
};


typedef long object;


/*! \brief Object structure
 *
 * The default structure that represents all objects int he system.
 */
struct objectStruct 
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


# define nilobj (object) 0

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
typedef std::map<object, long>    TObjectRefs;
typedef TObjectFreeList::iterator   TObjectFreeListIterator;
typedef TObjectFreeListInv::iterator   TObjectFreeListInvIterator;
typedef TObjectFreeList::reverse_iterator   TObjectFreeListRevIterator;

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

        //! Mark function for the garbage collection.
        /*! When performing the mark part of mark/sweep, this function is called
         *  for each object. It recursively calls visit on all object ID's in the 
         *  data area for any object that has standard object data.
         *
         *  \param x The object to start marking from.
         */
        void visit(register object x);

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
        objectStruct& objectFromID(object id);

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

        /*! Register a held reference to an object
         *
         * Objects registered via this mechanism will be guaranteed not to be collected
         * during garbage collection, until the matching unrefObject is called. This 
         * should not normally be used, instead use the ObjectHandle class to maintain
         * references. Internally, the memory manager maintains a count of references
         * registered against each object.
         *
         * \param o The object to register a reference against.
         */
        void refObject(long o);
        /*! Release a previously registered object reference.
         *
         * Partner call to refObject, that releases a held reference. If the object is 
         * found in the internally managed reference table, the count of references is 
         * reduced by 1. If it is zero or less, the reference is dropped entirely, meaning
         * that the object is once again visible to the garbage collection, provided it is
         * not referenced internally within the object memory space.
         *
         * \param o The object to release.
         */
        void unrefObject(long o);

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
        TObjectFreeList objectFreeList;
        TObjectFreeListInv objectFreeListInv;
        TObjectTable    objectTable;
        TObjectRefs     objectReferences;
        bool            noGC;
        size_t          growAmount;

        static MemoryManager* m_pInstance;

        size_t growObjectStore(size_t amount);

        FRIEND_TEST(MemoryManagerTest, AllocateFromFree);
};

extern MemoryManager* theMemoryManager;

#define objectRef(x) (MemoryManager::Instance()->objectFromID(x))

inline ObjectHandle::ObjectHandle() : m_handle(0)
{
}

inline ObjectHandle::ObjectHandle(const ObjectHandle& from)
{
    m_handle = from.m_handle;
    if(m_handle)
        MemoryManager::Instance()->refObject(m_handle);
}

inline ObjectHandle::ObjectHandle(long object) : m_handle(object) 
{
    MemoryManager::Instance()->refObject(object);
}

inline ObjectHandle::ObjectHandle(long object, MemoryManager* manager) : 
  m_handle(object)
{
    MemoryManager::Instance()->refObject(object);
}

inline ObjectHandle::~ObjectHandle() 
{
    MemoryManager::Instance()->unrefObject(handle());
}

inline ObjectHandle& ObjectHandle::operator=(long o)
{
    if(m_handle)
        MemoryManager::Instance()->unrefObject(m_handle);
    m_handle = o;
    MemoryManager::Instance()->refObject(o);

    return *this;
}

inline objectStruct* ObjectHandle::operator->() const
{
  return &MemoryManager::Instance()->objectFromID(m_handle);
}

inline ObjectHandle::operator objectStruct&() const
{
    return MemoryManager::Instance()->objectFromID(m_handle);
}

inline ObjectHandle::operator long() const
{
    return m_handle;
}

inline int ObjectHandle::hash() const
{
    return m_handle;
}
