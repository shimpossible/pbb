#include "pbb/msg/BinaryEncoder.h"

#include "pbb/msg/ProtocolInfo.h"
namespace pbb {
namespace msg {


bool BinaryEncoder::Put(const char* name, const char* value)
{
	int bytes;
	int len = strlen(value);
	uint32_t data = Encode7Bit(len, bytes);

	m_Data.AddTail(&data, bytes);
	m_Data.AddTail(value, len);

	return false;
}

}
}

