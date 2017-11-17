#include "pbb/msg/TCPTransport.h"
#include "pbb/msg/TCPTransportProtocol.h"
#include "pbb/msg/RouteConfig.h"

namespace pbb {
namespace msg {


TCPTransport::TCPTransport(RouteConfig& config, uint16_t port)
	: mRouteConfig(config)
	, mServer(*this)
{
	// TODO: move actual start somewhere else.  Constructor is only for initializing defaults
	mServer.Start(port);
	
	mRouteConfig.ConfigureInbound<TCPTransportProtocol>(this, &TCPTransport::DispatchMessage);
}

void TCPTransport::DispatchMessage(void* ctx, Link& link, Message* msg)
{
	TCPTransport* self = (TCPTransport*)ctx;
	printf("Received: %d\r\n", msg->GetCode());
	switch (msg->GetProtcolCRC())
	{
	case TCPTransportProtocol::CRC:
		TCPTransportProtocol::Dispatch<TCPTransport>(msg, self);
		break;
	default:
		// unknonw message
		break;
	}
}

void TCPTransport::Transmit(Link& link, Message* msg)
{
	// Loop through all clients that accept the protocol
	msg->GetProtcolCRC();
}

void TCPTransport::Receive(Link& link, Message * msg)
{
	mRouteConfig.MessageHandlers().Dispatch(link, msg);
	//mHandlers.Dispatch(link, msg);
}

void TCPTransport::Update()
{
	mServer.Update();
}

void TCPTransport::ConfigureOutbound(ProtocolInfo* info)
{
}

void TCPTransport::ConfigureInbound(ProtocolInfo* info)
{
}

pbb::msg::ProtocolInfo* TCPTransport::FindInbound(ProtocolInfo& info)
{
	ProtocolInfoCollection::iterator it;
	// Find Protocol by Name
	for (it = mInboundProtocols.begin();
	it != mInboundProtocols.end();
		it++)
	{
		if (strcmp(it->Name, info.Name) == 0) return &*it;
	}
	return 0;
}

TCPServer::TCPServer(TCPTransport& transport)
	: m_Transport(transport)
	, m_SocketCallback(*this)
{
}

void TCPServer::Update()
{
	m_SocketMgr.Update();
}

bool TCPServer::Start(uint16_t port)
{
	m_Socket = m_SocketMgr.OpenAndListen(port, m_SocketCallback);

	return m_Socket != net::Socket::InvalidSocket;
}

bool TCPServer::ConnectTo(const char* address, uint16_t port)
{
	// connect to remote server given an address and port, since the protocol is
	// symetric, tree this as another client
	net::SocketAddress addr(net::SocketAddress::INET, address, port);

	// ConnectTo will call the 'state_changed' call back when a connection happens
	net::Socket* socket = m_SocketMgr.ConnectTo(addr, m_SocketCallback);
	return socket != net::Socket::InvalidSocket;
}

TCPServer::Connection * TCPServer::FindClient(net::pbb_socket_t id)
{
	ClientSocketMap::iterator it = m_KnownConnections.find(id);
	if (it != m_KnownConnections.end()) return it->second;

	return nullptr;
}

void TCPServer::AddPending(net::pbb_socket_t id, TCPServer::Connection* client)
{
	m_KnownConnections[id] = client;

	// sends hello (16byte id)
	client->SendHello();
}

void TCPServer::Callbacks::state_changed(pbb::net::Socket * socket, net::SocketManager::State state)
{
	switch (state)
	{
	case net::SocketManager::CONNECTED: // Connected to remote server..
		// Treat this as another client connection
		Connection* client = new Connection(socket, mServer.m_Transport, mServer, false);
		mServer.AddPending(*socket, client);
		break;
	}
}

void TCPServer::Callbacks::accepted(pbb::net::Socket * socket, pbb::net::Socket* remote, pbb::net::SocketAddress& address)
{
	// new client connects to server
	Connection* client = new Connection(remote, mServer.m_Transport, mServer, true);
	mServer.AddPending(*remote, client);
}

void TCPServer::Callbacks::received(pbb::net::Socket * socket, void * data, size_t len)
{
	Connection* client = mServer.FindClient(*socket);
	if (client != nullptr)
	{
		client->Receive(data, len);
	}
}


size_t TCPServer::NumberOfConnections()
{
	return m_KnownConnections.size();
}

void TCPServer::OnHelloReceived(Connection * connection, ObjectId & id)
{
	// TODO: associate ID with connection
	printf("Connection %p remote id = %016llx %016llx\r\n", connection, id.first, id.second);
	if (connection->IsServer())
	{
		// Server Requests encoders from client
		connection->BeginRequestEncoders();
	}
}


uint32_t Encoded7BitBytes(uint32_t val)
{
	if (val >= (0x80 * 0x80 * 0x80)) return 4;
	if (val >= (0x80 * 0x80)) return 3;
	if (val >= 0x80) return 2;
	return 1;
}

PBB_API bool Encode7Bit(DataChain& buff, size_t val, bool prefix=false)
{
	uint32_t ret = 0;
	unsigned char* ptr = (unsigned char*)&ret;
    size_t bytes = 1;
	while (val > 0x80)
	{
		*ptr++ = (val | 0x80) & 0xFF;
		val = val >> 7; // shift off the lower 7 bits
		bytes++;
	}
	// val should be less than 0x80 at this point
	*ptr = (unsigned char)val;

	if (bytes <= buff.Available())
	{
		if (prefix)  buff.AddHead(&ret, bytes);
		else         buff.AddTail(&ret, bytes);
		
		return true;
	}
	return false;
}

/**
7 bit integer encoding, LSByte first
*/
PBB_API uint32_t Encode7Bit(uint32_t val, int& bytes)
{
	uint32_t ret = 0;
	unsigned char* ptr = (unsigned char*)&ret;
	bytes = 1;
	while (val > 0x80)
	{
		*ptr++ = (val | 0x80) & 0xFF;
		val = val >> 7; // shift off the lower 7 bits
		bytes++;
	}
	// val should be less than 0x80 at this point
	*ptr = (unsigned char)val;
	return ret;
}

/**
	Decode 7bit encoded integer
	@param buff Buffer of data
	@param len  Number of bytes of data
	@param val  Decoded integer
	@returns pointer to input buffer, after decoded bytes.  If nothing decoded, returns original buffer
	*/
PBB_API uint8_t* Decode7Bit(uint8_t* buff, uint32_t len, uint32_t& val)
{
	uint8_t* ret = buff;

	if (len > 4) len = 4;

	val = 0;
	// at most 4 bytes
	for (uint32_t i = 0;i < len;i++)
	{
		ret++;
		val += (ret[-1] & 0x7F) << (i * 7);
		if (ret[-1] < 0x80)
		{
			break;
		}
	}

	// not enough bytes
	if (len == 0 || ret[-1] & 0x80) return buff;
	return ret;
}

/*
	@returns TRUE if correctly parsed integer
	*/
PBB_API bool Decode7Bit(DataChain& chain, uint32_t& val)
{
	uint8_t* ret = chain.GetBuffer();
    size_t len = chain.Size();
    size_t i;
	if (len > 4) len = 4;

	val = 0;
	// at most 4 bytes
	for (i = 0;i < len;i++)
	{
		ret++;
		val += (ret[-1] & 0x7F) << (i * 7);
		if (ret[-1] < 0x80)
		{
			break;
		}
	}

	// not enough bytes
	if (len == 0 || ret[-1] & 0x80) return false;

	// Remove used bytes
	chain.Shift(i+1);
	return true;
}

PBB_API uint32_t Decode7Bit(uint8_t* val)
{
	uint32_t ret = 0;

	// at most 4 bytes
	for (int i = 0;i < 4;i++)
	{
		ret += (*val & 0x7F) << (i * 7);
		if (*val < 0x80)
		{
			break;
		}
	}
	return ret;
}

ProtocolInfo * TCPServer::Connection::GetProtocolForChannel(uint32_t chan)
{
	ChannelMapCollection::iterator it = mChannelMap.find(chan);
	if (it != mChannelMap.end()) return it->second;
	return nullptr;
}

bool TCPServer::Connection::GetChannelForProtocol(uint32_t proto, uint8_t& chan)
{
	bool result = false;
	if (proto == TCPTransportProtocol::CRC)
	{
		chan = 0;
		result = true;
	}
	else
	{
		ProtocolToChannelCollection::iterator it = mProtocolMap.find(proto);
		if (it != mProtocolMap.end())
		{
			chan = it->second;
			result = true;
		}
	}
	return result;
}

void TCPServer::Connection::BeginRequestEncoders()
{
	// Loop through each remote protocol
	mProtoIter = mRemoteProtocols.begin();
	if(mProtoIter != mRemoteProtocols.end())
	{
		EncoderRequest req;
		// Request encoders from remote system
		req.ProtocolCRC = mProtoIter->CRC;
		Send(req);
	}

}

void TCPServer::Connection::IncomingProtocolCallback(ProtocolInfo* info)
{
	pbb::msg::AddProtocol msg;
	msg.Name        = info->Name;
	msg.ProtocolCRC = info->CRC;
	// List of supported Encoders
	//msg.Encoders    = info->Encoders;
	Send(msg);
}

void TCPServer::Connection::SendInboundProtocols()
{
	mTransport.mRouteConfig.ForEachIncommingProtocols(&Connection::IncomingProtocolCallback, this);
}
void TCPServer::Connection::SendOutboundProtocols()
{

}
#define TASK_START(x) switch(x){ case -1:

/* exit one loop */
#define TASK_YIELD(x) x = __LINE__; return; case __LINE__:

/* block until Y is true */
#define TASK_UNTIL(x, y) x = __LINE__; case __LINE__: if(!(y)) return;

/* Block while Y is true */
#define TASK_WHILE(x, y) TASK_UNTIL(x,!(y))

#define TASK_END(x)   x = __LINE__; case __LINE__: default: break; }


TCPServer::Connection::ProtocolInfoCollection::iterator protoItr;

void TCPServer::Connection::SendHello()
{
	Send(&mId, sizeof(mId) );

	if (mIsServer)
	{
		SendInboundProtocols();
	}
	else
	{
		SendOutboundProtocols();
	}
}

/**
Call when new data arives for client
@param src  Pointer to buffer of new data
@param len  Number of bytes in buffer
*/
void TCPServer::Connection::Receive(const void* src, size_t len)
{
	uint8_t *src8 = (uint8_t*)src;
	printf("%p >> ", this);
	for (size_t i = 0;i < len;i++)
	{
		printf("%02x ", src8[i]);
	}
	printf("\r\n");
	this->mReceiveBuffer.AddTail(src, len);
	ReceiveHello();

	if (mState == STATE_CONNECTED)
	{
		TASK_START(mReceiveMessageState)
		while (true)
		{
			/*
			Size        1-4 bytes
			Channel     1 byte
			Msg Type    1 byte
			Message Id  2 byte
			Payload     n bytes
			*/

			TASK_UNTIL(mReceiveMessageState, Decode7Bit(mReceiveBuffer, nameLength))
			TASK_UNTIL(mReceiveMessageState, mReceiveBuffer.Size() >= (nameLength))

			{
				// Read Message Header
				// TODO: use BinaryDecoder for endianess
				MessageHeader header;
				mReceiveBuffer.Shift(&header, sizeof(header));
				uint32_t payloadSize = nameLength - sizeof(header);

				uint32_t protocolCrc;
				ProtocolInfo* info;
				if (header.ChannelId == 0)
				{
					info = TCPTransportProtocol::Info();
				}
				else
				{
					info = GetProtocolForChannel(header.ChannelId);
				}
				if (info != nullptr)
				{
					protocolCrc = info->CRC;

					// Which decoder to use
					//ProtocolEncodingMap::iterator protItr = mProtocolEncoding.find(protocolCrc);
					// no match..
					//if (protItr == mProtocolEncoding.end()) break;
					//uint32_t decoder = protItr->second;

					// FIXME: assume binary decoder for now
					uint32_t decoder = BinaryDecoder::ID;

					// Create message
					Message* msg = this->mTransport.CreateMessage(protocolCrc, header.MessageId);

					// Decode
					if (msg == nullptr || // bad messageId
						msg->Get(mReceiveBuffer, decoder) == false // error in decode
						)
					{
						// TODO: error decoding..
					}
					else
					{
						// TODO: route message to server first..
						// route message
						mTransport.Receive(mLink, msg);

					}
					msg->Release();
				}
				else
				{
					// TODO: err Unknown channel..
				}
			}
		} // while true

		TASK_END(mReceiveMessageState)
	}
}

void TCPServer::Connection::ReceiveHello()
{
	TASK_START(mReceiveState)
    // Wait until we have enough for 16byte ID
	TASK_UNTIL(mReceiveState, mReceiveBuffer.Size() >= 16)	
	mReceiveBuffer.Shift(&mId, 16);

#if 0
	do{
		// Wait for enough bytes to read length of name
		TASK_UNTIL(mReceiveState, Decode7Bit(mReceiveBuffer, nameLength))
		
		if (nameLength==0) break;

		/* enough for Name and 4byte CRC */
		TASK_UNTIL(mReceiveState, mReceiveBuffer.Size() >= (nameLength + 4))

		name = new char[nameLength + 1];
		// copy name from buffer
		memcpy(name, mReceiveBuffer.GetBuffer(), nameLength);
		name[nameLength] = 0; // null terminate
		memcpy(&crc, mReceiveBuffer.GetBuffer() + nameLength, 4);
		mReceiveBuffer.Shift(nameLength + 4);

		AddProtocol(name, crc);
	} while (nameLength != 0);
#endif

	// once id and All protcols have been received
	mServer.OnHelloReceived(this, mId);

	mState = STATE_CONNECTED;

	TASK_END(mReceiveState)

}

void TCPServer::Connection::AddProtocol(const char* name, uint32_t crc)
{
	ProtocolInfo pi(name, crc);
	// TODO: limit the total number of protocols?
	mRemoteProtocols.push_back(pi);

	ProtocolInfo* local = mTransport.FindInbound(pi);
	// We have a known match?
	if (local != nullptr)
	{
		uint8_t channel = mChannelMap.size() + 1;
		mChannelMap[channel] = local;
		mProtocolMap[local->CRC] = channel;
	}
	else
	{
		// no match yet, store to match later
	}
}

void TCPServer::Connection::Send(const void * src, size_t len)
{
	int sent = 0;
	uint8_t* src8 = (uint8_t*)src;
	printf("%p << ", this);
	for (int i = 0;i < len;i++)
	{
		printf("%02x ", src8[i]);
	}
	printf("\r\n");
	// TODO: ensure all data is sent..
	net::Error err = mSocket->Send(src, len, sent);
}

void TCPServer::Connection::Send(pbb::msg::Message& msg)
{
	uint32_t protocol = msg.GetProtcolCRC();

	mTransmitBuffer.Reset();

	// Channel
	// MsgType
	// MsgId
	// -----------
	// Payload

	// Find Channel for CRC
	MessageHeader header;
	header.MsgType = 0;
	if (GetChannelForProtocol(msg.GetProtcolCRC(), header.ChannelId))	
	{
		header.MessageId = msg.GetCode();
		mTransmitBuffer.AddTail(&header, sizeof(header));

		msg.Put(mTransmitBuffer, BinaryEncoder::ID);

		// Prefix with length
		Encode7Bit(mTransmitBuffer, mTransmitBuffer.Size(), true);

		Send(mTransmitBuffer.GetBuffer(), mTransmitBuffer.Size());
	}

}

bool TCPServer::Connection::ProcessReconcile()
{
	// TODO: loop through Server Protocols and match against remote
	// right now this always does remote (even if remote is server)

	// Loop through each Remote protocol and see which one we match
	ProtocolInfoCollection::iterator it;
	for (it = mRemoteProtocols.begin();
         it != mRemoteProtocols.end();
		 it++)
	{
		// Find a local Inbound Protocol by name
		ProtocolInfo* local = mTransport.FindInbound(*it);
		if (local != nullptr)  // found a match
		{
			mChannelMap[mChannelMap.size() + 1] = local;
		}
		else
		{
			// No match, ignore
		}
	}
	return true;
}

bool TCPServer::Connection::ProcessResolveEncoders()
{
	bool loop = false;

	ChannelMapCollection::iterator it;
	it = mChannelMap.begin();
	if (it != mChannelMap.end())
	{
		uint32_t channelId = it->first;

	}	

	return loop;
}

void TCPTransport::AddProtocolHandler(AddProtocol* msg)
{

}

const char* EncoderResponse::NAME = "EncoderResponse";
const char* EncoderRequest::NAME = "EncoderRequest";

} // namespace msg
} // namespace pbb