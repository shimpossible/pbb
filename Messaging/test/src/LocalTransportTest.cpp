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
TEST_F(LocalTransportTest, LocalTransport)
{

    TestMessage myMsg;
    myMsg.data = 0x1234;
    pbb::Link myLink;
    MessageHandlerCollection handlers;
    LocalTransport transport(handlers);
    
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
