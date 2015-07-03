#include "pbb/net/Socket.h"

#ifdef PBB_OS_IS_WINDOWS
#include <WinSock2.h>
#include <Ws2tcpip.h>
#pragma comment( lib, "ws2_32.lib" ) // linker must use this lib for sockets
#endif

namespace pbb {
namespace net {

    SocketAddress::SocketAddress()
    {
        mIPv4.sin_family = AF_INET;
        mIPv4.sin_addr.s_addr = INADDR_ANY;
        mIPv4.sin_port = 0;
    }

    SocketAddress::SocketAddress(sockaddr* addr, int len)
    {
        memcpy(&mIPv4, addr, len);
    }

    SocketAddress::SocketAddress(Family fam, const char* host, uint16_t port)
    {
        switch (fam)
        {
        case INET:
            mIPv4.sin_family = AF_INET;
            inet_pton(AF_INET, host, &mIPv4.sin_addr.s_addr);
            mIPv4.sin_port = htons(port);
            break;
        }
    }

    uint16_t SocketAddress::Port() const
    {
        return ntohs(mIPv4.sin_port);
    }

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
        Close();
    }

    Error Socket::Select(SocketCollection& readSocks,
        SocketCollection& writeSocks,
        SocketCollection& errSocks,
        int timeout, int& ready)
    {
        fd_set readSet;
        fd_set writeSet;
        fd_set errorSet;
        FD_ZERO(&readSet);
        FD_ZERO(&writeSet);
        FD_ZERO(&errorSet);
        timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = timeout;
        uint32_t max = 0;

        for (SocketCollection::const_iterator it = readSocks.begin();
        it != readSocks.end();
            it++
            )
        {
            FD_SET((*it)->mSocket, &readSet);
            max = (max > (*it)->mSocket) ? max : (*it)->mSocket;
        }

        for (SocketCollection::const_iterator it = writeSocks.begin();
        it != writeSocks.end();
            it++
            )
        {
            FD_SET((*it)->mSocket, &writeSet);
            max = (max > (*it)->mSocket) ? max : (*it)->mSocket;
        }
        for (SocketCollection::const_iterator it = errSocks.begin();
        it != errSocks.end();
            it++
            )
        {
            FD_SET((*it)->mSocket, &errorSet);
            max = (max > (*it)->mSocket) ? max : (*it)->mSocket;
        }

        int r = ::select(max + 1, &readSet, &writeSet, &errorSet, &tv);
        if (r == SOCKET_ERROR) return LastError();

        // how many items are set
        ready = r;

        // see which is set
        SocketCollection readOut;
        SocketCollection writeOut;
        SocketCollection errorOut;
#define SELECT_SOCKETS( INLIST, SET, OUTLIST) \
            for (SocketCollection::const_iterator it = INLIST.begin(); it != INLIST.end(); it++ ) \
                if (FD_ISSET((*it)->mSocket, &SET)) OUTLIST.push_back( (*it) );

        SELECT_SOCKETS(readSocks, readSet, readOut);
        SELECT_SOCKETS(writeSocks, writeSet, writeOut);
        SELECT_SOCKETS(errSocks, errorSet, errorOut);

        std::swap(readSocks, readOut);
        std::swap(writeSocks, writeOut);
        std::swap(errSocks, errorOut);

        return PBB_ESUCCESS;
    }

    Error Socket::Poll(pdd_pollfd_t* fdarray, uint32_t fds, int timeout, int& ready)
    {
#ifdef PBB_OS_IS_WINDOWS
        int status = WSAPoll(fdarray, fds, timeout);
#else
        int status = poll(fdarray, fds, timeout);
#endif
        if (status == SOCKET_ERROR)
        {
            return LastError();
        }
        else
        {
            ready = status;
            return PBB_ESUCCESS;
        }
    }

    Error Socket::LastError()
    {
#ifdef PBB_IS_WINDOWS_OS
        return (Error)WSAGetLastError();
#else
	return (Error)errno;
#endif
    }

    Error Socket::PeerAddress(SocketAddress& address)
    {        
        socklen_t len = sizeof(address.mIPv6);
        if (::getpeername(mSocket, (sockaddr*)&address.mIPv4, &len) == SOCKET_ERROR)
        {
            return LastError();
        }
        return PBB_ESUCCESS;
    }

    Error Socket::Ioctl(uint32_t cmd, uint32_t& arg)
    {
#ifdef PBB_OS_IS_WINDOWS
        if (ioctlsocket(mSocket, cmd, (u_long*)&arg) == SOCKET_ERROR)
            return LastError();
#else
        if (ioctl(mSocket, cmd, &arg) != 0)
            return LastError();
#endif
        return PBB_ESUCCESS;
    }

    Error Socket::Address(SocketAddress& address)
    {
        socklen_t len = sizeof(address.mIPv6);
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
#ifdef PBB_OS_IS_WINDOWS
        if (::closesocket(mSocket) == SOCKET_ERROR)
#else
        if (::close(mSocket) != 0)
#endif
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

    Error Socket::Accept(Socket& other)
    {
        other.mSocket = ::accept(mSocket, 0, 0);
        if (other.mSocket == INVALID_SOCKET)
        {
            return LastError();
        }
        return PBB_ESUCCESS;
    }

    Error Socket::GetError()
    {
        Error err = PBB_ESUCCESS;
        socklen_t len = sizeof(err);
        int r = getsockopt(mSocket, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
        if (r == INVALID_SOCKET)
        {
            err = LastError();
        }
        return err;
    }
}
}
