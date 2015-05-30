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
    //////////////////////////
    // setup
    TestMessage myMsg;
    myMsg.data = 0x1234;
    pbb::Link myLink;
    MessageHandlerCollection handlers;
    handlers.Add(TEST_PROTOCOL::CRC, TEST_PROTOCOL::CreateMessage, this, &MsgReceive);    
    LocalTransport transport(handlers);

    //////////////////////////
    // Method to test
    transport.Transmit(myLink, &myMsg);    

    //////////////////////////
    // test results

    // MsgReceived should have been called 1 time
    ASSERT_EQ(1, received.size());
    // same message
    ASSERT_EQ(myMsg.GetCode(), received[0]->GetCode());

    // duplicate copy
    ASSERT_NE(&myMsg, received[0]);

    // Pooled message, check we only have 1 ref
    ASSERT_EQ(2, received[0]->AddRef());
    ASSERT_EQ(1, received[0]->Release());
    ASSERT_EQ(0, received[0]->Release());
}

