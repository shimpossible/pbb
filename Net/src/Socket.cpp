#include "pbb/net/Socket.h"

#include <WinSock2.h>
#include <Ws2tcpip.h>

#pragma comment( lib, "ws2_32.lib" ) // linker must use this lib for sockets

namespace pbb {
namespace net {

    Socket::Socket(SocketAddress::Family fam, Socket::Type socketType, int protocol)
    {
        int af;
        int type;
        switch (fam)
        {
        case SocketAddress::INET:
            af = AF_INET;
            break;
        case SocketAddress::INET6:
            af = AF_INET;
            break;
        }
        switch (socketType)
        {
        case Socket::RAW:
            type = SOCK_RAW;
            break;
        case Socket::STREAM: // tcp
            type = SOCK_STREAM;
            break;
        case Socket::DGRAM:  // udp
            type = SOCK_DGRAM;
            break;
        }
        mSocket = socket(af, type, protocol);
    }

    Socket::Socket(const Socket* other)
    {
    }

    Socket::~Socket()
    {
        closesocket(mSocket);
    }

    Error Socket::LastError()
    {
        return (Error)WSAGetLastError();
    }

    Error Socket::PeerAddress(SocketAddress& address)
    {        
        int len = sizeof(address.mIPv6);
        if (::getpeername(mSocket, (sockaddr*)&address.mIPv4, &len) == SOCKET_ERROR)
        {
            return LastError();
        }
        return PBB_ESUCCESS;
    }

    Error Socket::Address(SocketAddress& address)
    {
        int len = sizeof(address.mIPv6);
        if (::getsockname(mSocket, (sockaddr*)&address.mIPv4, &len) == SOCKET_ERROR)
        {
            return LastError();
        }
        return PBB_ESUCCESS;
    }

    Error Socket::Receive(void* dest, int len, int& received, int flags )
    {
        int r = ::recv(mSocket, (char*)dest, len, flags);
        
        // recv returns SOCKET_ERROR if there is an error
        switch(r)
        {
        case SOCKET_ERROR:
            received = 0;
            return LastError();
        default:
            received = r;
        }
        return PBB_ESUCCESS;
    }

    Error Socket::Close()
    {
        if (::closesocket(mSocket) == SOCKET_ERROR)
        {
            return LastError();
        }
        return PBB_ESUCCESS;
    }

    Error Socket::Bind(const SocketAddress& address, bool reuse)
    {
        if (::bind(mSocket, address, address.Length()) == SOCKET_ERROR)
        {
            return LastError();
        }
        return PBB_ESUCCESS;
    }

    Error Socket::Listen(int backlong)
    {
        if (::listen(mSocket, backlong) == SOCKET_ERROR)
        {
            return LastError();
        }
        return PBB_ESUCCESS;
    }

    Error Socket::Connect(const SocketAddress& address)
    {
        if (::connect(mSocket, address, address.Length()) == SOCKET_ERROR)
        {
            return LastError();
        }
        return PBB_ESUCCESS;
    }
}
}