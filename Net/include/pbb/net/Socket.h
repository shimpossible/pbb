#ifndef __PBB_NET_SOCKET_H__
#define __PBB_NET_SOCKET_H__
#include "pbb/pbb.h"
#include <vector>
#include <string.h> // memcpy

#ifdef PBB_OS_IS_WINDOWS
   #include <WinSock2.h>
   #include <ws2ipdef.h> // ipv6
   #include <Ws2tcpip.h>
#else
   #include <sys/types.h>
   #include <sys/socket.h>
   #include <errno.h>
   #include <netinet/in.h>
   #include <arpa/inet.h>
   #include <unistd.h>
   #include <fcntl.h>
   #include <sys/ioctl.h>
   #include <poll.h>
   #include <netdb.h>
#endif
namespace pbb {
namespace net {

#ifdef PBB_OS_IS_WINDOWS
    typedef SOCKET        pbb_socket_t;
#else    
    typedef int           pbb_socket_t;
#endif
    typedef struct pollfd pdd_pollfd_t;
    class Socket;
    typedef std::vector<Socket*> SocketCollection;

#ifndef INVALID_SOCKET   
#define INVALID_SOCKET (-1)
#endif

    enum Error
    {
        PBB_ESUCCESS         = 0, /* no error */
#ifdef PBB_OS_IS_WINDOWS
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
#else
        PBB_EINTR            = EINTR,
        PBB_EACCES           = EACCES,
        PBB_EFAULT           = EFAULT,
        PBB_EMFILE           = EMFILE,
        PBB_EAGAIN           = EAGAIN,
        PBB_EWOULDBLOCK      = EWOULDBLOCK,
        PBB_EINPROGRESS      = EINPROGRESS,
        PBB_EALREADY         = EALREADY,
        PBB_ENOTSOCK         = ENOTSOCK,
        PBB_EDESTADDRREQ     = EDESTADDRREQ,
        PBB_EMSGSIZE         = EMSGSIZE,
        PBB_EPROTOTYPE       = EPROTOTYPE,
        PBB_ENOPROTOOPT      = ENOPROTOOPT,
        PBB_EPROTONOSUPPORT  = EPROTONOSUPPORT,
        PBB_ESOCKTNOSUPPORT  = ESOCKTNOSUPPORT,
        PBB_ENOTSUP          = EOPNOTSUPP,
        PBB_EPFNOSUPPORT     = EPFNOSUPPORT,
        PBB_EAFNOSUPPORT     = EAFNOSUPPORT,
        PBB_EADDRINUSE       = EADDRINUSE,
        PBB_EADDRNOTAVAIL    = EADDRNOTAVAIL,
        PBB_ENETDOWN         = ENETDOWN,
        PBB_ENETUNREACH      = ENETUNREACH,
        PBB_ENETRESET        = ENETRESET,
        PBB_ECONNABORTED     = ECONNABORTED,
        PBB_ECONNRESET       = ECONNRESET,
        PBB_ENOBUFS          = ENOBUFS,
        PBB_EISCONN          = EISCONN,
        PBB_ENOTCONN         = ENOTCONN,
        PBB_ESHUTDOWN        = ESHUTDOWN,
        PBB_ETIMEDOUT        = ETIMEDOUT,
        PBB_ECONNREFUSED     = ECONNREFUSED,
        PBB_EHOSTDOWN        = EHOSTDOWN,
        PBB_EHOSTUNREACH     = EHOSTUNREACH,
        PNN_ESYSNOTREADY     = EOPNOTSUPP,
        PBB_ENOTINIT         = -80001,
        PBB_HOST_NOT_FOUND   = HOST_NOT_FOUND,
        PBB_TRY_AGAIN        = TRY_AGAIN,
        PBB_NO_RECOVERY      = NO_RECOVERY,
        PBB_NO_DATA          = NO_DATA,
#define SOCKET_ERROR (-1)
#endif
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

        SocketAddress();

        SocketAddress(sockaddr* addr, int len);

        SocketAddress(Family fam, const char* host, uint16_t port);
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

        uint16_t Port() const;

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
#ifdef PBB_OS_IS_WINDOWS
            // 1 - enable NON blocking
            // 0 - disable
            uint32_t arg = enable ? 0 : 1;
            return Ioctl(FIONBIO, arg);
#else
            // TODO: unix = O_NONBLOCK
            int flags = fcntl(mSocket, F_GETFL,0);
            if(enable==false)
            {
                fcntl(mSocket, F_SETFL, flags | O_NONBLOCK);
            }
            else
            {
                fcntl(mSocket, F_SETFL, flags & ~O_NONBLOCK);
            }
#endif
        }

        Error Ioctl(uint32_t cmd, uint32_t& arg);

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
        Error Accept(Socket& other);

        Error GetError();

        static Error LastError();

        static Error Select(SocketCollection& readSocks,
            SocketCollection& writeSocks,
            SocketCollection& errSocks,
            int timeout, int& ready);
        
        /**
            DONT use this on a socket that isn't connected yet
         */
        static Error Poll(pdd_pollfd_t* fdarray, uint32_t fds, int timeout, int& ready);

    protected:

        pbb_socket_t mSocket;
    private:
    };

} /* namespace net */
} /* namespace pbb */
#endif /* __PBB_SOCKET_H__ */
