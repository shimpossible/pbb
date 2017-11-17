#include "pbb/msg/TCPTransportProtocol.h"

namespace pbb {
namespace msg {


ProtocolInfo mInfo("TCPTransportProtocol", TCPTransportProtocol::CRC);

ProtocolInfo* TCPTransportProtocol::Info()
{
	return &mInfo;
}

Message* TCPTransportProtocol::CreateMessage(uint32_t code)
{
	switch (code)
	{
	case AddProtocol::CODE: return new AddProtocol();
	case EncoderRequest::CODE: return new EncoderRequest();
	case EncoderResponse::CODE: return new EncoderResponse();
	}
	return nullptr;
}


const char* AddProtocol::NAME = "AddProtocol";

}
}