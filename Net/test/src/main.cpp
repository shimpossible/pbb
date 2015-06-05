#include <stdio.h>
#include <WinSock2.h>
#include "gtest/gtest.h"

int main(int argc, char **argv) {

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
