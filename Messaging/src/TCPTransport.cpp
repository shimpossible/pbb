#include "pbb/msg/TCPTransport.h"
#include "pbb/msg/RouteConfig.h"

namespace pbb {
namespace msg {

TCPTransport::TCPTransport(MessageHandlerCollection& handlers, uint16_t port)
	: mHandlers(handlers)
{
    // TODO: open socket listening for new connections
    // start Receive Thread
}

void TCPTransport::Transmit(Link& link, Message* msg)
{
    // Loop through all clients that accept the protocol
    msg->GetProtcolCRC();
}

void TCPTransport::Receive(Link& link, Message * msg)
{
	mHandlers.Dispatch(link, msg);
}

void TCPTransport::ConfigureOutbound(ProtocolInfo& info)
{
    mOutboundProtocols.push_back(info);
}

void TCPTransport::ConfigureInbound(ProtocolInfo& info)
{
    mInboundProtocols.push_back(info);
}

pbb::msg::ProtocolInfo* TCPTransport::FindInbound(ProtocolInfo& info)
{
	ProtocolInfoCollection::iterator it;
	// Find Protocol by Name
	for (it = mInboundProtocols.begin();
	     it != mInboundProtocols.end();
		 it++)
	{
		if (strcmp(it->m_Name, info.m_Name) == 0) return &*it;
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

void TCPServer::AddPending(net::pbb_socket_t id, TCPServer::Connection * client)
{
	m_KnownConnections[id] = client;

	//TODO: Send connection request
}

void TCPServer::Callbacks::state_changed(pbb::net::Socket * socket, net::SocketManager::State state)
{
	switch (state)
	{
	case net::SocketManager::CONNECTED: // Connected to remote server..
		// Treat this as another client connection
		Connection* client = new Connection(socket, mServer.m_Transport);
		mServer.AddPending(*socket, client);
		break;
	}
}

void TCPServer::Callbacks::accepted(pbb::net::Socket * socket, pbb::net::Socket* remote, pbb::net::SocketAddress& address)
{
	// new client connects to server
	Connection* client = new Connection(remote, mServer.m_Transport);
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


uint32_t TCPServer::NumberOfConnections()
{
	return m_KnownConnections.size();
}


uint32_t Encoded7BitBytes(uint32_t val)
{
	if (val >= (0x80 * 0x80 * 0x80)) return 4;
	if (val >= (0x80 * 0x80)) return 3;
	if (val >= 0x80) return 2;
	return 1;
}

/**
7 bit integer encoding, LSByte first
*/
uint32_t Encode7Bit(uint32_t val, int& bytes)
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
uint8_t* Decode7Bit(uint8_t* buff, uint32_t len, uint32_t& val)
{
	uint8_t* ret = buff;

	if (len > 4) len = 4;

	// at most 4 bytes
	for (uint32_t i = 0;i<len;i++)
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

uint32_t Decode7Bit(uint8_t* val)
{
	uint32_t ret = 0;

	// at most 4 bytes
	for (int i = 0;i<4;i++)
	{
		ret += (*val & 0x7F) << (i * 7);
		if (*val < 0x80)
		{
			break;
		}
	}
	return ret;
}

bool TCPServer::Connection::ProcessInitName()
{
	bool loop = false;
	do
	{
		loop = false;
		uint32_t len = 0;
		uint32_t size = mReceiveBuffer.Size();
		uint8_t* buff = mReceiveBuffer.GetBuffer();
		uint8_t* buff2 = Decode7Bit(buff, size, len);
		int needed = buff2 - buff;
		if (buff != buff2) // enough for length
		{
			// get length of string
			if (len == 0)
			{
				// no more protocols
				mState = INIT_RECONCILE;
				loop = true;
				mReceiveBuffer.Shift(1);
				break; // break out of do loop
			}
			else
			{
				// have enough for name and CRC?
				if (size >= (needed + len + 4))
				{
					char* name = new char[len + 1];
					uint32_t crc;
					// copy name from buffer
					memcpy(name, mReceiveBuffer.GetBuffer() + needed, len);
					name[len] = 0; // null terminate
					memcpy(&crc, mReceiveBuffer.GetBuffer() + needed + len, 4);
					AddProtocol(name, crc);

					mReceiveBuffer.Shift(len + needed + 4);
					loop = true;
				}
			}
		}
	} while (loop);
	return loop;
}

ProtocolInfo * TCPServer::Connection::GetProtocolForChannel(uint32_t chan)
{
	ChannelMapCollection::iterator it = mChannelMap.find(chan);
	if (it != mChannelMap.end()) return it->second;
	return nullptr;
}

/**
Call when new data arives for client
@param src  Pointer to buffer of new data
@param len  Number of bytes in buffer
*/
void TCPServer::Connection::Receive(const void* src, size_t len)
{
	this->mReceiveBuffer.AddTail(src, len);

	bool loop = false;
	do
	{
		loop = false;
		switch (mState)
		{
		case INIT_ID:            loop = ProcessInitId();  break;
		case INIT_PROTOCOL_NAME: loop = ProcessInitName(); break;
		case INIT_RECONCILE:     loop = ProcessReconcile(); break;
		case CONNECTED:          loop = ProcessReceiveMessage(); break;
		}
	} while (loop);
}
/**
   Size        1-4 bytes
   Channel     1 byte
   Msg Type    1 byte
   Message Id  2 byte
   Payload     n bytes
 */
bool TCPServer::Connection::ProcessReceiveMessage()
{
	bool loop = false;
	do
	{
		loop = false;
		uint32_t len = 0;
		uint8_t* buff = mReceiveBuffer.GetBuffer();
		uint8_t* buff2 = Decode7Bit(buff, mReceiveBuffer.Size(), len);
		int encodedBytes = buff2 - buff;
		if (buff != buff2)
		{
			if (mReceiveBuffer.Size() >= encodedBytes + len)
			{
				// shift of 7bit encoded number
				mReceiveBuffer.Shift(encodedBytes);
				
				// Read Message Header
				// TODO: use BinaryDecoder for endianess
				MessageHeader header;
				mReceiveBuffer.Shift(&header, sizeof(header));
				uint32_t payloadSize = len - sizeof(header);

				uint32_t protocolCrc;
				//TODO: Get protocol for channel id
				ProtocolInfo* info = GetProtocolForChannel(header.ChannelId);
				if (info != nullptr)
				{
					protocolCrc = info->m_CRC;

					// Which decoder to use
					uint32_t decoder = mProtocolEncoding.find(protocolCrc)->second;

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
						// route message
						mTransport.Receive(mLink, msg);

					}
				}
				else
				{
					// TODO: err Unknown channel..
				}
			}
		}
	} while (loop);
	return loop;
}

bool TCPServer::Connection::ProcessReconcile()
{
	// Loop through each Remote protocol and see which one we match
	ProtocolInfoCollection::iterator it;
	for (it = mRemoteProtocols.begin();
         it != mRemoteProtocols.end();
		 it++)
	{
		// Find a local Inbound Protocol by name
		ProtocolInfo* local = mTransport.FindInbound(*it);
		if (local != nullptr)
		{
			if (local->m_CRC == it->m_CRC)  // exactly match, use highest prefered encoding
			{
				mProtocolEncoding[local->m_CRC] = mBestEncoding;
			}
			else // Not exact match, use best fallback encoding
			{
				mProtocolEncoding[local->m_CRC] = mFallbackEncoding;
			}

			mChannelMap[mChannelMap.size() + 1] = local;
		}
		else
		{
			// No match, ignore
		}
	}
	mState = CONNECTED;
	return true;
}

} // namespace msg
} // namespace pbb