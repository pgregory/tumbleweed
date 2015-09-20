#include "gtest/gtest.h"
#include "objmemory.h"

int main(int argc, char **argv)
{
    std::cout << "Initial: " << ObjectHandle::getListHead() << std:: endl;
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}
