#ifndef __TEST_PROTOCOL_H__
#define __TEST_PROTOCOL_H__
#include <pbb/Message.h>
#include <pbb/ITransport.h>

#include <vector>

class TestMessage : public pbb::Message
{
public:
    virtual uint32_t GetProtcolCRC() { return 1; }
    virtual uint32_t GetCode() { return 2; }
    virtual void Copy(Message* other)
    {
        if (other == 0 || this == other) return;
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

    TEST_PROTOCOL()        
    {
    }

    static pbb::Message* CreateMessage(uint32_t code)
    {
        return new TestMessage();
    }

};


class TestTransport : public pbb::ITransport
{
public:
    std::vector<pbb::Message*> received;
    std::vector<uint32_t> outbound;
    TestTransport()
    {
    }
    virtual void Transmit(pbb::Link&, pbb::Message* msg)
    {
        msg->AddRef();
        received.push_back(msg);
    }

    virtual void ConfigureOutbound(uint32_t crc)
    {
        outbound.push_back(crc);
    }

};

#endif