#include "pbb/LocalTransport.h"
#include "pbb/RouteConfig.h"

namespace pbb
{
    LocalTransport::LocalTransport(const MessageHandlerCollection& handlers)
        : mHandlers(handlers)
    {
    }

    void LocalTransport::Transmit(Link& link, Message* msg)
    {
        //TODO: Create a ref counted copy of the message and forward it
        Message* clone = Clone(msg);
        mHandlers.Dispatch(link, clone);
        clone->Release();
    }

    void LocalTransport::ConfigureOutbound(uint32_t crc)
    {
        // Since this is local, we dont have to do anything
    }

    /**
    Create a clone of a message based on protocol and id
    */
    Message* LocalTransport::Clone(Message* msg)
    {
        uint32_t proto = msg->GetProtcolCRC();
        uint32_t code = msg->GetCode();
        Message* cloned = GetMessage(proto, code);

        cloned->Copy(msg);
        return cloned;
    }

    Message* LocalTransport::GetMessage(uint32_t protocol, uint32_t code)
    {
        Message* message = 0;
        std::map<uint32_t, std::list<Message*> >::iterator it;
        it = mMessagePool.find(protocol);

        if (it != mMessagePool.end())
        {
            if (it->second.size())
            {
                // free message in pool, use that
                message = it->second.front();
                it->second.pop_front();
            }
        }
        else
        {
            // initialize a list
            mMessagePool[protocol].clear();
        }
        if(message == 0)
        {
            // no more messages, create a new one
            message = mHandlers.Create(protocol, code);
            message->AddToPool(*this);
        }

        message->AddRef();
        return message;
    }
    void LocalTransport::ReleaseMessage(Message* msg)
    {
        std::map<uint32_t, std::list<Message*> >::iterator it;
        it = mMessagePool.find(msg->GetProtcolCRC());

        // Tried to release message not part of pool
        assert(it != mMessagePool.end());

        if (it != mMessagePool.end())
        {
            it->second.push_back(msg);
        }
    }

} /* namespace pbb */
