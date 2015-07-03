#include "pbb/TCPTransport.h"
#include "pbb/RouteConfig.h"

namespace pbb {
namespace msg {

TCPServer::ClientSocketMap     TCPServer::s_Clients;
TCPServer::ServerSocketMap     TCPServer::s_KnownConnections;

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