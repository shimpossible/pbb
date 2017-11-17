#ifndef __TEST_PROTOCOL_H__
#define __TEST_PROTOCOL_H__
#include <pbb/msg/Message.h>
#include <pbb/msg/ITransport.h>

#include <vector>

// Decoders the Message supports
#include "pbb/msg/BinaryDecoder.h"
#include "pbb/msg/BinaryEncoder.h"

class TestMessage : public pbb::msg::Message
{
public:
	const static pbb::msg::MessageID CODE = 2;
	const static char* NAME;

    uint32_t GetProtcolCRC() { return 1; }
	pbb::msg::MessageID GetCode() { return TestMessage::CODE; }

	bool Get(pbb::DataChain& data, uint32_t protocol);
	bool Get(pbb::msg::BinaryDecoder& decoder);

	bool Put(pbb::DataChain& data, uint32_t protocol);
	bool Put(pbb::msg::BinaryEncoder& encoder);

    virtual void Copy(pbb::msg::Message* other)
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
	static const char* NAME;

    TEST_PROTOCOL()        
    {
    }

    static pbb::msg::Message* CreateMessage(uint32_t code)
    {
        return new TestMessage();
    }

	static pbb::msg::ProtocolInfo* Info();

};

class TestTransport : public pbb::msg::ITransport
{
public:
    std::vector<pbb::msg::Message*> received;
    std::vector<uint32_t> outbound;
    TestTransport()
    {
    }
    virtual void Transmit(pbb::msg::Link&, pbb::msg::Message* msg)
    {
        msg->AddRef();
        received.push_back(msg);
    }

	virtual void ConfigureOutbound(pbb::msg::ProtocolInfo* info)
	{
		outbound.push_back(info->CRC);
	}

	virtual void ConfigureInbound(pbb::msg::ProtocolInfo* info)
	{

	}
};

#endif