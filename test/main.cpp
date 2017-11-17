#include <stdio.h>
#ifdef WIN32
#include <WinSock2.h>
#endif
#include "gtest/gtest.h"

int main(int argc, char **argv) {

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
