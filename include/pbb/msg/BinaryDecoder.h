#ifndef __PBB_MSG_DECODER_H__
#define __PBB_MSG_DECODER_H__
#include <pbb/pbb.h>
#include <pbb/DataChain.h>

namespace pbb {
namespace msg {

PBB_API uint32_t Decode7Bit(uint8_t* val);

/**
Decode 7bit encoded integer
@param buff Buffer of data
@param len  Number of bytes of data
@param val  Decoded integer
@returns pointer to input buffer, after decoded bytes.  If nothing decoded, returns original buffer
*/
PBB_API uint8_t* Decode7Bit(uint8_t* buff, uint32_t len, uint32_t& val);

class PBB_API BinaryDecoder
{
public:
	const static uint32_t ID = 0xCCCCCCCC;

	BinaryDecoder(DataChain& data)
		: m_Data(data)
	{

	}

	void Get(const char* name, const char*& value);
	/**
	  Allow specialization based on type
	 */
	template<typename T>
	void Get(const char* name, T& value)
	{
		m_Data.Shift(&value, sizeof(value));
	}

protected:

	DataChain& m_Data;
};


}
}

#endif /* __PBB_MSG_DECODER_H__ */