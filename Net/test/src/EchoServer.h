#include "pbb/Thread.h"
#include "pbb/net/Socket.h"

#include <stdio.h>

class EchoServer : pbb::IRunnable
{
public:
    EchoServer();
    ~EchoServer()
    {
        mRunning = false;
        mThread.Join();
    }
    virtual void Run();
protected:

    volatile bool mRunning;
    pbb::Thread mThread;
    pbb::net::Socket  mSocket;
    pbb::net::SocketAddress mAddr;
};