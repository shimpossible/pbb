#include "LocalTransport.h"
#include "RouteConfig.h"

namespace pbb
{
    LocalTransport::LocalTransport(RouteConfig& config)
        : mConfig(config)
    {
        mConfig.ConfigureTransport(this);
    }

    void LocalTransport::Transmit(Link& link, Message* msg)
    {
        //TODO: Create a ref counted copy of the message and forward it
        Message* clone = Clone(msg);
        mConfig.OnReceive(link, clone);
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
        Message* cloned = mConfig.CreateMessage(proto, code);
        cloned->Copy(msg);
        return cloned;
    }

} /* namespace pbb */