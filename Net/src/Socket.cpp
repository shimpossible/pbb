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

    const Socket* Socket::InvalidSocket = (Socket*)INVALID_SOCKET;

    Socket* Socket::Create(SocketAddress::Family fam, Socket::Type socketType, int protocol)
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
        return (Socket*)socket(af, type, protocol);
    }

    Error Socket::Select(
        SocketCollection* readSocks,
        SocketCollection* writeSocks,
        SocketCollection* errSocks,
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

        if(readSocks)
        for (SocketCollection::const_iterator it = readSocks->begin();
             it != readSocks->end();
             it++
            )
        {
            FD_SET(*(*it), &readSet);
            max = (max > *(*it)) ? max : *(*it);
        }

        if(writeSocks)
        for (SocketCollection::const_iterator it = writeSocks->begin();
             it != writeSocks->end();
             it++
            )
        {
            FD_SET(*(*it), &writeSet);
            max = (max > *(*it)) ? max : *(*it);
        }

        if(errSocks)
        for (SocketCollection::const_iterator it = errSocks->begin();
             it != errSocks->end();
             it++
            )
        {
            FD_SET( *(*it), &errorSet);
            max = (max > *(*it)) ? max : *(*it);
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
         if(INLIST) { \
            for (SocketCollection::const_iterator it = INLIST->begin(); it != INLIST->end(); it++ ) \
                if (FD_ISSET(*(*it), &SET)) OUTLIST.push_back( (*it) ); \
          INLIST->swap(OUTLIST); }

        SELECT_SOCKETS(readSocks,  readSet,  readOut);
        SELECT_SOCKETS(writeSocks, writeSet, writeOut);
        SELECT_SOCKETS(errSocks,   errorSet, errorOut);

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
#ifdef PBB_OS_IS_WINDOWS
        return (Error)WSAGetLastError();
#else
	return (Error)errno;
#endif
    }

    Error Socket::PeerAddress(SocketAddress& address)
    {        
        socklen_t len = sizeof(address.mIPv6);
        if (::getpeername(*this, (sockaddr*)&address.mIPv4, &len) == SOCKET_ERROR)
        {
            return LastError();
        }
        return PBB_ESUCCESS;
    }

    Error Socket::Ioctl(uint32_t cmd, uint32_t& arg)
    {
#ifdef PBB_OS_IS_WINDOWS
        if (ioctlsocket(*this, cmd, (u_long*)&arg) == SOCKET_ERROR)
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
        if (::getsockname(*this, (sockaddr*)&address.mIPv4, &len) == SOCKET_ERROR)
        {
            return LastError();
        }
        return PBB_ESUCCESS;
    }

    Error Socket::Send(const void* src, int len, int& sent, int flags)
    {
        int r = ::send(*this, (char*)src, len, flags);

        // send returns SOCKET_ERROR if there is an error
        switch (r)
        {
        case SOCKET_ERROR:
            sent = 0;
            return LastError();
        default:
            sent = r;
        }
        return PBB_ESUCCESS;
    }

    Error Socket::Receive(void* dest, int len, int& received, int flags )
    {
        int r = ::recv(*this, (char*)dest, len, flags);
        
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
        if (::closesocket(*this) == SOCKET_ERROR)
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
        if (::bind(*this, address, address.Length()) == SOCKET_ERROR)
        {
            return LastError();
        }
        return PBB_ESUCCESS;
    }

    Error Socket::Listen(int backlong)
    {
        if (::listen(*this, backlong) == SOCKET_ERROR)
        {
            return LastError();
        }
        return PBB_ESUCCESS;
    }

    Error Socket::Connect(const SocketAddress& address)
    {
        if (::connect(*this, address, address.Length()) == SOCKET_ERROR)
        {
            return LastError();
        }
        return PBB_ESUCCESS;
    }

    Error Socket::Accept(Socket*& other, SocketAddress* addr)
    {
        socklen_t len = SocketAddress::MaxLength;
        other = (Socket*)::accept(*this, (sockaddr*)&addr->mIPv4, &len);
        if (*other == INVALID_SOCKET)
        {
            return LastError();
        }
        return PBB_ESUCCESS;
    }

    Error Socket::GetError()
    {
        Error err = PBB_ESUCCESS;
        socklen_t len = sizeof(err);
        int r = getsockopt(*this, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
        if (r == INVALID_SOCKET)
        {
            err = LastError();
        }
        return err;
    }
}
}
