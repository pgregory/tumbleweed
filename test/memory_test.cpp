#include <gtest/gtest.h>
#include "memory.h"


class MemoryManagerTest : public ::testing::Test
{
  protected:
    virtual void SetUp() 
    {
      MemoryManager::Instance()->Initialise(7, 10);
      // Allocate a simple object with size.
      arrayID = MemoryManager::Instance()->allocObject(10);
    }

    virtual void TearDown()
    {
    }

    ObjectHandle        arrayID;
};
    

TEST_F(MemoryManagerTest, IsInitiallyEmpty)
{
  // Check that on initial setup, the only valid object is nilobj
  EXPECT_EQ(2, MemoryManager::Instance()->objectCount());
  // Check that the free slots table is 6499 (6500 less nilobj)
  EXPECT_EQ(5, MemoryManager::Instance()->freeSlotsCount());
  // Check that a call to garbageCollect results in no change.
  EXPECT_EQ(0, MemoryManager::Instance()->garbageCollect());
}


TEST_F(MemoryManagerTest, DereferenceNil)
{
  // Check that dereferencing 0 results in the nilobj
  EXPECT_EQ(0, MemoryManager::Instance()->objectFromID(0).size);
}

TEST_F(MemoryManagerTest, ObjectSize)
{
  ASSERT_EQ(10, MemoryManager::Instance()->objectFromID(arrayID).size);
}


TEST_F(MemoryManagerTest, GarbageCollect)
{
  // Allocate one object size 7, for later.
  MemoryManager::Instance()->allocObject(7);
  // Allocate 4 more objects, to fill the space.
  for(int i = 0; i < 4; ++i)
    MemoryManager::Instance()->allocObject(1);
  EXPECT_EQ(0, MemoryManager::Instance()->freeSlotsCount());
  // Do a GC on the manager m3, should free 5 objects
  EXPECT_EQ(5, MemoryManager::Instance()->garbageCollect());

  // With GC working, we should be able to reallocate
  // similar objects, without changing the size.
  ObjectHandle o1 = MemoryManager::Instance()->allocObject(7);
  EXPECT_EQ(7, MemoryManager::Instance()->storageSize());
  EXPECT_EQ(3, MemoryManager::Instance()->objectCount());
}

TEST_F(MemoryManagerTest, GrowObjectTable)
{
  // Hold references to all objects, so the table must grow.
  ObjectHandle o1 = MemoryManager::Instance()->allocObject(1);
  ObjectHandle o2 = MemoryManager::Instance()->allocObject(1);
  ObjectHandle o3 = MemoryManager::Instance()->allocObject(1);
  ObjectHandle o4 = MemoryManager::Instance()->allocObject(1);
  ObjectHandle o5 = MemoryManager::Instance()->allocObject(1);
  EXPECT_EQ(0, MemoryManager::Instance()->freeSlotsCount());
  EXPECT_EQ(7, MemoryManager::Instance()->objectCount());
  // Check that the system is able to reallocated the object
  // table to accommodate additional objects.
  MemoryManager::Instance()->allocObject(1);
  // Should be 7 objects including the new one...
  EXPECT_EQ(8, MemoryManager::Instance()->objectCount());
  // ...and 9 more slots, as growAmount is set to 10.
  EXPECT_EQ(9, MemoryManager::Instance()->freeSlotsCount());
}

TEST_F(MemoryManagerTest, AllocateFromFree)
{
  {
    ObjectHandle o1 = MemoryManager::Instance()->allocObject(1);
    ObjectHandle o2 = MemoryManager::Instance()->allocObject(2);
    ObjectHandle o3 = MemoryManager::Instance()->allocObject(3);
    ObjectHandle o4 = MemoryManager::Instance()->allocObject(3);
    ObjectHandle o5 = MemoryManager::Instance()->allocObject(3);
    EXPECT_EQ(0, MemoryManager::Instance()->freeSlotsCount());
    EXPECT_EQ(7, MemoryManager::Instance()->objectCount());
  }

  // As the object handles are scoped, they should free on the
  // first allocation, allowing the manager to deliver an object
  // from the free store.
  ObjectHandle o1 = MemoryManager::Instance()->allocObject(2);
  // After one allocation, the GC should result in 5 freed, then one 
  // is reused, resulting in 4 free in the tracker.
  EXPECT_EQ(1, MemoryManager::Instance()->objectFreeList.count(1));
  EXPECT_EQ(0, MemoryManager::Instance()->objectFreeList.count(2));
  EXPECT_EQ(3, MemoryManager::Instance()->objectFreeList.count(3));
  ObjectHandle o2 = MemoryManager::Instance()->allocObject(2);
  // After allocating the second, there should be 3 free, 4 objects,
  // and the three free should all be of size 3.
  EXPECT_EQ(3, MemoryManager::Instance()->freeSlotsCount());
  EXPECT_EQ(4, MemoryManager::Instance()->objectCount());
  EXPECT_EQ(7, MemoryManager::Instance()->storageSize());
  EXPECT_EQ(0, MemoryManager::Instance()->objectFreeList.count(2));
  EXPECT_EQ(1, MemoryManager::Instance()->objectFreeList.count(1));
  EXPECT_EQ(2, MemoryManager::Instance()->objectFreeList.count(3));
}



