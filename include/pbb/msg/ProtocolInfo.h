#ifndef __PBB_MSG_PROTOCOL_INFO_H__
#define __PBB_MSG_PROTOCOL_INFO_H__

#include <functional>
#include <pbb/pbb.h>
#include "BinaryDecoder.h"
#include "BinaryEncoder.h"

namespace pbb {
namespace msg {

class PBB_API ProtocolInfo
{
public:
	ProtocolInfo() {};
	ProtocolInfo(const char* name, uint32_t crc)
	{
		Name = name;
		CRC = crc;

		// Precompute hash, since it doesnt change
		std::string s;
		s.append(Name);
		s.append((char*)&CRC, 4);
		std::hash<std::string> fn;
		m_Hash = fn(s);
	}

	bool Get(BinaryDecoder& decoder)
	{
		decoder.Get("Name", (char*&)Name);
		decoder.Get("CRC",  CRC);

		// Precompute hash, since it doesnt change
		std::string s;
		s.append(Name);
		s.append((char*)&CRC, 4);
		std::hash<std::string> fn;
		m_Hash = fn(s);
		return true;
	}

	bool Put(BinaryEncoder& encoder)
	{
		encoder.Put("Name", Name);
		encoder.Put("CRC", CRC);

		return true;
	}

    size_t Hash() const
	{
		return m_Hash;
	}

	const char* Name;
	uint32_t    CRC;
protected:

    size_t    m_Hash;
};

template<>
inline bool BinaryEncoder::Put<ProtocolInfo>(const char* name, ProtocolInfo& value)
{
	return value.Put(*this);
}

}
}
#endif