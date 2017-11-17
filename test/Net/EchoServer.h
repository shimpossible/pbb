#include "pbb/thread/Thread.h"
#include "pbb/net/Socket.h"

#include <stdio.h>

class EchoServer : pbb::IRunnable
{
public:
    EchoServer();
    ~EchoServer()
    {
        mRunning = false;
	printf("Waiting for thread to end\r\n");
        mThread.Join();
    }
    virtual void Run();
    void Stop(){ mRunning = 0; }
protected:

    volatile bool mRunning;
    pbb::Thread mThread;
    pbb::net::Socket*  mSocket;
    pbb::net::SocketAddress mAddr;
};
