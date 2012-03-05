#include <gtest/gtest.h>
#include "memory.h"


class MemoryManagerTest : public ::testing::Test
{
  protected:
    virtual void SetUp() 
    {
      // Allocate a simple object with size.
      m2_arrayID = m2.allocObject(10);

      // Create a MemoryManager with a specific size.
      m3 = new MemoryManager(6);
      m3->setGrowAmount(10);
      // Add some objects for gc.
      m3->allocObject(1);
      m3->allocObject(2);
      m3->allocObject(4);
      m3->allocObject(8);
      m3->allocObject(8);
    }

    virtual void TearDown()
    {
      delete m3;
    }

    MemoryManager m1;
    MemoryManager m2;
    MemoryManager* m3;
    object        m2_arrayID;
};
    

TEST_F(MemoryManagerTest, IsInitiallyEmpty)
{
  // Check that on initial setup, the only valid object is nilobj
  EXPECT_EQ(1, m1.objectCount());
  // Check that the free slots table is 6499 (6500 less nilobj)
  EXPECT_EQ(6499, m1.freeSlotsCount());
  // Check that a call to garbageCollect results in no change.
  EXPECT_EQ(0, m1.garbageCollect());
}


TEST_F(MemoryManagerTest, DereferenceNil)
{
  // Check that dereferencing 0 results in the nilobj
  EXPECT_EQ(0, m1.objectFromID(0).size);
}

TEST_F(MemoryManagerTest, ObjectSize)
{
  ASSERT_EQ(10, m2.objectFromID(m2_arrayID).size);
}


TEST_F(MemoryManagerTest, GarbageCollect)
{
  // Do a GC on the manager m3, should free 5 objects
  EXPECT_EQ(5, m3->garbageCollect());

  // With GC working, we should be able to reallocate
  // similar objects, without changing the size.
  m3->objectFromID(m3->allocObject(1)).referenceCount = 1;
  EXPECT_EQ(2, m3->objectCount());
}

TEST_F(MemoryManagerTest, GrowObjectTable)
{
  // Hold references to all objects, so the table must grow.
  for(int i = 1; i < 6; ++i)
    m3->refObject(i);
  // Check that the system is able to reallocated the object
  // table to accommodate additional objects.
  m3->objectFromID(m3->allocObject(1)).referenceCount = 1;
  // Should be 7 objects including the new one...
  EXPECT_EQ(7, m3->objectCount());
  // ...and 9 more slots, as growAmount is set to 10.
  EXPECT_EQ(9, m3->freeSlotsCount());
  for(int i = 1; i < 6; ++i)
    m3->unrefObject(i);
}



