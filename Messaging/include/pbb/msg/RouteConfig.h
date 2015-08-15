#ifndef __PBB_ROUTE_CONFIG_H__
#define __PBB_ROUTE_CONFIG_H__

#include "Link.h"
#include "Message.h"
#include "ITransport.h"
#include "LocalTransport.h"
#include "ProtocolInfo.h"
#include "MessageHandlerCollection.h"
#include <pbb/delegate.h>

#include <list>

namespace pbb {
namespace msg {

/**
  Keeps track of where to route messages and
    Transports to send message out on
 */
class PBB_API RouteConfig
{
public:

	typedef std::list<ProtocolInfo*> ProcolInfoCollection;

    RouteConfig();
    ~RouteConfig();

	Message* CreateMessage(uint32_t protocolCrc, uint32_t msgId);

    /**
        * Configure a Transport for this RouteConfig
        * @param tport  Instance of a ITransport class
        */
    void ConfigureTransport(ITransport& tport);

    template<typename PROTOCOL_T>
    void ConfigureOutbound()
    {
        mOutgoingHandlers.push_back(PROTOCOL_T::CRC);
		ProtocolInfo* info = PROTOCOL_T::Info();
		mOutgoingProtocols.push_back(info);

        // Notify all Transports of the new protocol
        std::list<ITransport*>::iterator it;
        for (it = mTransport.begin();
        it != mTransport.end();
            it++)
        {
            (*it)->ConfigureOutbound(info);
        }
    }

    template<typename PROTOCOL_T>
    void ConfigureInbound(void* ctx, MessageHandler fptr)
    {
        // TODO: wrap access
        mIncommingHandlers.Add(PROTOCOL_T::CRC, PROTOCOL_T::CreateMessage, ctx, fptr);

		ProtocolInfo* info = PROTOCOL_T::Info();
		mIncommingProtocols.push_back(info);

        // Notify all Transports of the new protocol
        std::list<ITransport*>::iterator it;
        for (it = mTransport.begin();
        it != mTransport.end();
            it++)
        {
            (*it)->ConfigureInbound(info);
        }
    }

    /**
    Send a message to all handlers registers
    for the protocol of the messages
    @param src  Source of message
    @param msg  Message to send
    */
    void Send(Link& src, Message* msg);

    const MessageHandlerCollection& MessageHandlers()
    {
        return mIncommingHandlers;
    }

	template<typename T>
	void ForEachIncommingProtocols(void (T::*fptr)(ProtocolInfo*), T* self)
	{
		ProcolInfoCollection::iterator it = mIncommingProtocols.begin();

		for (;it != mIncommingProtocols.end(); it++)
		{
			(self->*fptr)( *it );
		}
	}

	template<typename T>
	void ForEachOutgoingProtocols(void (T::*fptr)(ProtocolInfo*), T* self)
	{
		ProcolInfoCollection::iterator it = mOutgoingProtocols.begin();

		for (;it != mOutgoingProtocols.end(); it++)
		{
			(self->*fptr)( *it );
		}
	}

private:
    // Who to notify when a message is received
    MessageHandlerCollection mIncommingHandlers;

    // Messages that will be sent
    std::list<uint32_t> mOutgoingHandlers;
    // Transports to send messages on
    std::list<ITransport*> mTransport;

	ProcolInfoCollection mIncommingProtocols;
	ProcolInfoCollection mOutgoingProtocols;

    //! Every RouteConfig can route amoung its local members
    LocalTransport  mLocalTransport;
};

} /* namespace msg */
} /* namespace pbb */


namespace std
{
	template<>
	struct hash<pbb::msg::ProtocolInfo>
	{
		std::size_t operator()(pbb::msg::ProtocolInfo const& p) const
		{
			return p.Hash();
		}
	};
}
#endif /* __PBB_ROUTE_CONFIG_H__ */