#include <stdio.h>
#include "pbb/RouteConfig.h"
#include <vector>

#include "TestProtocol.h"

#include "gtest/gtest.h"

using namespace pbb;

class LocalTransportTest : public ::testing::Test {
public:
    std::vector<Message*> received;
    static void MsgReceive(void* ctx, Link& link, Message* msg)
    {
        LocalTransportTest* self = (LocalTransportTest*)ctx;

        self->received.push_back(msg);
        // dont let it get disposed
        msg->AddRef();
    }
};

/**
    By default RouteConfig to route to anyone local connection configured 
    to receive the data
 */
TEST_F(LocalTransportTest, Transmit)
{

    TestMessage myMsg;
    myMsg.data = 0x1234;
    pbb::Link myLink;

    // route messages to our MsgReceive
    MessageHandlerCollection handlers;
    handlers.Add(TEST_PROTOCOL::CRC, TEST_PROTOCOL::CreateMessage, this, &MsgReceive);

    // the object under test
    LocalTransport transport(handlers);

    // Send a message through local transport 
    transport.Transmit(myLink, &myMsg);    

    // MsgReceived should have been called 1 time
    ASSERT_EQ(1, received.size());
    // same message
    ASSERT_EQ(myMsg.GetCode(), received[0]->GetCode());

    // duplicate copy
    ASSERT_NE(&myMsg, received[0]);

    // Pooled message
    received[0]->Release();
}

class TestTransport : public ITransport
{
    public:     
        std::vector<Message*> received;
        std::vector<uint32_t> outbound;
        TestTransport()
        {
        }
        virtual void Transmit(Link&, Message* msg)
        {
            msg->AddRef();
            received.push_back(msg);
        }

        virtual void ConfigureOutbound(uint32_t crc)
        {
            outbound.push_back(crc);
        }

};
