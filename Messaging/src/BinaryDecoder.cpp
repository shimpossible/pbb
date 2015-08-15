#include "pbb/msg/BinaryDecoder.h"

namespace pbb {
namespace msg {


void BinaryDecoder::Get(const char* name, const char*& value)
{
	uint32_t len;
	uint8_t* buff1 = m_Data.GetBuffer();
	uint8_t* buff2 = Decode7Bit(buff1, m_Data.Size(), len);
	
	m_Data.Shift(buff2-buff1); // Shift off the 7bit encoded length

	// create buffer for string
	char* p = new char[len+1];
	// copy string from buffer
	m_Data.Shift(p, len);
	p[len] = 0;
	value = p;
}

}
}
