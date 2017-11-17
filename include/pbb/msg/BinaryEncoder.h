#ifndef __PBB_MSG_BINARY_ENCODER_H__
#define __PBB_MSG_BINARY_ENCODER_H__
#include <pbb/pbb.h>
#include <pbb/DataChain.h>
#include "Message.h"
#include "Array.h"

namespace pbb {
namespace msg {

/**
7 bit integer encoding, LSByte first
*/
uint32_t PBB_API Encode7Bit(uint32_t val, int& bytes);

class PBB_API BinaryEncoder
{
public:
	const static uint32_t ID = 0xCCCCCCCC;

	BinaryEncoder(DataChain& data)
		: m_Data(data)
	{

	}

	bool BeginMessage(MessageID code, const char* name)
	{
		return true;
	}

	bool EndMessage(uint32_t code, const char* name)
	{
		return true;
	}

	bool Put(const char* name, const char* value);

	template<typename T>
	bool Put(const char* name, T& value)
	{
		return m_Data.AddTail(&value, sizeof(value)) == sizeof(value);
	}

	template<typename T>
	bool Put(const char* name, Array<T>& value)
	{
		int bytes;
		uint32_t encodedLen = Encode7Bit(value.Length(), bytes);

		m_Data.AddTail(&encodedLen, bytes);

		for (uint32_t i = 0;i < value.Length();i++)
		{
			T& t = value[i];
			Put("", t);
		}
		return true;
	}

	template<typename T, uint32_t L>
	bool Put(const char* name, StaticArray<T,L>& value)
	{
		int bytes;
		uint32_t encodedLen = Encode7Bit(value.Length(), bytes);

		m_Data.AddTail(&encodedLen, bytes);

		for (uint32_t i = 0;i < value.Length();i++)
		{
			T& t = value[i];
			Put("", t);
		}
		return true;
	}

protected:

	DataChain& m_Data;
};

}
}

#endif /* __PBB_MSG_DECODER_H__ */