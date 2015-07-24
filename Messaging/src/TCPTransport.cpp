#include "pbb/TCPTransport.h"
#include "pbb/RouteConfig.h"

namespace pbb {
namespace msg {

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


TCPServer::TCPServer()
	: m_SocketCallback(*this)
{
}
bool TCPServer::Start(uint16_t port)
{
	m_Socket = m_SocketMgr.OpenAndListen(0x1234, m_SocketCallback);

	return m_Socket != net::Socket::InvalidSocket;
}

bool TCPServer::ConnectTo(const char* address, uint16_t port)
{
	// TODO: connect to remote server given an address and port
	return false;
}

TCPConnection * TCPServer::FindClient(net::pbb_socket_t id)
{
	ClientSocketMap::iterator it = m_KnownConnections.find(id);
	if (it != m_KnownConnections.end()) return it->second;

	return nullptr;
}

void TCPServer::AddPending(net::pbb_socket_t id, TCPConnection * client)
{
	m_KnownConnections[id] = client;

	//TODO: Send connection request
}

void TCPServer::Callbacks::state_changed(pbb::net::Socket * socket, net::SocketManager::State state)
{
	switch (state)
	{
	case net::SocketManager::CONNECTED: // Connected to remote server..
		// TODO: implement
		break;
	}
}

void TCPServer::Callbacks::accepted(pbb::net::Socket * socket, pbb::net::Socket* remote, pbb::net::SocketAddress& address)
{
	// new client connects to server
	TCPConnection* client = new TCPConnection(remote, m_Transport);
	m_Transport.AddPending(*remote, client);
}

void TCPServer::Callbacks::received(pbb::net::Socket * socket, void * data, size_t len)
{
	TCPConnection* client = m_Transport.FindClient(*socket);
	if (client != nullptr)
	{
		client->Receive(data, len);
	}
}

} // namespace msg
} // namespace pbb