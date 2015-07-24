#ifndef __SOCKET_MANAGER_H__
#define __SOCKET_MANAGER_H__

#include <pbb/net/Socket.h>

namespace pbb {
namespace net {

class PBB_NET_API SocketManager
{
public:
    enum State
    {
        LISTENING,
        CONNECTED,
        PENDING_CONNECTION,
        DNS_LOOKUP,
        DISCONNECTED,
    };

    class ISocketCallback
    {
    public:
        virtual void state_changed(pbb::net::Socket* socket, State state) = 0;
        virtual void accepted(pbb::net::Socket* socket, pbb::net::Socket* remote, pbb::net::SocketAddress& address) = 0;
        virtual void received(pbb::net::Socket* socket, void* data, size_t len) = 0;
    };

    SocketManager();
    ~SocketManager();

    Error Close(Socket* sock);

    void Update();
    pbb::net::Socket* OpenAndListen(uint16_t port, ISocketCallback& ops);
    pbb::net::Socket* ConnectTo(const char* address, uint16_t port, ISocketCallback& ops);
    pbb::net::Socket* ConnectTo(const char* address, ISocketCallback& ops);
    pbb::net::Socket* ConnectTo(SocketAddress& address, ISocketCallback& ops);
protected:
    struct SocketControlBlock
    {
        pbb::net::Socket*      socket;
        State                  state;
        ISocketCallback&       ops;
        SocketControlBlock*    next;
        SocketControlBlock(State st, ISocketCallback& opts)
            : socket(0)
            , state( st)
            , ops(opts) 
            , next( 0 )

        {
        }
    };

    void AddSocket(pbb::net::Socket* socket, State state, ISocketCallback& callbacks);
    bool UpdateListening(SocketControlBlock* scb);
    bool UpdateDnsLookup(SocketControlBlock* scb);
    bool UpdatePending(SocketControlBlock* scb);
    bool UpdateConnected(SocketControlBlock* scb);
    SocketControlBlock*   mKnownSockets;

    char mRecvBuffer[2048];
private:
};

} // namespace net
} // namespace pbb

#endif /* __SOCKET_MANAGER_H__ */