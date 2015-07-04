#include <stdio.h>

#include "pbb/net/Socket.h"
#include "EchoServer.h"

#include "gtest/gtest.h"

#pragma comment( lib, "ws2_32.lib" ) // linker must use this lib for sockets

using namespace pbb;
using namespace pbb::net;

class SocketTest : public ::testing::Test {
public:
};

TEST_F(SocketTest, Echo)
{
    EchoServer srv;
    Thread thred;
	printf("Trying to start\r\n");
    thred.Start(srv);   

    srv.Stop();
    thred.Join();
}

/**
    Open a socket and make a connection,
    also test an invalid connection
 */
TEST_F(SocketTest, AsyncConnect)
{
    pbb::net::Error result;
    //////////////////////////
    // setup
    SocketAddress addr(SocketAddress::INET, "127.0.0.1", 0);
    Socket* client = Socket::Create(addr.AddressFamily(), Socket::TCP, IPPROTO_TCP);
    Socket* server = Socket::Create(addr.AddressFamily(), Socket::TCP, IPPROTO_TCP);

    // setup a listening socket
    result = server->Bind(addr, true);
    ASSERT_EQ(PBB_ESUCCESS, result);

    // get port server is listening on
    SocketAddress servAddr;
    server->Address(servAddr);

    // non-blocking
    client->SetBlocking(false);

    // Connect to non-listening server as async
    result = client->Connect(servAddr);
    int ready = 0;

    SocketCollection rc;
    SocketCollection wc;
    SocketCollection ec;
    rc.push_back(client);
    wc.push_back(client);
    ec.push_back(client);

    // should fail
    Error serr = Socket::Select(&rc, &wc, &ec, 10000000, ready);
    ASSERT_EQ(1, ec.size() + wc.size());

    // no one listening
    result = client->GetError();
    ASSERT_EQ(PBB_ECONNREFUSED, result);

    client->Close();
    server->Close();
}

/**
Open a socket and make a connection,
also test an invalid connection
*/
TEST_F(SocketTest, SyncConnect)
{
    pbb::net::Error result;
    //////////////////////////
    // setup
    SocketAddress addr(SocketAddress::INET, "127.0.0.1", 0);
    Socket* client = Socket::Create(addr.AddressFamily(), Socket::TCP, IPPROTO_TCP);
    Socket* server = Socket::Create(addr.AddressFamily(), Socket::TCP, IPPROTO_TCP);

    // setup a listening socket
    result = server->Bind(addr, true);
    ASSERT_EQ(PBB_ESUCCESS, result);

    result = server->Listen(10);
    ASSERT_EQ(PBB_ESUCCESS, result);

    // get port server is listening on
    SocketAddress servAddr;
    server->Address(servAddr);

    // normal blocking (Default)
    //client.SetBlocking(true);

    // Connect to non-listening server as async
    result = client->Connect(servAddr);

    int ready = 0;

    SocketCollection rc;
    SocketCollection wc;
    SocketCollection ec;
    rc.push_back(client);
    wc.push_back(client);
    ec.push_back(client);

    // should fail
    Error serr = Socket::Select(&rc, &wc, &ec, 10000000, ready);
    ASSERT_EQ(0, ec.size());
    ASSERT_EQ(1, wc.size());

    // connect went well
    result = client->GetError();
    ASSERT_EQ(PBB_ESUCCESS, result);

    Socket* clientSock = 0;
    result = server->Accept(clientSock);
    ASSERT_EQ(PBB_ESUCCESS, result);

}
