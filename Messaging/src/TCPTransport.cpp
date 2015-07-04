#include "pbb/TCPTransport.h"
#include "pbb/RouteConfig.h"

namespace pbb {
namespace msg {

TCPServer::ClientSocketMap     TCPServer::s_Clients;
TCPServer::ServerSocketMap     TCPServer::s_KnownConnections;

TCPServer::TCPServer()
{
    m_Socket = pbb::net::Socket::Create(net::SocketAddress::INET, net::Socket::TCP);
}
bool TCPServer::Start(uint16_t port)
{
    if (m_Socket->Listen(10) != net::PBB_ESUCCESS) return false;

    // Keep track of connection
    s_KnownConnections[*m_Socket] = this;
    return true;
}

bool TCPServer::ConnectTo(const char* address, uint16_t port)
{
    // TODO: connect to remove server given an address and port
    return false;
}

TCPTransport::TCPTransport(uint16_t port)
{
    // TODO: open socket listening for new connections
    // start Receive Thread
}

void TCPTransport::Transmit(Link& link, Message* msg)
{
    // Loop through all clients that accept the protcolol
    msg->GetProtcolCRC();
}

void TCPTransport::ConfigureOutbound(uint32_t crc)
{
    mOutboundProtocols.push_back(crc);
}

void TCPTransport::ConfigureInbound(uint32_t crc)
{
    mInboundProtocols.push_back(crc);
}

} // namespace msg
} // namespace pbb