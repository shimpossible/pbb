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
		switch (other.type)
		{
		case 0:
		case 1:
			// copy
			memcpy(this, &other, sizeof(*this));
			break;
		case 2:
			this->type = other.type;
			this->received.data = malloc(other.received.len);
			this->received.len = other.received.len;
			memcpy(this->received.data, other.received.data, other.received.len);
			break;
		}		
	};
	~SocketManagerOps()
	{
		if (type == 2)
		{
			free(received.data);
		}
	}

};

class TestCallbacks : public SocketManager::ISocketCallback
{
public:
    std::list<SocketManagerOps*> opsData;
    void state_changed(pbb::net::Socket* socket, pbb::net::SocketManager::State state)
    {
		SocketManagerOps* ops = new SocketManagerOps();
        ops->type = 0;
        ops->state_changed.socket = socket;
        ops->state_changed.state = state;

        opsData.push_back(ops);
    }
    void accepted(pbb::net::Socket* socket, pbb::net::Socket* remote, pbb::net::SocketAddress& address)
    {
		SocketManagerOps* ops = new SocketManagerOps();
        ops->type = 1;
        ops->accepted.socket = socket;
        ops->accepted.remote = remote;
        ops->accepted.address = address;

        opsData.push_back(ops);
    }
    void received(pbb::net::Socket* socket, void* data, size_t len)
    {
		SocketManagerOps* ops = new SocketManagerOps();
        ops->type = 2;
        ops->received.socket = socket;
        ops->received.data = malloc(len);
        memcpy(ops->received.data, data, len);
        ops->received.len = len;

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
	SocketManagerOps* front = ops.opsData.front();
    ASSERT_EQ(0, front->type);
    ASSERT_EQ(serverSock, front->state_changed.socket);
    ASSERT_EQ(SocketManager::LISTENING, front->state_changed.state);
    ops.opsData.pop_front();
	free(front);

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
	SocketManagerOps* front = opsListen.opsData.front();
    ASSERT_EQ(serverSock, front->state_changed.socket);
    ASSERT_EQ(SocketManager::LISTENING, front->state_changed.state);
    opsListen.opsData.pop_front();
	delete front;

    // verify it has a port
    SocketAddress addr;
    e = serverSock->Address(addr);
    ASSERT_EQ(PBB_ESUCCESS, e);
    ASSERT_NE(0, addr.Port()); // must have a non-zero port

    Socket* client = mgr.ConnectTo("127.0.0.1", addr.Port(), opsConnect);
    ASSERT_NE((Socket*)0, serverSock);

    // check callback was called
    ASSERT_EQ(1, opsConnect.opsData.size());
	front = opsConnect.opsData.front();
    ASSERT_EQ(0, front->type);
    ASSERT_EQ(client, front->state_changed.socket);
    ASSERT_EQ(SocketManager::PENDING_CONNECTION, front->state_changed.state);
    opsConnect.opsData.pop_front();
	delete front;

    // do an update
    mgr.Update();

    // check callback was called, one for ACCEPT and one for CONNECTED
    ASSERT_EQ(1, opsConnect.opsData.size());
    ASSERT_EQ(1, opsListen.opsData.size());
	
	front = opsConnect.opsData.front();
    ASSERT_EQ(client, front->state_changed.socket);
    ASSERT_EQ(SocketManager::CONNECTED, front->state_changed.state);
    opsConnect.opsData.pop_front();
	delete front;

	front = opsListen.opsData.front();
    ASSERT_EQ(1, front->type);
    ASSERT_EQ(serverSock, front->accepted.socket);
    ASSERT_NE(Socket::InvalidSocket, front->accepted.remote);
    opsListen.opsData.pop_front();
	delete front;
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
	SocketManagerOps* front = ops.opsData.front();
	ASSERT_EQ(1, front->type); // accepted
	Socket* clientSocket = front->accepted.remote;
    ops.opsData.pop_front();
	delete front;

    // send data
    int sent;
    e = other->Send("ABCD", 5,sent);
    ASSERT_EQ(PBB_ESUCCESS, e); // was able to send

    mgr.Update();

	// Receved was called
	ASSERT_EQ(1, ops.opsData.size());
	front = ops.opsData.front();
	ASSERT_EQ(2, front->type); // received
	ASSERT_EQ(5, front->received.len);
	ASSERT_STREQ("ABCD", (char*)front->received.data);
	ASSERT_EQ(clientSocket, front->received.socket);
	ops.opsData.pop_front();
	delete front;

    e = mgr.Close(serverSock); ASSERT_EQ(PBB_ESUCCESS, e);
}