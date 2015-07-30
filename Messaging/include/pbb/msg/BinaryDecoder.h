#ifndef __PBB_MSG_DECODER_H__
#define __PBB_MSG_DECODER_H__
#include <pbb/pbb.h>
#include <pbb/DataChain.h>

namespace pbb {
namespace msg {

class BinaryDecoder
{
public:
	BinaryDecoder(DataChain& data)
		: m_Data(data)
	{

	}
	void Get(const char* name, uint8_t& value)
	{
		m_Data.Shift(&value, sizeof(value));
	}
	void Get(const char* name, uint16_t& value)
	{
		m_Data.Shift(&value, sizeof(value));
	}
	void Get(const char* name, uint32_t& value)
	{
		m_Data.Shift(&value, sizeof(value));
	}
	void Get(const char* name, uint64_t& value)
	{
		m_Data.Shift(&value, sizeof(value));
	}

	void Get(const char* name, int8_t& value)
	{
		m_Data.Shift(&value, sizeof(value));
	}
	void Get(const char* name, int16_t& value)
	{
		m_Data.Shift(&value, sizeof(value));
	}
	void Get(const char* name, int32_t& value)
	{
		m_Data.Shift(&value, sizeof(value));
	}
	void Get(const char* name, int64_t& value)
	{
		m_Data.Shift(&value, sizeof(value));
	}

	void Get(const char* name, float& value)
	{
		m_Data.Shift(&value, sizeof(value));
	}
	void Get(const char* name, double& value)
	{
		m_Data.Shift(&value, sizeof(value));
	}

	/**
	  Generic method to decode a custom type
	 */
	template<typename T>
	virtual void Get(const char*name, T& value)
	{
		value.Get(this);
	}
protected:

	DataChain& m_Data;
};

}
}

#endif /* __PBB_MSG_DECODER_H__ */