#ifndef __PBB_TCP_TRANSPORT_H__
#define __PBB_TCP_TRANSPORT_H__

#include <pbb/net/Socket.h>
#include <pbb/net/SocketManager.h>
#include <pbb/DataChain.h>

#include "RouteConfig.h"
#include "Link.h"
#include "ITransport.h"
#include "Message.h"
#include "MessageHandlerCollection.h"
#include <vector>
#include <unordered_map>

#include "TCPTransportProtocol.h"

namespace pbb {
namespace msg {

class TCPServer;
class TCPTransport;


/**
  Bytes needed to encode number
 */
uint32_t PBB_API Encoded7BitBytes(uint32_t val);
class PBB_API TCPServer
{
public:

	enum MsgType
	{
		MSG_SYSTEM,
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


	/**
	    
	 */
	class PBB_API Connection
	{
	public:

		typedef std::vector<ProtocolInfo> ProtocolInfoCollection;
		typedef std::map<uint32_t, uint32_t> ProtocolEncodingMap;

		enum State
		{
			STATE_HELLO,
			STATE_CONNECTED,
		};

		Connection(pbb::net::Socket* socket, TCPTransport& transport, TCPServer& server, bool isServer=false)
			: mIsServer(isServer)
			, mSocket(socket)
			, mReceiveBuffer(2048, 0)
			, mTransmitBuffer(2048, 4) /* 4 bytes for prefixed length */
			, mTransport(transport)
			, mState(STATE_HELLO)
			, mServer(server)

		{
			printf("new connection %p\r\n", this);
			mProtocolEncoding[0] = BinaryDecoder::ID;

			mReceiveState = -1;
			mReceiveMessageState = -1;
			nameLength = 0;
		}

		~Connection()
		{

		}

		enum State State()
		{
			return mState;
		}

		void SendHello();

		/**
		Call when new data arives for client
		@param src  Pointer to buffer of new data
		@param len  Number of bytes in buffer
		*/
		void Receive(const void* src, size_t len);

		void ReceiveHello();

		void Send(pbb::msg::Message& msg);

		/**
		 Send data
		*/
		void Send(const void* src, size_t len);

		bool ProcessReceiveMessage();

		/**
		  Given a list of remote protocols, go through them a decide which ones to keep
		 */
		bool ProcessReconcile();
		bool ProcessResolveEncoders();

		void AddProtocol(const char* name, uint32_t crc);

		ProtocolInfoCollection& RemoteProtocols() { return mRemoteProtocols;  }

		ProtocolInfo* GetProtocolForChannel(uint32_t chan);
		bool GetChannelForProtocol(uint32_t proto, uint8_t& chan);

		bool IsServer() { return mIsServer; }

		void BeginRequestEncoders();

		void SendInboundProtocols();
		void SendOutboundProtocols();

	protected:
		typedef std::map<uint32_t, ProtocolInfo*> ChannelMapCollection;
		typedef std::map<uint32_t, uint32_t>      ProtocolToChannelCollection;
		enum State mState;

		bool          mIsServer;
		net::Socket*  mSocket;
		DataChain     mReceiveBuffer;
		DataChain     mTransmitBuffer;
		ObjectId      mId;
		TCPTransport& mTransport;
		TCPServer&    mServer;

		// Protocol info received from Remote connection
		ProtocolInfoCollection      mRemoteProtocols;
		ChannelMapCollection        mChannelMap;  //!< ChannelId to Protocol
		ProtocolToChannelCollection mProtocolMap; //!< Protocol CRC to channel Id

		ProtocolInfoCollection::iterator mProtoIter;

		
		// Quick lookup when sending
		ProtocolEncodingMap    mProtocolEncoding;

		uint32_t   mBestEncoding;
		uint32_t   mFallbackEncoding;

		Link       mLink;

		int mReceiveState;
		int mReceiveMessageState;
		uint32_t nameLength;


		void IncomingProtocolCallback(ProtocolInfo* info);
	};

    TCPServer(TCPTransport& transport);

	void Update();
    bool Start(uint16_t port);
    bool ConnectTo(const char* address, uint16_t port);

	net::Error Address(pbb::net::SocketAddress& addr)
	{		
		return m_Socket->Address(addr);
	}
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

	void OnHelloReceived(Connection* connection, ObjectId& id);

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
	@param config Routes
    @param port  TCP port to listen on
    */
    TCPTransport(RouteConfig& config, uint16_t port);

    /**
    Called by RouteConfig when sending a message.  To Transmit
    outgoing message to all local endpoints
	@param link Who sent the message
	@param msg  Message to send
    */
    virtual void Transmit(Link& link, Message* msg);
	void Receive(Link& link, Message* msg);

	void Update();

    /**
    Notify transport of Protocols that will be sent
    RouteConfig will call this whenever RouteConfig::ConfigureOutbound is called
    */
    virtual void ConfigureOutbound(ProtocolInfo* info);

    virtual void ConfigureInbound(ProtocolInfo* info);

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
			message = mRouteConfig.CreateMessage(protocol, code);
			
			// Couldn't create message
			if (message == nullptr) return message;

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


	typedef std::vector<ProtocolInfo> ProtocolInfoCollection;

	TCPServer& Server() { return mServer; }

	//////////////////////////////////////////
	// TCPTransportProtocol
	void AddProtocolHandler(AddProtocol* msg);
	//
	//////////////////////////////////////////
protected:

	friend TCPServer::Connection;

	RouteConfig&                             mRouteConfig;
	TCPServer                                mServer;
    std::map<uint32_t, std::list<Message*> > mMessagePool;
    // Protocols we can send
	ProtocolInfoCollection mOutboundProtocols;
    // Protocols we can receive
	ProtocolInfoCollection mInboundProtocols;

	static void DispatchMessage(void* ctx, Link& link, Message* msg);

private:
};

} /* namespace msg */
}/* namespace pbb*/
#endif /* __PBB_TCP_TRANSPORT_H__ */
