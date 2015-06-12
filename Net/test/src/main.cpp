#include <stdio.h>
#ifdef WIN32
#include <WinSock2.h>
#endif
#include "gtest/gtest.h"

int main(int argc, char **argv) {

#ifdef WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
