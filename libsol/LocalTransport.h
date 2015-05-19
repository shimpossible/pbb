#ifndef __LOCAL_TRANSPORT_H__
#define __LOCAL_TRANSPORT_H__

#include "oe_types.h"
#include "RouteConfig.h"
#include "ITransport.h"
#include "Message.h"

/**
Transports the message to the local endpoints
*/
class LocalTransport : public ITransport
{
public:
    LocalTransport(RouteConfig& config);

    /**
    Called by RouteConfig when sending a message.  To Transmit
    outgoing message to all local endpoints
    */
    virtual void Transmit(Link& link, Message* msg);

    virtual void ConfigureOutbound(uint32_t crc);
protected:

    /**
      The routeconfig we are configured with
     */
    RouteConfig& mConfig;

    /**
    Create a clone of a message based on protocol and id
    */
    Message* Clone(Message* msg);
private:
};

#endif