TEST(ObjectHandleTest, AppendToList)
{
  // Check that the handle list is empty to start
  EXPECT_EQ(0, ObjectHandle::numTotalHandles());
  // Check that appending to the list creates a single
  // entry list.
  ObjectHandle h1(1);
  EXPECT_EQ(1, ObjectHandle::numTotalHandles());
  EXPECT_TRUE(ObjectHandle::getListHead() == &h1);
  EXPECT_TRUE(ObjectHandle::getListHead()->handle() == 1);
  EXPECT_TRUE(ObjectHandle::getListHead() == ObjectHandle::getListTail());
  EXPECT_TRUE(h1.prev() == NULL);
  EXPECT_TRUE(h1.next() == NULL);
  // Add another, should append to the end
  ObjectHandle h2(2);
  EXPECT_EQ(2, ObjectHandle::numTotalHandles());
  EXPECT_TRUE(ObjectHandle::getListTail() == &h2);
  EXPECT_TRUE(ObjectHandle::getListHead() == &h1);
  EXPECT_TRUE(h1.next() == &h2);
  EXPECT_TRUE(h2.prev() == &h1);
  EXPECT_TRUE(h1.prev() == NULL);
  EXPECT_TRUE(h2.next() == NULL);
  // Add another, check linkage
  ObjectHandle h3(3);
  EXPECT_EQ(3, ObjectHandle::numTotalHandles());
  EXPECT_TRUE(ObjectHandle::getListTail() == &h3);
  EXPECT_TRUE(ObjectHandle::getListHead() == &h1);
  EXPECT_TRUE(h2.next() == &h3);
  EXPECT_TRUE(h3.prev() == &h2);
  EXPECT_TRUE(h3.next() == NULL);
}

TEST(ObjectHandleTest, RemoveIsolatedHeadFromList)
{
  // Check that the previous tests have left the list empty.
  EXPECT_EQ(0, ObjectHandle::numTotalHandles());
  // Add a single entry
  {
    ObjectHandle h1(1);
    EXPECT_EQ(1, ObjectHandle::numTotalHandles());
    EXPECT_TRUE(&h1 == ObjectHandle::getListHead()); 
    EXPECT_TRUE(&h1 == ObjectHandle::getListTail()); 
    // When it goes out of scope, it should be removed from the list,
    // leaving an empty list.
  }
  EXPECT_EQ(0, ObjectHandle::numTotalHandles());
  EXPECT_TRUE(NULL == ObjectHandle::getListHead()); 
  EXPECT_TRUE(NULL == ObjectHandle::getListTail()); 
}

TEST(ObjectHandleTest, RemoveHeadFromList)
{
  // Check that the previous tests have left the list empty.
  EXPECT_EQ(0, ObjectHandle::numTotalHandles());
  // Allocate a new object handle that we can manually delete.
  ObjectHandle* h1 = new ObjectHandle(1);
  // ..and a static one that will go out of scope automatically.
  ObjectHandle h2(2);
  EXPECT_EQ(2, ObjectHandle::numTotalHandles());
  EXPECT_TRUE(ObjectHandle::getListHead() == h1);
  EXPECT_TRUE(ObjectHandle::getListTail() == &h2);

  delete(h1);

  EXPECT_EQ(1, ObjectHandle::numTotalHandles());
  EXPECT_TRUE(ObjectHandle::getListHead() == &h2); 
  EXPECT_TRUE(ObjectHandle::getListTail() == &h2); 
}


TEST(ObjectHandleTest, RemoveTailFromList)
{
  // Check that the previous tests have left the list empty.
  EXPECT_EQ(0, ObjectHandle::numTotalHandles());
  // Add one to the head
  ObjectHandle h1(1);
  {
    ObjectHandle h2(2);
    EXPECT_EQ(2, ObjectHandle::numTotalHandles());
    EXPECT_TRUE(&h1 == ObjectHandle::getListHead());
    EXPECT_TRUE(&h2 == ObjectHandle::getListTail());
  }
  EXPECT_EQ(1, ObjectHandle::numTotalHandles());
  EXPECT_TRUE(&h1 == ObjectHandle::getListHead());
  EXPECT_TRUE(&h1 == ObjectHandle::getListTail());
}


