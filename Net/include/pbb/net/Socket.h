#ifndef __PBB_NET_SOCKET_H__
#define __PBB_NET_SOCKET_H__
#include "pbb/pbb.h"
#include <vector>

// TODO: only on windows
#include <WinSock2.h>
#include <ws2ipdef.h> // ipv6
#include <Ws2tcpip.h>
namespace pbb {
namespace net {

    typedef SOCKET        pbb_socket_t;
    typedef struct pollfd pdd_pollfd_t;
    class Socket;
    typedef std::vector<Socket*> SocketCollection;

#define PBB_INVALID_SOCKET  INVALID_SOCKET
    enum Error
    {
        // TODO: only on windows
        PBB_ESUCCESS         = 0, /* no error */
        PBB_EINTR            = WSAEINTR,
        PBB_EACCES           = WSAEACCES,
        PBB_EFAULT           = WSAEFAULT,
        PBB_EMFILE           = WSAEMFILE,
        PBB_EAGAIN           = WSAEWOULDBLOCK,
        PBB_EWOULDBLOCK      = WSAEWOULDBLOCK,
        PBB_EINPROGRESS      = WSAEINPROGRESS,
        PBB_EALREADY         = WSAEALREADY,
        PBB_ENOTSOCK         = WSAENOTSOCK,
        PBB_EDESTADDRREQ     = WSAEDESTADDRREQ,
        PBB_EMSGSIZE         = WSAEMSGSIZE,
        PBB_EPROTOTYPE       = WSAEPROTOTYPE,
        PBB_ENOPROTOOPT      = WSAENOPROTOOPT,
        PBB_EPROTONOSUPPORT  = WSAEPROTONOSUPPORT,
        PBB_ESOCKTNOSUPPORT  = WSAESOCKTNOSUPPORT,
        PBB_ENOTSUP          = WSAEOPNOTSUPP,
        PBB_EPFNOSUPPORT     = WSAEPFNOSUPPORT,
        PBB_EAFNOSUPPORT     = WSAEAFNOSUPPORT,
        PBB_EADDRINUSE       = WSAEADDRINUSE,
        PBB_EADDRNOTAVAIL    = WSAEADDRNOTAVAIL,
        PBB_ENETDOWN         = WSAENETDOWN,
        PBB_ENETUNREACH      = WSAENETUNREACH,
        PBB_ENETRESET        = WSAENETRESET,
        PBB_ECONNABORTED     = WSAECONNABORTED,
        PBB_ECONNRESET       = WSAECONNRESET,
        PBB_ENOBUFS          = WSAENOBUFS,
        PBB_EISCONN          = WSAEISCONN,
        PBB_ENOTCONN         = WSAENOTCONN,
        PBB_ESHUTDOWN        = WSAESHUTDOWN,
        PBB_ETIMEDOUT        = WSAETIMEDOUT,
        PBB_ECONNREFUSED     = WSAECONNREFUSED,
        PBB_EHOSTDOWN        = WSAEHOSTDOWN,
        PBB_EHOSTUNREACH     = WSAEHOSTUNREACH,
        PNN_ESYSNOTREADY     = WSASYSNOTREADY,
        PBB_ENOTINIT         = WSANOTINITIALISED,
        PBB_HOST_NOT_FOUND   = WSAHOST_NOT_FOUND,
        PBB_TRY_AGAIN        = WSATRY_AGAIN,
        PBB_NO_RECOVERY      = WSANO_RECOVERY,
        PBB_NO_DATA          = WSANO_DATA,
    };

    class PBB_API SocketAddress
    {
        friend class Socket;
    public:
        enum Family
        {
            //! IPv4
            INET = AF_INET,
            //! IPv6
            INET6 = AF_INET6,
        };
        const static int MaxLength = sizeof(sockaddr_in6);

        SocketAddress()
        {
            mIPv4.sin_family = AF_INET;
            mIPv4.sin_addr.s_addr = INADDR_ANY;
            mIPv4.sin_port = 0;
        }

