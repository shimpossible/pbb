#ifndef __PBB_TCP_TRANSPORT_H__
#define __PBB_TCP_TRANSPORT_H__

//#include "RouteConfig.h"
#include <pbb/net/Socket.h>
#include <pbb/net/SocketManager.h>
#include <pbb/DataChain.h>

#include "Link.h"
#include "ITransport.h"
#include "Message.h"
#include "MessageHandlerCollection.h"
#include <vector>
#include <unordered_map>


namespace pbb {
namespace msg {

class TCPServer;
class TCPTransport;

/**
7 bit integer encoding, LSByte first
*/
uint32_t PBB_API Encode7Bit(uint32_t val, int& bytes);
uint32_t PBB_API Decode7Bit(uint8_t* val);

/**
  Bytes needed to encode number
 */
uint32_t PBB_API Encoded7BitBytes(uint32_t val);
class PBB_API TCPServer
{
public:

	enum MsgType
	{
		MSG_TYPE_NORMAL,
		MSG_TYPE_OBJECT,
		MSG_TYPE_PING,
	};
	struct MessageHeader
	{
		uint8_t  ChannelId;
		uint8_t  MsgType;
		uint16_t MessageId;
	};

	class PBB_API Connection
	{
	public:

		typedef std::vector<ProtocolInfo> ProtocolInfoCollection;
		typedef std::map<uint32_t, uint32_t> ProtocolEncodingMap;

		enum State
		{
			INIT_ID,
			INIT_PROTOCOL_NAME,
			INIT_PROTOCOL_CRC,
			INIT_RECONCILE,
			CONNECTED
		};

		Connection(pbb::net::Socket* socket, TCPTransport& transport)
			: mReceiveBuffer(2048, 0)
			, mTransport(transport)
			, mState( INIT_ID )
		{
		}

		~Connection()
		{

		}

		enum State State()
		{
			return mState;
		}

		/**
		Call when new data arives for client
		@param src  Pointer to buffer of new data
		@param len  Number of bytes in buffer
		*/
		void Receive(const void* src, size_t len);

		bool ProcessReceiveMessage();

		/**
		  Given a list of remote protocols, go through them a decide which ones to keep
		 */
		bool ProcessReconcile();
		bool ProcessInitId()
		{
			bool loop = false;
			if(mReceiveBuffer.Size() >= 16)
			{
				mReceiveBuffer.Shift(&mId, 16);
				mState = INIT_PROTOCOL_NAME;
				loop = true;
			}
			return loop;
		}

		bool ProcessInitName();

		void AddProtocol(const char* name, uint32_t crc)
		{
			// TODO: limit the total number of protocols (to save memory)
			mRemoteProtocols.push_back(ProtocolInfo(name, crc));
		}

		ProtocolInfoCollection& RemoteProtocols() { return mRemoteProtocols;  }

		ProtocolInfo* GetProtocolForChannel(uint32_t chan);
	protected:
		typedef std::map<uint32_t, ProtocolInfo*> ChannelMapCollection;
		enum State mState;
		DataChain  mReceiveBuffer;
		ObjectId   mId;
		TCPTransport& mTransport;
		// Protocol info received from Remote connection
		ProtocolInfoCollection mRemoteProtocols;
		ChannelMapCollection   mChannelMap;
		// Quick lookup when sending
		ProtocolEncodingMap    mProtocolEncoding;

		uint32_t   mBestEncoding;
		uint32_t   mFallbackEncoding;

		Link       mLink;
	};

    TCPServer(TCPTransport& transport);

	void Update();
    bool Start(uint16_t port);
    bool ConnectTo(const char* address, uint16_t port);

    /**
    Sends message to specific object on remote server
    @param server Remote service to send to
    @param object id of object on remote server
    @param msg    message to send
    */
    //int32_t Send(JellyID server, ObjectID objectId, Message* msg);

    /**
    Sends to a specfic service/server
    */
    int32_t Send(Link& link, Message* message);

