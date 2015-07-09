#include <pbb/net/SocketManager.h>
#include "gtest/gtest.h"
#include <list>

using namespace pbb::net;

class SocketManagerTest : public ::testing::Test {
public:
};

struct SocketManagerOps {
    SocketManagerOps() {};
    ~SocketManagerOps() { };
    int type;
    union
    {
        struct a {
            pbb::net::Socket* socket;
            pbb::net::SocketManager::State state;
        } state_changed;
        struct b {
            pbb::net::Socket* socket;
            pbb::net::Socket* remote;
            pbb::net::SocketAddress address;
        } accepted;
        struct {
            pbb::net::Socket* socket;
            void* data;
            size_t len;
        } received;
    };
};

std::list<SocketManagerOps> opsData;

static void test_state_changed(pbb::net::Socket* socket, pbb::net::SocketManager::State state)
{
    SocketManagerOps ops;
    ops.type = 0;
    ops.state_changed.socket = socket;
    ops.state_changed.state = state;
    
    opsData.push_back(ops);
}
static void test_accepted(pbb::net::Socket* socket, pbb::net::Socket* remote, pbb::net::SocketAddress& address)
{
    SocketManagerOps ops;
    ops.type = 1;
    ops.accepted.socket  = socket;
    ops.accepted.remote  = remote;
    ops.accepted.address = address;

    opsData.push_back(ops);
}
static void test_received(pbb::net::Socket* socket, void* data, size_t len)
{
    SocketManagerOps ops;
    ops.type = 2;
    //ops.received.socket = socket;
    // TOOD: copy the data
    //ops.received.data = data;
    //ops.received.len = len;

    opsData.push_back(ops);
}

TEST_F(SocketManagerTest, OpenAndListen)
{
    Error e;
    SocketManager::SocketCallback ops;
    ops.accepted      = test_accepted;
    ops.received      = test_received;
    ops.state_changed = test_state_changed;
    SocketManager mgr;

    // Creates a new socket on random port
    Socket* serverSock = mgr.OpenAndListen(0, ops);

    ASSERT_NE((Socket*)0, serverSock);
    ASSERT_NE(Socket::InvalidSocket, serverSock);

    ASSERT_EQ(1, opsData.size());
    // check callback was called
    ASSERT_EQ(0, opsData.front().type);
    ASSERT_EQ(serverSock, opsData.front().state_changed.socket);
    ASSERT_EQ(SocketManager::LISTENING, opsData.front().state_changed.state);
    opsData.pop_front();

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
    do
    {
        e = serverSock->Accept(client);
    } while (e == PBB_EAGAIN);

    ASSERT_EQ(PBB_ESUCCESS, e);
    ASSERT_NE((Socket*)0, client);

    e = client->Close(); ASSERT_EQ(PBB_ESUCCESS, e);
    e = other->Close();  ASSERT_EQ(PBB_ESUCCESS, e);

    e = mgr.Close(serverSock); ASSERT_EQ(PBB_ESUCCESS, e);
}


TEST_F(SocketManagerTest, ConnectTo)
{
    Error e;
    SocketManager::SocketCallback ops;
    ops.accepted      = test_accepted;
    ops.received      = test_received;
    ops.state_changed = test_state_changed;
    SocketManager mgr;

    // Creates a new socket on random port
    Socket* serverSock = mgr.OpenAndListen(0, ops);

    ASSERT_NE((Socket*)0, serverSock);
    ASSERT_NE(Socket::InvalidSocket, serverSock);

    // check callback was called
    ASSERT_EQ(1, opsData.size());
    ASSERT_EQ(serverSock, opsData.front().state_changed.socket);
    ASSERT_EQ(SocketManager::LISTENING, opsData.front().state_changed.state);
    opsData.pop_front();

    // verify it has a port
    SocketAddress addr;
    e = serverSock->Address(addr);
    ASSERT_EQ(PBB_ESUCCESS, e);
    ASSERT_NE(0, addr.Port()); // must have a non-zero port

    Socket* client = mgr.ConnectTo("127.0.0.1", addr.Port(), ops);
    ASSERT_NE((Socket*)0, serverSock);

    // check callback was called
    ASSERT_EQ(1, opsData.size());
    ASSERT_EQ(0, opsData.front().type);
    ASSERT_EQ(client, opsData.front().state_changed.socket);
    ASSERT_EQ(SocketManager::PENDING_CONNECTION, opsData.front().state_changed.state);
    opsData.pop_front();

    // do an update
    mgr.Update();

    // check callback was called, one for ACCEPT and one for CONNECTED
    ASSERT_EQ(2, opsData.size());

    // can be 2 differnt orders
    if(0 == opsData.front().type)
    {
        ASSERT_EQ(client, opsData.front().state_changed.socket);
        ASSERT_EQ(SocketManager::CONNECTED, opsData.front().state_changed.state);
        opsData.pop_front();

        ASSERT_EQ(1, opsData.front().type);
        ASSERT_EQ(serverSock, opsData.front().accepted.socket);
        ASSERT_NE(Socket::InvalidSocket, opsData.front().accepted.remote);
        opsData.pop_front();
    }
    else
    {
        ASSERT_EQ(serverSock, opsData.front().accepted.socket);
        ASSERT_NE(Socket::InvalidSocket, opsData.front().accepted.remote);
        opsData.pop_front();

        ASSERT_EQ(0, opsData.front().type);
        ASSERT_EQ(client, opsData.front().state_changed.socket);
        ASSERT_EQ(SocketManager::CONNECTED, opsData.front().state_changed.state);
        opsData.pop_front();

    }
    
}