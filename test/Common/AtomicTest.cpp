#include "gtest/gtest.h"
#include <stdio.h>
#include "pbb/Atomic.h"

class AtomicTest : public ::testing::Test {
public:
};

TEST_F(AtomicTest, Add)
{
   pbb::Atomic<int32_t> val(0);
   ++val;

    ASSERT_EQ(1, val);
} 