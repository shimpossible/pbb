#include <stdio.h>
#include <vector>
#include "pbb/msg/MessageQueue.h"
#include "pbb/msg/TCPTransport.h"
#include "TestProtocol.h"

#include "gtest/gtest.h"

using namespace pbb::msg;

class TCPServerTest : public ::testing::Test {
public:
	std::list<Message*> received;
	static void MsgReceive(void* ctx, Link& link, Message* msg)
	{
		TCPServerTest* self = (TCPServerTest*)ctx;

		self->received.push_back(msg);
		// dont let it get disposed
		msg->AddRef();
	}
};

TEST_F(TCPServerTest, Connect)
{
	MessageHandlerCollection handlers;
	TCPTransport tport(handlers, 0);
	TCPServer server1(tport);
	TCPServer server2(tport);
	bool started = server1.Start(64000);
	server1.Update();
	bool connected = server2.ConnectTo("127.0.0.1", 64000);
	
	for (int i = 0;i < 5;i++)
	{
		server1.Update(); // Complete Connection on Serve end
		server2.Update(); // Complete connection on client end
	}
	
	// Both got sockets
	ASSERT_EQ(true, started);
	ASSERT_EQ(true, connected);

	// Both created connections
	ASSERT_EQ(1, server1.NumberOfConnections());
	ASSERT_EQ(1, server2.NumberOfConnections());
}

TEST_F(TCPServerTest, ConnectionReceive)
{
	MessageQueue queue;
	MessageHandlerCollection handlers;
	handlers.Add(TEST_PROTOCOL::CRC, TEST_PROTOCOL::CreateMessage, this, MsgReceive);

	TCPTransport tport(handlers, 64000);
	ProtocolInfo pinfo("TEST", TEST_PROTOCOL::CRC);
	tport.ConfigureInbound(pinfo);
	
	TCPServer::Connection conn(0, tport);

	// default state
	ASSERT_EQ(TCPServer::Connection::INIT_ID, conn.State());

	// 16 byte id
	conn.Receive("1234123412341234",16);
	ASSERT_EQ(TCPServer::Connection::INIT_PROTOCOL_NAME, conn.State());

	// same name, different crc
	uint8_t buff[] = {
		0x4, 'T','E','S','T', 0xAA,0xBB,0xCC,0xDD
	};
	conn.Receive(buff, 9);
	// Parsed 1 remote protocol
	ASSERT_EQ(1, conn.RemoteProtocols().size());
	// still reading protocols
	ASSERT_EQ(TCPServer::Connection::INIT_PROTOCOL_NAME, conn.State());

	// End of protocols
	buff[0] = 0;
	conn.Receive(buff,1);
	// Now in the connected state
	ASSERT_EQ(TCPServer::Connection::CONNECTED, conn.State());
	
	/*
	Size        1 - 4 bytes
	Channel     1 byte
	Msg Type    1 byte
	Message Id  2 byte
	Payload     n bytes
	*/

	uint8_t msgBuff[] = {
		0x8, 
		1, // channel
		0, // type
		2, // msg id
		0, // msg id
		// payload
		0x1,0x2,0x3,0x4
	};
	conn.Receive(msgBuff, sizeof(msgBuff));

	ASSERT_EQ(1, received.size());
	Message* msg = received.front();
	received.pop_front();

	ASSERT_EQ(2, msg->GetCode());
	ASSERT_EQ(TEST_PROTOCOL::CRC, msg->GetProtcolCRC());
	TestMessage* tst_msg = (TestMessage*)msg;
	ASSERT_EQ(0x04030201, tst_msg->data);
	msg->Release();

}