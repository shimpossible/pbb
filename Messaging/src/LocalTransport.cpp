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
        Message* cloned = mHandlers.Create(proto, code);
        cloned->Copy(msg);
        return cloned;
    }

} /* namespace pbb */