#ifndef __PBB_ENDPOINT_H__
#define __PBB_ENDPOINT_H__

#include "RouteConfig.h"
#include "MessageQueue.h"

namespace pbb {
namespace msg {
/**
Endpoint for messages.
*/
class Endpoint
{
public:

    typedef void(__cdecl *DispatchFptr)(Link& link, Message* msg);

    Endpoint(uint32_t type)
        : mConfig(RouteConfig::LocalInstance())
    {
        mLink.type = type;
        mLink.local = true;
    }

    /**
    @param type  Service Type
    */
    Endpoint(uint32_t type, RouteConfig& config)
        : mConfig(config)
    {
        mLink.type = type;
        mLink.local = true;
    }

    /**
    Processes messages from queue
    @param queue  Queue of messages
    */
    void ProcessQueue(MessageQueue& queue)
    {
        Link link;
        Message* msg;
        // Queue has internal locking, so none is needed here
        while (!queue.Empty())
        {
            queue.Dequeue(link, msg);
            uint32_t crc = msg->GetProtcolCRC();
            mDispatch[crc](link, msg);

            // Once out of the Message Queue we can reelase it
            msg->Release();
        }
    }

    /**
    Send a message to all endpoints registered for the
    protocol
    @param msg Message being sent
    */
    void Send(Message* msg)
    {
        // Route local
        mConfig.Send(mLink, msg);
    }

    /**
    Add a message queue for the given handler and Protocol
    */
    template<typename PROTCOL_T>
    void ConfigureInbound(MessageQueue* queue)
    {
        mQueue[PROTCOL_T::CRC] = queue;
    }

    /**
    Register a handler for all messages received for the given protocol
    @param dispatch  Callback to dispatch messages on
    */
    template<typename PROTCOL_T>
    void ConfigureInbound(DispatchFptr fptr)
    {
        mDispatch[PROTCOL_T::CRC] = fptr;
        mConfig.ConfigureInbound<PROTCOL_T>(this, Dispatch);
    }

    /**
        * Register a protocol that will be sent.  This is to allow
        * creation of "channels" on transports
        */
    template<typename PROTCOL_T>
    void ConfigureOutbound()
    {
        mConfig.ConfigureOutbound<PROTCOL_T>();
    }

    static void Dispatch(void* ctx, Link& link, Message* msg)
    {
        Endpoint* self = (Endpoint*)ctx;
        self->Dispatch(link, msg);
    }

    /**
    Dispatch a message to its given dispatchers
    or Enqueue.  This is called by RouteConfig
    */
    void Dispatch(Link& link, Message* msg)
    {
        // TODO: check for queue
        uint32_t crc = msg->GetProtcolCRC();
        std::map<uint32_t, MessageQueue*>::iterator it;
        it = mQueue.find(crc);
        if (it != mQueue.end())
        {
            // Enqueu
            it->second->Enqueue(link, msg);
        }
        else
        {
            // No queue, dispatch directly
            mDispatch[crc](link, msg);
        }
    }

protected:
    Link      mLink;  //!< Local Link

    RouteConfig& mConfig;
    std::map<uint32_t, MessageQueue*> mQueue;
    std::map<uint32_t, DispatchFptr> mDispatch;
};

} /* namespace msg */
} /* namespace pbb */
#endif /* __ENDPOINT_H__ */