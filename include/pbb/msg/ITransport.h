#ifndef __PBB_ITRANSPORT_H__
#define __PBB_ITRANSPORT_H__
#include <pbb/pbb.h>

#include "ProtocolInfo.h"

namespace pbb {
namespace msg {

class Link;
class Message;

class PBB_API ITransport
{
public:
    /**
        Call by RouteConfig when a message is sent
        @param link    Link that sent the message
        @param message Message to be sent.
        */
    virtual void Transmit(Link& link, Message* message) = 0;
    /**
        Notified of Protocols that will be sent on this transport
        RouteConfig will call this whenever RouteConfig::ConfigureOutbound is called
        */
	virtual void ConfigureOutbound(ProtocolInfo* info) = 0;

	virtual void ConfigureInbound(ProtocolInfo* info) = 0;

protected:
    virtual ~ITransport() {}
};

} /* namespace msg */
} /* namespace pbb */
#endif
