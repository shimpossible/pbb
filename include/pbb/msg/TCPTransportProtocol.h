#ifndef __PBB_TCP_TRANSPORT_PROTOCOL_H__
#define __PBB_TCP_TRANSPORT_PROTOCOL_H__


#include "Message.h"
#include "ProtocolInfo.h"

namespace pbb {
namespace msg {

class PBB_API TCPTransportProtocol
{
public:
	static const uint32_t CRC = 0xABCD1234;
	static const char* NAME;
	static Message* CreateMessage(uint32_t code);
	static ProtocolInfo* Info();

	template<typename T>
	static void Dispatch(Message* msg, T* target)
	{
		switch (msg->GetCode())
		{
		case AddProtocol::CODE: target->AddProtocolHandler((AddProtocol*)msg); break;
		default:
			// unknown
			break;
		}
	};

private:
	TCPTransportProtocol();
};

class PBB_API AddProtocol : public Message
{
public:
	const static pbb::msg::MessageID CODE = 2;
	const static char* NAME;

	uint32_t GetProtcolCRC() { return TCPTransportProtocol::CRC; }
	pbb::msg::MessageID GetCode() { return CODE; }

	bool Get(pbb::DataChain& data, uint32_t protocol)
	{
		switch (protocol)
		{
		case BinaryDecoder::ID:
			BinaryDecoder decoder(data);
			return Get(decoder);
		}
		return false;
	}

	bool Put(pbb::DataChain& data, uint32_t protocol)
	{
		switch (protocol)
		{
		case BinaryEncoder::ID:
			BinaryEncoder encoder(data);
			return Put(encoder);
		}
		return false;
	}

	bool Get(pbb::msg::BinaryDecoder& decoder)
	{
		decoder.Get("Name", Name);
		decoder.Get("ProtocolCRC", ProtocolCRC);
		return true;
	}
	bool Put(pbb::msg::BinaryEncoder& encoder) const
	{
		encoder.BeginMessage(CODE, NAME);
		encoder.Put("Name", Name);
		encoder.Put("ProtocolCRC", ProtocolCRC);
		encoder.EndMessage(CODE, NAME);
		return true;
	}

	virtual void Copy(pbb::msg::Message* other)
	{
		if (other == 0 || this == other) return;
		// not same message?
		if (other->GetProtcolCRC() != this->GetProtcolCRC() ||
			other->GetCode() != other->GetCode())
			return;

		AddProtocol* otherMsg = (AddProtocol*)other;

		this->ProtocolCRC = otherMsg->ProtocolCRC;
	}

	/// Start Data
	const char* Name;
	uint32_t ProtocolCRC;
	/// End Data
};

class PBB_API EncoderRequest : public Message
{
public:
	const static pbb::msg::MessageID CODE = 0;
	const static char* NAME;

	uint32_t GetProtcolCRC() { return TCPTransportProtocol::CRC; }
	pbb::msg::MessageID GetCode() { return CODE; }

	bool Get(pbb::DataChain& data, uint32_t protocol)
	{
		switch (protocol)
		{
		case BinaryDecoder::ID:
			BinaryDecoder decoder(data);
			return Get(decoder);
		}
		return false;
	}

	bool Put(pbb::DataChain& data, uint32_t protocol)
	{
		switch (protocol)
		{
		case BinaryEncoder::ID:
			BinaryEncoder encoder(data);
			return Put(encoder);
		}
		return false;
	}

	bool Get(pbb::msg::BinaryDecoder& decoder)
	{
		decoder.Get("ProtocolCRC", ProtocolCRC);
		return true;
	}
	bool Put(pbb::msg::BinaryEncoder& encoder) const
	{
		encoder.BeginMessage(CODE, NAME);
		encoder.Put("ProtocolCRC", ProtocolCRC);
		encoder.EndMessage(CODE, NAME);
		return true;
	}

	virtual void Copy(pbb::msg::Message* other)
	{
		if (other == 0 || this == other) return;
		// not same message?
		if (other->GetProtcolCRC() != this->GetProtcolCRC() ||
			other->GetCode() != other->GetCode())
			return;

		EncoderRequest* otherMsg = (EncoderRequest*)other;

		this->ProtocolCRC = otherMsg->ProtocolCRC;
	}

	/// Start Data
	uint32_t ProtocolCRC;
	/// End Data
};

class PBB_API EncoderResponse : public Message
{
public:
	const static pbb::msg::MessageID CODE = 1;
	const static char* NAME;

	uint32_t GetProtcolCRC() { return TCPTransportProtocol::CRC; }
	pbb::msg::MessageID GetCode() { return CODE; }

	bool Get(pbb::DataChain& data, uint32_t protocol)
	{
		switch (protocol)
		{
		case BinaryDecoder::ID:
			BinaryDecoder decoder(data);
			return Get(decoder);
		}
		return false;
	}
	bool Put(pbb::DataChain& data, uint32_t protocol)
	{
		switch (protocol)
		{
		case BinaryEncoder::ID:
			BinaryEncoder encoder(data);
			return Put(encoder);
		}
		return false;
	}

	bool Get(pbb::msg::BinaryDecoder& decoder)
	{
		decoder.Get("Encoders", Encoders);
		return true;
	}
	bool Put(pbb::msg::BinaryEncoder& encoder) const
	{
		encoder.BeginMessage(CODE, NAME);
		encoder.Put("Encoders", Encoders);
		encoder.EndMessage(CODE, NAME);
		return true;
	}

	virtual void Copy(pbb::msg::Message* other)
	{
		if (other == 0 || this == other) return;
		// not same message?
		if (other->GetProtcolCRC() != this->GetProtcolCRC() ||
			other->GetCode() != other->GetCode())
			return;

		EncoderResponse* otherMsg = (EncoderResponse*)other;

		this->Encoders = otherMsg->Encoders;
	}

	/// Start Data
	Array<uint32_t> Encoders;
	/// End Data
};

}
}

#endif