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

TEST_F(TCPServerTest, DISABLED_Connect)
{
	//MessageHandlerCollection handlers;
	RouteConfig routeConfig;
	TCPTransport tport(routeConfig, 0);
	TCPServer server1(tport);
	TCPServer server2(tport);

	// TODO: make this a random port..
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
	pbb::net::Error socketStatus;
	MessageQueue queue;
	MessageHandlerCollection handlers;
	RouteConfig routeConfig;
	routeConfig.ConfigureInbound<TEST_PROTOCOL>(this, MsgReceive);

	TCPTransport tport(routeConfig, 0);

	//ProtocolInfo pinfo("TEST", TEST_PROTOCOL::CRC);
	//tport.ConfigureInbound(&pinfo);


	// Connect to self..
	pbb::net::SocketAddress addr1;
	socketStatus = tport.Server().Address(addr1);
	ASSERT_EQ(pbb::net::PBB_ESUCCESS, socketStatus);

	bool connected = tport.Server().ConnectTo("127.0.0.1", addr1.Port() );
	ASSERT_EQ(true, connected);
	
	for (int i = 0;i < 10;i++)
	{
		tport.Update();
		tport.Update();
	}
	ASSERT_EQ(2, tport.Server().NumberOfConnections());

	for (;;)
		tport.Update();


	TCPServer::Connection conn(0, tport, tport.Server(), true);

	// default state
	ASSERT_EQ(TCPServer::Connection::STATE_HELLO, conn.State());

	// 16 byte id
	conn.Receive("1234123412341234",16);
	//ASSERT_EQ(TCPServer::Connection::STATE_INIT_PROTOCOL_NAME, conn.State());
	// TODO: assert ID was parsed

	// same name, different crc
	uint8_t buff[] = {
		0x4, 'T','E','S','T', 0xAA,0xBB,0xCC,0xDD
	};
	conn.Receive(buff, sizeof(buff) );
	// Parsed 1 remote protocol
	ASSERT_EQ(1, conn.RemoteProtocols().size());
	// still reading protocols
	//ASSERT_EQ(TCPServer::Connection::STATE_INIT_PROTOCOL_NAME, conn.State());

	// End of protocols
	buff[0] = 0;
	conn.Receive(buff,1);
	// Now in the connected state
	//ASSERT_EQ(TCPServer::Connection::STATE_CONNECTED, conn.State());

	pbb::DataChain msgBuff(4, 100);
	BinaryEncoder msgEncode(msgBuff);
	char chan = 0;
	char type = 0;

	chan = 0;
	type = 0;
	msgEncode.Put("channel", chan);
	msgEncode.Put("msgtype", type);
	msgEncode.Put("id", EncoderResponse::CODE);

	EncoderResponse encoderResponse;
	encoderResponse.Encoders[0] = 0;

	encoderResponse.Put(msgEncode);

	int bytes;
	uint32_t size7 = Encode7Bit(msgBuff.Size(), bytes);
	msgBuff.AddHead(&size7, bytes);
	conn.Receive(msgBuff.GetBuffer(), msgBuff.Size());

	// Server requests for list of Encoders
	
	/*
	Size        1 - 4 bytes
	Channel     1 byte
	Msg Type    1 byte
	Message Id  2 byte
	Payload     n bytes
	*/
	/*
	uint8_t msgBuff[] = {
		0x8, 
		1, // channel
		0, // type
		2, // msg id
		0, // msg id
		// payload
		0x1,0x2,0x3,0x4
	};
	*/

	msgBuff.Reset();

	chan = 1;
	type = 0;
	msgEncode.Put("channel",chan); // channel
	msgEncode.Put("msgtype",type); // type

	TestMessage msgOrig;
	msgOrig.data = 0x04030201;
	msgOrig.Put(msgEncode);

	size7 =  Encode7Bit(msgBuff.Size(), bytes);
	msgBuff.AddHead(&size7, bytes);
	

	conn.Receive(msgBuff.GetBuffer(), msgBuff.Size() );

	ASSERT_EQ(1, received.size());
	Message* msg = received.front();
	received.pop_front();

	ASSERT_EQ(2, msg->GetCode());
	ASSERT_EQ(TEST_PROTOCOL::CRC, msg->GetProtcolCRC());
	TestMessage* tst_msg = (TestMessage*)msg;
	ASSERT_EQ(0x04030201, tst_msg->data);
	msg->Release();

}