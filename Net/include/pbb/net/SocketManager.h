#ifndef __SOCKET_MANAGER_H__
#define __SOCKET_MANAGER_H__

#include <pbb/net/Socket.h>

namespace pbb {
namespace net {

class PBB_API SocketManager
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

    struct SocketCallback
    {
        void (CDECL *state_changed)(pbb::net::Socket* socket, State state);
        void (CDECL *accepted)(pbb::net::Socket* socket, pbb::net::Socket* remote, pbb::net::SocketAddress& address);
        void (CDECL *received)(pbb::net::Socket* socket, void* data, size_t len);
    };

    SocketManager();
    ~SocketManager();

    Error Close(Socket* sock);

    void Update();
    pbb::net::Socket* OpenAndListen(uint16_t port, SocketCallback& ops);
    pbb::net::Socket* ConnectTo(const char* address, uint16_t port, SocketManager::SocketCallback& ops);
    pbb::net::Socket* ConnectTo(const char* address, SocketManager::SocketCallback& ops);
    pbb::net::Socket* ConnectTo(SocketAddress& address, SocketManager::SocketCallback& ops);
protected:
    struct SocketControlBlock
    {
        pbb::net::Socket*      socket;
        State                  state;
        SocketCallback         ops;
        SocketControlBlock*    next;
        SocketControlBlock()
        {
        }
    };

    void AddSocket(pbb::net::Socket* socket, State state, SocketCallback& callbacks);
    bool UpdateListening(SocketControlBlock* scb);
    bool UpdateDnsLookup(SocketControlBlock* scb);
    bool UpdatePending(SocketControlBlock* scb);

    SocketControlBlock*   mKnownSockets;
private:
};

} // namespace net
} // namespace pbb

#endif /* __SOCKET_MANAGER_H__ */