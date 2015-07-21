#include <pbb/net/SocketManager.h>
#include "gtest/gtest.h"
#include <list>

using namespace pbb::net;

class SocketManagerTest : public ::testing::Test {
public:
};

typedef struct SocketManagerOps SocketManagerOps;

struct SocketManagerOps {
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
	SocketManagerOps() {}
	//~SocketManagerOps() {}
	SocketManagerOps(const SocketManagerOps& other)
	{
		// copy
		memcpy(this, &other, sizeof(*this));
	};
};

class TestCallbacks : public SocketManager::ISocketCallback
{
public:
    std::list<SocketManagerOps> opsData;
    void state_changed(pbb::net::Socket* socket, pbb::net::SocketManager::State state)
    {
        SocketManagerOps ops;
        ops.type = 0;
        ops.state_changed.socket = socket;
        ops.state_changed.state = state;

        opsData.push_back(ops);
    }
    void accepted(pbb::net::Socket* socket, pbb::net::Socket* remote, pbb::net::SocketAddress& address)
    {
        SocketManagerOps ops;
        ops.type = 1;
        ops.accepted.socket = socket;
        ops.accepted.remote = remote;
        ops.accepted.address = address;

        opsData.push_back(ops);
    }
    void received(pbb::net::Socket* socket, void* data, size_t len)
    {
        SocketManagerOps ops;
        ops.type = 2;
        ops.received.socket = socket;
        ops.received.data = malloc(len);
        memcpy(ops.received.data, data, len);
        ops.received.len = len;

        opsData.push_back(ops);
    }
};

TEST_F(SocketManagerTest, OpenAndListen)
{
    Error e;   
    SocketManager mgr;
    TestCallbacks ops;

    // Creates a new socket on random port
    Socket* serverSock = mgr.OpenAndListen(0, ops);

    ASSERT_NE((Socket*)0, serverSock);
    ASSERT_NE(Socket::InvalidSocket, serverSock);

    ASSERT_EQ(1, ops.opsData.size());
    // check callback was called
    ASSERT_EQ(0, ops.opsData.front().type);
    ASSERT_EQ(serverSock, ops.opsData.front().state_changed.socket);
    ASSERT_EQ(SocketManager::LISTENING, ops.opsData.front().state_changed.state);
    ops.opsData.pop_front();

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

    // Wait for socket to be ready
    SocketCollection rd;
    rd.push_back(serverSock);
    int ready;
    Socket::Select(&rd, 0, 0, 100, ready);

    // Accept client trying to connect
    Socket* client = 0;
    e = serverSock->Accept(client);

    ASSERT_EQ(PBB_ESUCCESS, e);
    ASSERT_NE((Socket*)0, client);

    e = client->Close(); ASSERT_EQ(PBB_ESUCCESS, e);
    e = other->Close();  ASSERT_EQ(PBB_ESUCCESS, e);

    e = mgr.Close(serverSock); ASSERT_EQ(PBB_ESUCCESS, e);
}


TEST_F(SocketManagerTest, ConnectTo)
{
    Error e;
    SocketManager mgr;
    TestCallbacks opsListen;
    TestCallbacks opsConnect;

    // Creates a new socket on random port
    Socket* serverSock = mgr.OpenAndListen(0, opsListen);

    ASSERT_NE((Socket*)0, serverSock);
    ASSERT_NE(Socket::InvalidSocket, serverSock);

    // check callback was called
    ASSERT_EQ(1, opsListen.opsData.size());
    ASSERT_EQ(serverSock, opsListen.opsData.front().state_changed.socket);
    ASSERT_EQ(SocketManager::LISTENING, opsListen.opsData.front().state_changed.state);
    opsListen.opsData.pop_front();

    // verify it has a port
    SocketAddress addr;
    e = serverSock->Address(addr);
    ASSERT_EQ(PBB_ESUCCESS, e);
    ASSERT_NE(0, addr.Port()); // must have a non-zero port

    Socket* client = mgr.ConnectTo("127.0.0.1", addr.Port(), opsConnect);
    ASSERT_NE((Socket*)0, serverSock);

    // check callback was called
    ASSERT_EQ(1, opsConnect.opsData.size());
    ASSERT_EQ(0, opsConnect.opsData.front().type);
    ASSERT_EQ(client, opsConnect.opsData.front().state_changed.socket);
    ASSERT_EQ(SocketManager::PENDING_CONNECTION, opsConnect.opsData.front().state_changed.state);
    opsConnect.opsData.pop_front();

    // do an update
    mgr.Update();

    // check callback was called, one for ACCEPT and one for CONNECTED
    ASSERT_EQ(1, opsConnect.opsData.size());
    ASSERT_EQ(1, opsListen.opsData.size());

    ASSERT_EQ(client, opsConnect.opsData.front().state_changed.socket);
    ASSERT_EQ(SocketManager::CONNECTED, opsConnect.opsData.front().state_changed.state);
    opsConnect.opsData.pop_front();

    ASSERT_EQ(1, opsListen.opsData.front().type);
    ASSERT_EQ(serverSock, opsListen.opsData.front().accepted.socket);
    ASSERT_NE(Socket::InvalidSocket, opsListen.opsData.front().accepted.remote);
    opsListen.opsData.pop_front();
}

TEST_F(SocketManagerTest, OnReceive)
{
    Error e;
    SocketManager mgr;
    TestCallbacks ops;

    // Creates a new socket on random port
    Socket* serverSock = mgr.OpenAndListen(0, ops);

    ASSERT_NE((Socket*)0, serverSock);
    ASSERT_NE(Socket::InvalidSocket, serverSock);

    ASSERT_EQ(1, ops.opsData.size());
    ops.opsData.pop_front();

    // verify it has a port
    SocketAddress addr;
    e = serverSock->Address(addr);

    // Connect to server socket
    Socket* other = Socket::Create(SocketAddress::INET, Socket::TCP);
    SocketAddress serverAddr(SocketAddress::INET, "127.0.0.1", addr.Port());
    e = other->Connect(serverAddr);
    ASSERT_EQ(PBB_ESUCCESS, e);

    // Wait for socket to be ready
    SocketCollection rd;
    rd.push_back(serverSock);
    int ready;
    Socket::Select(&rd, 0, 0, 100, ready);

    // Accept client trying to connect
    mgr.Update();

    ASSERT_EQ(1, ops.opsData.size());
    ASSERT_EQ(1, ops.opsData.front().type); // connected
    ops.opsData.pop_front();

    // send data
    int sent;
    e = other->Send("ABCD", 4,sent);
    ASSERT_EQ(PBB_ESUCCESS, e); // was able to send

    mgr.Update();

    e = mgr.Close(serverSock); ASSERT_EQ(PBB_ESUCCESS, e);
}