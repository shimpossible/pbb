#include <pbb/net/SocketManager.h>

using namespace pbb::net;

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

            return curr->socket->Close();

        }
        curr = curr->next;
    }

    return PBB_ESUCCESS;
}

Socket* SocketManager::OpenAndListen(uint16_t port, SocketManager::SocketCallback& ops)
{
    Socket* sock = Socket::Create(SocketAddress::INET, Socket::TCP);

    SocketAddress addrAny;
    sock->Bind(addrAny, true);
    sock->Listen(10);
    sock->SetBlocking(false);

    if (ops.state_changed)
    {
        ops.state_changed(sock, LISTENING);
    }

    return sock;
}

void SocketManager::AddSocket(Socket* socket, SocketManager::State state, SocketManager::SocketCallback& callbacks)
{
    SocketControlBlock* scb = new SocketControlBlock();
    scb->ops      = callbacks;
    scb->socket   = socket;
    scb->state    = state;

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
            break;
        }

        curr->next;
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
            if (ready == 1)
            {

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
        if (scb->ops.accepted)
        {
            scb->ops.accepted(scb->socket, other, addr);
        }
    }

    return false;
}
bool SocketManager::UpdateDnsLookup(SocketControlBlock* scb)
{
    return false;
}