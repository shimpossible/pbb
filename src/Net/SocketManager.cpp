#include <pbb/net/SocketManager.h>

using namespace pbb::net;

SocketManager::SocketManager()
    : mKnownSockets(nullptr)
{

}
SocketManager::~SocketManager()
{
    //TODO: close all sockets

    SocketControlBlock* curr = mKnownSockets;
    while (curr != nullptr)
    {
        SocketControlBlock* last = curr;
        curr->socket->Close();
        curr = curr->next;

        delete last;
    }
}

Error SocketManager::Close(Socket* sock)
{
    SocketControlBlock* curr = mKnownSockets;
    SocketControlBlock* last = nullptr;
    while (curr != nullptr)
    {
        if (curr->socket == sock)
        {
            // Remove from list
            if (last != nullptr)
            {
                last->next = curr->next;
            }
            else
            {
                mKnownSockets = curr->next;
            }

            delete curr;
            return sock->Close();

        }
        last = curr;
        curr = curr->next;
    }

    return PBB_ESUCCESS;
}

Socket* SocketManager::ConnectTo(const char* address, uint16_t port, SocketManager::ISocketCallback& ops)
{
    // TODO: check if its ipv4, ipv6, or dns entry..
    SocketAddress addr(SocketAddress::INET, address, port);
    return ConnectTo(addr, ops);
}

Socket* SocketManager::ConnectTo(SocketAddress& address, SocketManager::ISocketCallback& ops)
{
    Error e;
    // TODO: allow user to specify TCP or UDP
    Socket* clientSock = Socket::Create(address.AddressFamily(), Socket::TCP);
    if (clientSock != Socket::InvalidSocket)
    {
        State state = PENDING_CONNECTION;
        e = clientSock->SetBlocking(false); if (e != PBB_ESUCCESS) goto error;
        e = clientSock->Connect(address);   
        switch (e)
        {
        case PBB_ESUCCESS: // connected..
            state = CONNECTED;
        case PBB_EAGAIN: // Connection in progress
            break;
        default:
            goto error;
        }

        AddSocket(clientSock, state, ops);
        ops.state_changed(clientSock, state);
    }

    goto fin;
error:
    if (clientSock != Socket::InvalidSocket)
    {
        clientSock->Close();
        clientSock = (Socket*)Socket::InvalidSocket;
    }
fin:
    return clientSock;
}

Socket* SocketManager::OpenAndListen(uint16_t port, SocketManager::ISocketCallback& ops)
{
    Socket* sock = Socket::Create(SocketAddress::INET, Socket::TCP);

	SocketAddress addrAny(SocketAddress::INET, "0.0.0.0", port);
	
    sock->Bind(addrAny, true);
    sock->Listen(10);
    sock->SetBlocking(false);

    AddSocket(sock, LISTENING, ops);
    ops.state_changed(sock, LISTENING);

    return sock;
}

void SocketManager::AddSocket(Socket* socket, SocketManager::State state, SocketManager::ISocketCallback& callbacks)
{
    SocketControlBlock* scb = new SocketControlBlock(state,callbacks);
    //scb->ops      = callbacks;
    scb->socket   = socket;

    // TODO: compare and swap
    scb->next = mKnownSockets;
    mKnownSockets = scb;
}

void SocketManager::Update()
{
    SocketControlBlock* curr = mKnownSockets;

    while (curr != nullptr)
    {
        switch(curr->state)
        {
        case LISTENING:
            UpdateListening(curr);
            break;
        case DNS_LOOKUP:
            UpdateDnsLookup(curr);
            break;
        case PENDING_CONNECTION:
            UpdatePending(curr);
            break;
        case CONNECTED: 
            UpdateConnected(curr);
            break;
        }

        curr = curr->next;
    }
}

bool SocketManager::UpdateConnected(SocketControlBlock* scb)
{
    int recv_length = sizeof(mRecvBuffer);
    int bytesReceived;
    Error e = scb->socket->Receive(mRecvBuffer, recv_length, bytesReceived);

    // No data yet
    if (e == PBB_EWOULDBLOCK || e == PBB_EAGAIN)
    {
        return false;
    }
    else if (e == PBB_ESUCCESS)
    {
        // read all available data, and it was 0. socket is closed
        if(bytesReceived <= 0) return true;

        scb->ops.received(scb->socket, mRecvBuffer, bytesReceived);
        return false;
    }
    else
    {
        // other error
        return true;
    }
}

bool SocketManager::UpdatePending(SocketControlBlock* scb)
{
    Error e = scb->socket->GetError();
    switch(e)
    {
    // not connected yet
    case PBB_EINPROGRESS: return false;
    // connected with no errors
    case PBB_ESUCCESS:
        SocketCollection wr;
        wr.push_back(scb->socket);
        int ready = 0;
        if (scb->socket->Select(0, &wr, 0, 0, ready) == PBB_ESUCCESS)
        {
            if (ready == 1) // connection complete
            {
                scb->state = CONNECTED;
                scb->ops.state_changed(scb->socket, scb->state);
            }
        }
        return false;
    }
    return false;
}
bool SocketManager::UpdateListening(SocketControlBlock* scb)
{
    Socket* other;
    SocketAddress addr;

    // Accept any listen socket
    if (scb->socket->Accept(other, &addr) == PBB_ESUCCESS)
    {
        // got a listening socket, go for it
        other->SetBlocking(false);
        AddSocket(other, CONNECTED, scb->ops);
        scb->ops.accepted(scb->socket, other, addr);
    }

    return false;
}
bool SocketManager::UpdateDnsLookup(SocketControlBlock* scb)
{
    return false;
}