#include <pbb/net/Net.h>
#ifdef _MSC_VER
#include <WinSock2.h>
#endif

namespace pbb {
namespace net {

	class Initializer
	{
	public:
		Initializer()
		{
#ifdef WIN32
			WSADATA wsaData;
			WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
		}
	};

	static Initializer init;
}
}
