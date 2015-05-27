#ifndef __PBB_LOCAL_TRANSPORT_H__
#define __PBB_LOCAL_TRANSPORT_H__

//#include "RouteConfig.h"
#include "ITransport.h"
#include "Message.h"

namespace pbb {

    class RouteConfig;
    /**
    Transports the message to the local endpoints
    */
    class LocalTransport : public ITransport
    {
    public:
        static LocalTransport& LocalInstance();

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

}/* namespace pbb*/
#endif /* __PBB_LOCAL_TRANSPORT_H__ */