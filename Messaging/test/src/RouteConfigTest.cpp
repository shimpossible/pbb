#include <stdio.h>
#include "pbb/RouteConfig.h"
#include <vector>
#include "testprotocol.h"
#include "gtest/gtest.h"

using namespace pbb::msg;

class RouteConfigTest : public ::testing::Test {
public:

    std::vector<Message*> received;
    static void MsgReceive(void* ctx, Link& link, Message* msg)
    {
        RouteConfigTest* self = (RouteConfigTest*)ctx;
                
        self->received.push_back(msg);
        // dont let it get disposed
        msg->AddRef();
    }
};

/**
    By default RouteConfig to route to any local connection configured 
    to receive the data
 */
TEST_F(RouteConfigTest, LocalTransport)
{
    RouteConfig rc;
    rc.ConfigureInbound<TEST_PROTOCOL>(this, MsgReceive);

    TestMessage myMsg;
    myMsg.data = 0x1234;
    Link myLink;

    rc.Send(myLink, &myMsg);

    // Ensure it routed to the MsgReceive function
    ASSERT_EQ(1, this->received.size());
    // Ensure it gave a NEW instance
    ASSERT_NE(this->received[0], &myMsg);
    TestMessage* other = (TestMessage*)this->received[0];
    ASSERT_EQ(0x1234, other->data);

    // Ensure Release works..
    this->received[0]->Release();

}

/**
    Multiple Transports.  RouteConfig should route to all
    defined transports
 */
TEST_F(RouteConfigTest, Transport)
{
    RouteConfig rc;

    // Add test transport
    TestTransport tport;    
    rc.ConfigureTransport(tport);

    // Route all received TEST_PROTOCOL message to MsgReceive
    rc.ConfigureInbound<TEST_PROTOCOL>(this, MsgReceive);

    TestMessage myMsg;
    TestMessage* other;
    myMsg.data = 0x1234;
    Link myLink;

    // Send a message, should go through TestTransport 
    rc.Send(myLink, &myMsg);

    // Ensure it routed to the local MsgReceive function
    ASSERT_EQ(1, this->received.size());
    // Ensure it gave a NEW instance
    ASSERT_NE(this->received[0], &myMsg);
    other = (TestMessage*)this->received[0];
    ASSERT_EQ(0x1234, other->data);

    ASSERT_EQ(2, other->AddRef());
    ASSERT_EQ(1, other->Release());
    ASSERT_EQ(0, other->Release());

    // Test it also routed to the TestTransport
    ASSERT_EQ(1, tport.received.size());
    other = (TestMessage*)tport.received[0];
    // ensure data matches
    ASSERT_EQ(0x1234, other->data);

    // Test Transport doesn't use a pooled message
    ASSERT_EQ(0, other->AddRef());
    ASSERT_EQ(0, other->Release());

    // Should still be 1, since 'other' should be a duplicate of myMsg

}