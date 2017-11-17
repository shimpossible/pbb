#include <pbb/net/Net.h>
#ifdef _MSC_VER
#include <WinSock2.h>
#endif
#include <stdio.h>

namespace pbb {
namespace net {

        Initializer::Initializer()
		{
#ifdef WIN32
			WSADATA wsaData;
			WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
		}


    //const Initializer init;

    class Base
    {
    protected:
        Base();
    };

    Base::Base()
    {
        printf("Base!!");
    }
    
        

}
}
