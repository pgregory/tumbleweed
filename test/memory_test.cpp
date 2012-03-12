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



