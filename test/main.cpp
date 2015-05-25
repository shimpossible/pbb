#include "other/oe_types.h"
#include "other/LocalTransport.h"
#include "other/Endpoint.h"
#include "other/RouteConfig.h"

#include "Thread/include/pbb/thread.h"

#define NO_TYPE (1)

// A protocol
class MessageProto
{
public:
    const static uint32_t CRC = 0;
    enum CODE
    {
        CODE_MyMessage = 0
    };

    static Message* CreateMessage(uint32_t code);

    template<typename HANDLER_T>
    static void Dispatch(Link& link, Message* msg, HANDLER_T* handler)
    {
        switch (msg->GetCode())
        {
        case CODE_MyMessage:
            handler->MyMessageHandler(link, (MyMessage*)msg);
        }
    }
};

class MyMessage : public Message
{
public:

    virtual uint32_t GetProtcolCRC()
    {
        return MessageProto::CRC;
    }
    virtual uint32_t GetCode()
    {
        return MessageProto::CODE_MyMessage;
    }
    virtual void Copy(Message* other)
    {
        if ((other->GetCode() == this->GetCode()) &&
            (other->GetProtcolCRC() == this->GetProtcolCRC())
            )
        {
            // Copy fields
        }
    }
};

MyMessage pool[100];
int pool_index = 0;
Message* MessageProto::CreateMessage(uint32_t code)
{
    switch (code)
    {
    case CODE_MyMessage:
        if (pool_index >= 100) pool_index = 0;
        pool[pool_index].AddRef();
        return &pool[pool_index++];
        //new MyMessage();
    default:
        return 0;
    }
};

class MyService
{
public:
    Endpoint* mEndpoint;
    MyService()
    {
        mEndpoint = new Endpoint(NO_TYPE);
        this->Configure(mEndpoint);
        Instance = this;
    }

    void Configure(Endpoint* srv)
    {
        srv->ConfigureInbound<MessageProto>(MyService::Dispatch);
        srv->ConfigureInbound<MessageProto>(&mQueue);
    }

    void test()
    {
        MyMessage msg;
        for (int i = 0;i < 100;i++)
            mEndpoint->Send(&msg);
    }
    // static callback registered by protocol ID
    // called for each incoming message
    static void Dispatch(Link& link, Message* msg)
    {
        switch (msg->GetProtcolCRC())
        {
        case MessageProto::CRC:
            MessageProto::Dispatch<MyService>(link, msg, Instance);
        }
    }

    void MyMessageHandler(Link& link, Message* msg)
    {
        printf("my message %p received from %s %d\n",
            msg,
        link.local?"local":"remote",
        link.type);
    }

    void Idle()
    {
        mEndpoint->ProcessQueue(mQueue);
    }

protected:

    MessageQueue mQueue;
    static MyService* Instance;
};
MyService* MyService::Instance = 0;

void Configure();

class MyThread
{
public:
    void Run()
    {
        printf("Hello");
    }
};

void main(int argc, const char** argv)
{
    MyThread myThread;
    pbb::Thread thrd;
    thrd.Start<MyThread>(myThread, 0,0, 0xFF, "foo");

    printf("Thread: %s\r\n", thrd.GetName());

    thrd.Join();

    Configure();

    MyService service;
    service.test();
    service.Idle();

    printf("done");
}