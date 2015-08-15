#include "TestProtocol.h"

using namespace pbb::msg;

const char* TestMessage::NAME = "TestMessage";
ProtocolInfo mInfo("TEST_PROTOCOL", TEST_PROTOCOL::CRC);

ProtocolInfo* TEST_PROTOCOL::Info()
{
	return &mInfo;
}

bool TestMessage::Get(pbb::DataChain& data, uint32_t protocol)
{
	switch (protocol)
	{
	case BinaryDecoder::ID:
			BinaryDecoder decoder(data);
			return Get(decoder);
	}
	return false;
}

bool TestMessage::Put(pbb::DataChain & data, uint32_t protocol)
{
	switch (protocol)
	{
	case BinaryEncoder::ID:
		BinaryEncoder encoder(data);
		return Put(encoder);
	}
	return false;
}

bool TestMessage::Put(pbb::msg::BinaryEncoder& encoder)
{
	encoder.BeginMessage(CODE, NAME);
	encoder.Put("data", this->data);
	return true;
}

bool TestMessage::Get(BinaryDecoder& decoder)
{	
	decoder.Get("data", this->data);
	return true;
}