	uint32_t NumberOfConnections();
protected:

	/**
        There are a few layers of IDs.
        Server   -> Remote Server, contains 1 or more Endpoints
        Endpoint -> A single object on a remote server
        This keeps track of which connection an object is found on
        */
    typedef std::unordered_map<ObjectId, Connection*> ConnectionMap;
    /**
     Map a socket to a connection object
     */
    typedef std::unordered_map<net::pbb_socket_t, Connection*> ClientSocketMap;

	class Callbacks : public net::SocketManager::ISocketCallback
	{
	public:
		Callbacks(TCPServer& parent)
		: mServer( parent)
		{
			
		}
		virtual void state_changed(pbb::net::Socket* socket, net::SocketManager::State state);
		virtual void accepted(pbb::net::Socket* socket, pbb::net::Socket* remote, pbb::net::SocketAddress& address);
		virtual void received(pbb::net::Socket* socket, void* data, size_t len);

		TCPServer& mServer;
	};

	TCPTransport&              m_Transport;
	Callbacks                  m_SocketCallback;
	net::SocketManager         m_SocketMgr;
    net::Socket*               m_Socket;
    ClientSocketMap            m_KnownConnections;

    Connection* FindClient(net::pbb_socket_t id);
    /**
    A new client that hasn't negotiated protocols yet..
    */
    void AddPending(net::pbb_socket_t id, Connection* client);
};

/**
Transports the message to/from Remote endpoints
*/
class PBB_API TCPTransport : public ITransport, IMessagePool
{
public:
    /**
    New TCPTransport
	@param handlers  Callbacks on a received message
    @param port  TCP port to listen on
    */
    TCPTransport(MessageHandlerCollection& handlers, const uint16_t port);

    /**
    Called by RouteConfig when sending a message.  To Transmit
    outgoing message to all local endpoints
	@param link Who sent the message
	@param msg  Message to send
    */
    virtual void Transmit(Link& link, Message* msg);
	void Receive(Link& link, Message* msg);

    /**
    Notify transport of Protocols that will be sent
    RouteConfig will call this whenever RouteConfig::ConfigureOutbound is called
    */
    virtual void ConfigureOutbound(ProtocolInfo& info);

    virtual void ConfigureInbound(ProtocolInfo& info);

    /////////////////////////////////////////
    // IMessagePool functions

    virtual Message* CreateMessage(uint32_t protocol, uint32_t code) 
	{ 
		Message* message = 0;
		std::map<uint32_t, std::list<Message*> >::iterator it;
		it = mMessagePool.find(protocol);

		if (it != mMessagePool.end())
		{
			if (it->second.size())
			{
				// free message in pool, use that
				message = it->second.front();
				it->second.pop_front();
			}
		}
		else
		{
			// initialize a list
			mMessagePool[protocol].clear();
		}
		if (message == 0)
		{
			// no more messages, create a new one
			message = mHandlers.Create(protocol, code);
			message->AddToPool(*this);
		}

		message->AddRef();
		return message;
	}

    virtual void ReleaseMessage(Message* msg) 
	{
		std::map<uint32_t, std::list<Message*> >::iterator it;
		it = mMessagePool.find(msg->GetProtcolCRC());

		// Tried to release message not part of pool
		assert(it != mMessagePool.end());

		if (it != mMessagePool.end())
		{
			it->second.push_back(msg);
		}
	}
    /////////////////////////////////////////

	ProtocolInfo* FindInbound(ProtocolInfo& info);
protected:

	MessageHandlerCollection                 mHandlers;
    std::map<uint32_t, std::list<Message*> > mMessagePool;

	typedef std::vector<ProtocolInfo> ProtocolInfoCollection;
    // Protocols we can send
	ProtocolInfoCollection mOutboundProtocols;
    // Protocols we can receive
	ProtocolInfoCollection mInboundProtocols;
private:
};

} /* namespace msg */
}/* namespace pbb*/
#endif /* __PBB_TCP_TRANSPORT_H__ */
