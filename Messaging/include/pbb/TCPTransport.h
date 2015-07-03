#ifndef __PBB_TCP_TRANSPORT_H__
#define __PBB_TCP_TRANSPORT_H__

//#include "RouteConfig.h"
#include <pbb/net/Socket.h>

#include "Link.h"
#include "ITransport.h"
#include "Message.h"
#include "MessageHandlerCollection.h"
#include <vector>
#include <unordered_map>


namespace pbb {
namespace msg {

class TCPConnection;

class TCPServer
{
    //friend JellyConnection;
public:
    TCPServer();

    bool Start(uint16_t port);
    bool ConnectTo(const char* address);

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
        */
    typedef std::unordered_map<ObjectId, TCPConnection*> ConnectionMap;
    typedef std::unordered_map<net::pbb_socket_t, TCPConnection*> ClientSocketMap;
    typedef std::map<net::pbb_socket_t, TCPServer*>               ServerSocketMap;

    ConnectionMap              m_Connections;
    static ClientSocketMap     s_Clients;
    static ServerSocketMap     s_KnownConnections;

    //static net::ops s_NetOps;
    //static net::ops s_ClientNetOps;

    //static void net_state_changed(net::pbb_socket_t id, net::State state);
    static void net_accepted(net::pbb_socket_t id, net::pbb_socket_t other, net::SocketAddress& address);
    static void net_received(net::pbb_socket_t id, void* data, size_t len);

    static TCPServer* Find(net::pbb_socket_t id);
    static TCPConnection* FindClient(net::pbb_socket_t id);
    /**
    A new client that hasn't negotiated protocols yet..
    */
    static void AddPending(net::pbb_socket_t id, TCPConnection* client);
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
