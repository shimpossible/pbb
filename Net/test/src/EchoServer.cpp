/**
    Repeats back everything it receives
 */
#include "EchoServer.h"

EchoServer::EchoServer()
    : mThread()
    , mSocket( pbb::net::SocketAddress::INET, pbb::net::Socket::TCP)
{
    mRunning = true;
    mThread.Start(*this);
}

void EchoServer::Run()
{
    mSocket.Bind(mAddr, true);
    mSocket.Listen(10);
    pbb::net::pdd_pollfd_t socketfd;
    socketfd.fd = mSocket;
    socketfd.events = POLLRDNORM;

    pbb::net::SocketAddress sa;
    mSocket.Address(sa);
    printf("Port: %d\r\n", sa.Port());
    while(mRunning)
    {
        int ready = 0;
        pbb::net::Error e = mSocket.Poll(&socketfd, 1, 10, ready);
        if (ready)
        {
            printf("someone connected..\r\n");
        }
    }

    printf("Echo servo closing\r\n");
}
