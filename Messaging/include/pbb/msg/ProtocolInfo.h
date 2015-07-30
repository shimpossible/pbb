#ifndef __PBB_MSG_PROTOCOL_INFO_H__
#define __PBB_MSG_PROTOCOL_INFO_H__

#include <functional>
#include <pbb/pbb.h>

namespace pbb {
namespace msg {

class ProtocolInfo
{
public:
	ProtocolInfo(const char* name, uint32_t crc)
	{
		m_Name = name;
		m_CRC = crc;

		// Precompute hash, since it doesnt change
		std::string s;
		s.append(m_Name);
		s.append((char*)&m_CRC, 4);
		std::hash<std::string> fn;
		m_Hash = fn(s);
	}
	uint32_t Hash() const
	{
		return m_Hash;
	}
	const char* m_Name;
	uint32_t    m_CRC;
	uint32_t    m_Encoding;
	uint32_t    m_Hash;
};

}
}
#endif