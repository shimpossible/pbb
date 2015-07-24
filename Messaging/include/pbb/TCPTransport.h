#ifndef __PBB_TCP_TRANSPORT_H__
#define __PBB_TCP_TRANSPORT_H__

//#include "RouteConfig.h"
#include <pbb/net/Socket.h>
#include <pbb/net/SocketManager.h>

#include "Link.h"
#include "ITransport.h"
#include "Message.h"
#include "MessageHandlerCollection.h"
#include <vector>
#include <unordered_map>


namespace pbb {
namespace msg {

class TCPServer;
class TCPConnection;

class TCPConnection
{
public:
	TCPConnection(pbb::net::Socket* socket, TCPServer& server)
	{
	}
	~TCPConnection()
	{

	}

	/**
	  Call when new data arives for client
	  @param src  Pointer to buffer of new data
	  @param len  Number of bytes in buffer
	 */
	void Receive(const void* src, size_t len)
	{
	}
};

class TCPServer 
{
    //friend JellyConnection;
public:
    TCPServer();

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

protected:

    /**
        There are a few layers of IDs.
        Server   -> Remote Server, contains 1 or more Endpoints
        Endpoint -> A single object on a remote server
        This keeps track of which connection an object is found on
        */
    typedef std::unordered_map<ObjectId, TCPConnection*> ConnectionMap;
    /**
     Map a socket to a connection object
     */
    typedef std::unordered_map<net::pbb_socket_t, TCPConnection*> ClientSocketMap;


	class Callbacks : public net::SocketManager::ISocketCallback
	{
	public:
		Callbacks(TCPServer& parent)
		: m_Transport( parent)
		{
			
		}
		virtual void state_changed(pbb::net::Socket* socket, net::SocketManager::State state);
		virtual void accepted(pbb::net::Socket* socket, pbb::net::Socket* remote, pbb::net::SocketAddress& address);
		virtual void received(pbb::net::Socket* socket, void* data, size_t len);

		TCPServer& m_Transport;
	};

	Callbacks                  m_SocketCallback;
	net::SocketManager         m_SocketMgr;
    net::Socket*               m_Socket;
    ClientSocketMap            m_KnownConnections;

    TCPConnection* FindClient(net::pbb_socket_t id);
    /**
    A new client that hasn't negotiated protocols yet..
    */
    void AddPending(net::pbb_socket_t id, TCPConnection* client);
};

/**
Transports the message to/from Remote endpoints
*/
class PBB_API TCPTransport : public ITransport, IMessagePool
{
public:
    /**
    New TCPTransport
    @param port  TCP port to listen on
    */
    TCPTransport(const uint16_t port);

    /**
    Called by RouteConfig when sending a message.  To Transmit
    outgoing message to all local endpoints
    */
    virtual void Transmit(Link& link, Message* msg);

    /**
    Notify transport of Protocols that will be sent
    RouteConfig will call this whenever RouteConfig::ConfigureOutbound is called
    */
    virtual void ConfigureOutbound(uint32_t crc);

    virtual void ConfigureInbound(uint32_t crc);

    /////////////////////////////////////////
    // IMessagePool functions

    virtual Message* CreateMessage(uint32_t protocol, uint32_t code) { return 0; }
    virtual void ReleaseMessage(Message* msg) {}
    /////////////////////////////////////////
protected:

    std::map<uint32_t, std::list<Message*> > mMessagePool;

    // Protocols we can send
    std::vector<uint32_t> mOutboundProtocols;
    // Protocols we can receive
    std::vector<uint32_t> mInboundProtocols;
private:
};

} /* namespace msg */
}/* namespace pbb*/
#endif /* __PBB_TCP_TRANSPORT_H__ */
