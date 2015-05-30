#include <stdio.h>
#include "pbb/RouteConfig.h"
#include <vector>
#include "testprotocol.h"
#include "gtest/gtest.h"

using namespace pbb;
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
    By default RouteConfig to route to anyone local connection configured 
    to receive the data
 */
TEST_F(RouteConfigTest, LocalTransport)
{
    RouteConfig rc;
    rc.ConfigureInbound<TEST_PROTOCOL>(this, MsgReceive);

    TestMessage myMsg;
    myMsg.data = 0x1234;
    pbb::Link myLink;

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
/**
    Multiple Transports
 */
TEST_F(RouteConfigTest, Transport)
{
    RouteConfig rc;

    // Add test transport
    TestTransport tport;    
    rc.ConfigureTransport(tport);

    rc.ConfigureInbound<TEST_PROTOCOL>(this, MsgReceive);

    TestMessage myMsg;
    TestMessage* other;
    myMsg.data = 0x1234;
    pbb::Link myLink;

    rc.Send(myLink, &myMsg);

    // Ensure it routed to the local MsgReceive function
    ASSERT_EQ(1, this->received.size());
    // Ensure it gave a NEW instance
    ASSERT_NE(this->received[0], &myMsg);
    other = (TestMessage*)this->received[0];
    ASSERT_EQ(0x1234, other->data);

    other->Release();

    // Test it also routed to the TestTransport
    ASSERT_EQ(1, tport.received.size());
    other = (TestMessage*)tport.received[0];
    // ensure data matches
    ASSERT_EQ(0x1234, other->data);
    
    other->Release();
    // Should still be 1, since 'other' should be a duplicate of myMsg

}