#include <pbb/net/SocketManager.h>
#include "gtest/gtest.h"

using namespace pbb::net;

class SocketManagerTest : public ::testing::Test {
public:
};

TEST_F(SocketManagerTest, OpenAndListen)
{
    Error e;
    SocketManager::SocketCallback ops;
    ops.accepted = 0;
    ops.received = 0;
    ops.state_changed = 0;
    SocketManager mgr;

    // Creates a new socket on random port
    Socket* serverSock = mgr.OpenAndListen(0, ops);

    ASSERT_NE((Socket*)0, serverSock);
    ASSERT_NE(Socket::InvalidSocket(), serverSock);

    // verify it has a port
    SocketAddress addr;
    e = serverSock->Address(addr);
    ASSERT_EQ(PBB_ESUCCESS, e);
    ASSERT_NE(0, addr.Port()); // must have a non-zero port

    // Connect to server socket
    Socket* other = Socket::Create(SocketAddress::INET, Socket::TCP);
    SocketAddress serverAddr(SocketAddress::INET, "127.0.0.1", addr.Port());
    e = other->Connect(serverAddr);
    ASSERT_EQ(PBB_ESUCCESS, e);

    // Accept client trying to connect
    Socket* client = 0;
    e = serverSock->Accept(client);

    ASSERT_EQ(PBB_ESUCCESS, e);
    ASSERT_NE((Socket*)0, client);

    e = client->Close(); ASSERT_EQ(PBB_ESUCCESS, e);
    e = other->Close();  ASSERT_EQ(PBB_ESUCCESS, e);

    e = mgr.Close(serverSock); ASSERT_EQ(PBB_ESUCCESS, e);
    
}