TEST(ObjectHandleTest, RemoveInternalFromList)
{
  // Check that the previous tests have left the list empty.
  EXPECT_EQ(0, ObjectHandle::numTotalHandles());

  // Add two on the stack
  ObjectHandle h1;
  ObjectHandle h2;
  // and one on the heap
  ObjectHandle* h3 = new ObjectHandle();
  // and another two on the stack
  ObjectHandle h4;
  ObjectHandle h5;

  EXPECT_EQ(5, ObjectHandle::numTotalHandles());
  EXPECT_TRUE(&h1 == ObjectHandle::getListHead());
  EXPECT_TRUE(&h5 == ObjectHandle::getListTail());
  EXPECT_TRUE(NULL == h1.prev());
  EXPECT_TRUE(&h2 == h1.next());
  EXPECT_TRUE(&h1 == h2.prev());
  EXPECT_TRUE(h3 == h2.next());
  EXPECT_TRUE(&h2 == h3->prev());
  EXPECT_TRUE(&h4 == h3->next());
  EXPECT_TRUE(h3 == h4.prev());
  EXPECT_TRUE(&h5 == h4.next());
  EXPECT_TRUE(&h4 == h5.prev());
  EXPECT_TRUE(NULL == h5.next());

  // Now delete h3 from the middle.
  delete(h3);

  EXPECT_EQ(4, ObjectHandle::numTotalHandles());
  EXPECT_TRUE(&h1 == ObjectHandle::getListHead());
  EXPECT_TRUE(&h5 == ObjectHandle::getListTail());
  EXPECT_TRUE(NULL == h1.prev());
  EXPECT_TRUE(&h2 == h1.next());
  EXPECT_TRUE(&h1 == h2.prev());
  EXPECT_TRUE(&h4 == h2.next());
  EXPECT_TRUE(&h2 == h4.prev());
  EXPECT_TRUE(&h5 == h4.next());
  EXPECT_TRUE(&h4 == h5.prev());
  EXPECT_TRUE(NULL == h5.next());
}

TEST(ObjectHandleTest, ModifyInternalInList)
{
  // Check that the previous tests have left the list empty.
  EXPECT_EQ(0, ObjectHandle::numTotalHandles());

  // Add some on the stack
  ObjectHandle h1(1);
  ObjectHandle h2(2);
  ObjectHandle h3;
  ObjectHandle h4(4);
  ObjectHandle h5(5);

  EXPECT_EQ(5, ObjectHandle::numTotalHandles());
  EXPECT_TRUE(&h1 == ObjectHandle::getListHead());
  EXPECT_TRUE(&h5 == ObjectHandle::getListTail());
  EXPECT_TRUE(NULL == h1.prev());
  EXPECT_TRUE(&h2 == h1.next());
  EXPECT_TRUE(&h1 == h2.prev());
  EXPECT_TRUE(&h3 == h2.next());
  EXPECT_TRUE(&h2 == h3.prev());
  EXPECT_TRUE(&h4 == h3.next());
  EXPECT_TRUE(&h3 == h4.prev());
  EXPECT_TRUE(&h5 == h4.next());
  EXPECT_TRUE(&h4 == h5.prev());
  EXPECT_TRUE(NULL == h5.next());

  h3 = 3;

  EXPECT_EQ(5, ObjectHandle::numTotalHandles());
  EXPECT_TRUE(&h1 == ObjectHandle::getListHead());
  EXPECT_TRUE(&h5 == ObjectHandle::getListTail());
  EXPECT_TRUE(NULL == h1.prev());
  EXPECT_TRUE(&h2 == h1.next());
  EXPECT_TRUE(&h1 == h2.prev());
  EXPECT_TRUE(&h3 == h2.next());
  EXPECT_TRUE(&h2 == h3.prev());
  EXPECT_TRUE(&h4 == h3.next());
  EXPECT_TRUE(&h3 == h4.prev());
  EXPECT_TRUE(&h5 == h4.next());
  EXPECT_TRUE(&h4 == h5.prev());
  EXPECT_TRUE(NULL == h5.next());
  EXPECT_EQ(3, h3.handle());

  h3 = ObjectHandle(6);

  EXPECT_EQ(5, ObjectHandle::numTotalHandles());
  EXPECT_TRUE(&h1 == ObjectHandle::getListHead());
  EXPECT_TRUE(&h5 == ObjectHandle::getListTail());
  EXPECT_TRUE(NULL == h1.prev());
  EXPECT_TRUE(&h2 == h1.next());
  EXPECT_TRUE(&h1 == h2.prev());
  EXPECT_TRUE(&h3 == h2.next());
  EXPECT_TRUE(&h2 == h3.prev());
  EXPECT_TRUE(&h4 == h3.next());
  EXPECT_TRUE(&h3 == h4.prev());
  EXPECT_TRUE(&h5 == h4.next());
  EXPECT_TRUE(&h4 == h5.prev());
  EXPECT_TRUE(NULL == h5.next());
  EXPECT_EQ(6, h3.handle());
}