        SocketAddress(sockaddr* addr, int len)
        {
            memcpy(&mIPv4, addr, len);
        }

        SocketAddress(Family fam, const char* host, uint16_t port)
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
        ~SocketAddress() {};

        // Implicit cast
        virtual operator sockaddr* () const
        {
            return (sockaddr*)&mIPv4;
        }
        virtual Family AddressFamily() const
        {
            return (Family)mIPv4.sin_family;
        }

        uint16_t Port() const
        {
            return ntohs(mIPv4.sin_port);
        }
        uint32_t Length() const
        {
            if (mIPv4.sin_family == AF_INET)  return sizeof(mIPv4);
            if (mIPv4.sin_family == AF_INET6) return sizeof(mIPv6);
            return 0;
        }

    protected:

        union {
            struct sockaddr_in  mIPv4;
            struct sockaddr_in6 mIPv6;
        };
    private:
    };

    /*
      Network socket
     */
    class PBB_API Socket
    {
    public:
        enum Type
        {
            STREAM,
            DGRAM,
            TCP = STREAM,
            UDP = DGRAM,
            RAW,
        };

        Socket()
            :mSocket(INVALID_SOCKET)
        {
        }

        Socket(SocketAddress::Family fam, Type type, int protocol=0);
        Socket(const Socket* other);
        ~Socket();

        operator pbb_socket_t()
        {
            return mSocket;
        }

        /**
          Set blocking or non-blocking (async) mode
          @param enable false=async, true=blocking
         */
        Error SetBlocking(bool enable)
        {
            // 1 - enable NON blocking
            // 0 - disable
            uint32_t arg = enable ? 0 : 1;
            return Ioctl(FIONBIO, arg);
            // TODO: unix = O_NONBLOCK
        }

        Error Ioctl(uint32_t cmd, uint32_t& arg)
        {
            if (ioctlsocket(mSocket, cmd, (u_long*)&arg) == SOCKET_ERROR)
                return LastError();

            return PBB_ESUCCESS;
        }
        /*
            Get the local address and port the socket is on
        */
        Error Address(SocketAddress& address);

        Error PeerAddress(SocketAddress& address);

        Error Receive(void* dest, int len, int& received, int flags=0);
        Error Send(const void* src, int length, int flags);

        Error Close();
        
        Error Bind(const SocketAddress& address, bool reuse);
        Error Listen(int backlog);

        Error Connect(const SocketAddress& address);
        Error Accept(Socket& other)
        {
            other.mSocket = ::accept(mSocket, 0, 0);
            if (other.mSocket == INVALID_SOCKET)
            {
                return LastError();
            }
            return PBB_ESUCCESS;
        }

        Error GetError()
        {
            Error err = PBB_ESUCCESS;
            int len = sizeof(err);
            int r = getsockopt(mSocket, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
            if (r == INVALID_SOCKET)
            {
                err = LastError();
            }
            return err;
        }

        static Error LastError();

        static Error Select(SocketCollection& readSocks,
            SocketCollection& writeSocks,
            SocketCollection& errSocks,
            int timeout, int& ready)
        {
            FD_SET readSet;
            FD_SET writeSet;
            FD_SET errorSet;
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

            int r = ::select(max+1, &readSet, &writeSet, &errorSet, &tv);
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

            SELECT_SOCKETS(readSocks,  readSet,  readOut);
            SELECT_SOCKETS(writeSocks, writeSet, writeOut);
            SELECT_SOCKETS(errSocks, errorSet, errorOut);

            std::swap(readSocks,  readOut);
            std::swap(writeSocks, writeOut);
            std::swap(errSocks,   errorOut);

            return PBB_ESUCCESS;
        }
        /**
            DONT use this on a socket that isn't connected yet
         */
        static Error Poll(pdd_pollfd_t* fdarray, uint32_t fds, int timeout, int& ready)
        {
            int status = WSAPoll(fdarray, fds, timeout);
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
    protected:

        pbb_socket_t mSocket;
    private:
    };

} /* namespace net */
} /* namespace pbb */
#endif /* __PBB_SOCKET_H__ */