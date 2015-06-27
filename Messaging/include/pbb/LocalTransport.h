#ifndef __PBB_LOCAL_TRANSPORT_H__
#define __PBB_LOCAL_TRANSPORT_H__

//#include "RouteConfig.h"
#include "ITransport.h"
#include "Message.h"
#include "MessageHandlerCollection.h"

namespace pbb {

    /**
    Transports the message to the local endpoints
    */
    class PBB_API LocalTransport : public ITransport , IMessagePool
    {
    public:
        /**
            New LocalTransport
            @param handlers Functsion to call when a message is received
         */
        LocalTransport(const MessageHandlerCollection& handlers);

        /**
        Called by RouteConfig when sending a message.  To Transmit
        outgoing message to all local endpoints
        */
        virtual void Transmit(Link& link, Message* msg);

        virtual void ConfigureOutbound(uint32_t crc);

        virtual Message* GetMessage(uint32_t protocol, uint32_t code);
        virtual void ReleaseMessage(Message* msg);
    protected:

        /**
          The routeconfig we are configured with
         */
        const MessageHandlerCollection& mHandlers;

        /**
        Create a clone of a message based on protocol and id
        */
        Message* Clone(Message* msg);

        std::map<uint32_t, std::list<Message*> > mMessagePool;
    private:
    };

}/* namespace pbb*/
#endif /* __PBB_LOCAL_TRANSPORT_H__ */
