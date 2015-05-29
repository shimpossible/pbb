#include <stdio.h>
#include "RouteConfig.h"
#include <vector>

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

class TestMessage : public pbb::Message
{
public:
    virtual uint32_t GetProtcolCRC() { return 1; }
    virtual uint32_t GetCode() { return 2; }
    virtual void Copy(Message* other)
    {
        if (other==0 || this == other) return;
        // not same message?
        if (other->GetProtcolCRC() != this->GetProtcolCRC() ||
            other->GetCode() != other->GetCode())
            return;

        TestMessage* otherMsg = (TestMessage*)other;
        // do nothing..
        this->data = otherMsg->data;
    }

    /// Start Data
    uint32_t data;
    /// End Data
};

class TEST_PROTOCOL
{
public:
    static const uint32_t CRC = 0x00000001;    
    static TEST_PROTOCOL inst;

    PooledObject<TestMessage, TEST_PROTOCOL> mMsg;
    int releases;
    TEST_PROTOCOL()
        : mMsg(this)
    {
        releases = 0;
    }

    static Message* CreateMessage(uint32_t code)
    {
        // the only copy
        inst.releases = 0;
        // someone just asked for a copy, add a ref
        inst.mMsg.AddRef();
        return &inst.mMsg;
    }

    void Release(Message* msg)
    {
        // refcount SHOULD be 0
        releases++;

    }
};
TEST_PROTOCOL TEST_PROTOCOL::inst;


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

    ASSERT_EQ(0, TEST_PROTOCOL::inst.releases);
    // Ensure Release works..
    this->received[0]->Release();

    ASSERT_EQ(1, TEST_PROTOCOL::inst.releases);
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
    rc.ConfigureTransport(&tport);


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

    // Ensure release works, as expectd
    ASSERT_EQ(0, TEST_PROTOCOL::inst.releases);
    other->Release();
    ASSERT_EQ(1, TEST_PROTOCOL::inst.releases);

    // Test it also routed to the TestTransport
    ASSERT_EQ(1, tport.received.size());
    other = (TestMessage*)tport.received[0];
    // ensure data matches
    ASSERT_EQ(0x1234, other->data);
    
    other->Release();
    // Should still be 1, since 'other' should be a duplicate of myMsg
    ASSERT_EQ(1, TEST_PROTOCOL::inst.releases);

}