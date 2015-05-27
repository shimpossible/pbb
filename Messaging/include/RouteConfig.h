#ifndef __PBB_ROUTE_CONFIG_H__
#define __PBB_ROUTE_CONFIG_H__

#include "Link.h"
#include "Message.h"
#include "ITransport.h"
#include "LocalTransport.h"

#include <map>
#include <list>

namespace pbb {

    typedef void(*MessageCallback)(void* ctx, Link&, Message*);

    /**
    * Keeps track of where to route messages and
      Transports to send message out on
    */
    class RouteConfig
    {
    public:

        static RouteConfig& LocalInstance();

        RouteConfig();
        ~RouteConfig();

        /**
         * Configure a Transport for this RouteConfig
         * @param tport  Instance of a ITransport class
         */
        void ConfigureTransport(ITransport* tport);

        template<typename PROTOCOL_T>
        void ConfigureOutbound()
        {
            mOutgoingHandlers.push_back(PROTOCOL_T::CRC);
            // Notify all Transports of the new protocol
            std::list<ITransport*>::iterator it;
            for (it = mTransport.begin();
            it != mTransport.end();
                it++)
            {
                (*it)->ConfigureOutbound(PROTOCOL_T::CRC);
            }
        }

        template<typename PROTOCOL_T>
        void ConfigureInbound(void* ctx, MessageCallback fptr)
        {
            mIncommingHandlers[PROTOCOL_T::CRC].Add(ctx, fptr);
            mFactory[PROTOCOL_T::CRC] = &PROTOCOL_T::CreateMessage;
        }

        /**
        Send a message to all handlers registers
        for the protocol of the messages
        @param src  Source of message
        @param msg  Message to send
        */
        void Send(Link& src, Message* msg);

        /**
         * Create a message given the protocol CRC and code
         @param proto CRC of protocol
         @param code  message CODE specific to protcol
         @returns 0 if message can't be created
         */
        Message* CreateMessage(uint32_t proto, uint32_t code);

        /*
        Call when a message is received
        */
        void OnReceive(Link& src, Message* msg);

    protected:

        /**
        List of handlers allowing for easy enumeration
        */
        class DelegateList
        {
        public:
            void Add(void* ctx, MessageCallback fptr)
            {
                T t;
                t.ctx = ctx;
                t.fptr = fptr;
                mData.push_back(t);
            }

            void ForEach(Link& link, Message* msg)
            {
                std::list<T>::iterator it;
                for (it = mData.begin();
                it != mData.end();
                    it++)
                {
                    it->fptr(it->ctx, link, msg);
                }
            }
        protected:

            struct T {
                void* ctx;
                MessageCallback fptr;
            };
            std::list<T> mData;
        };
        // Messages that will be received
        std::map<uint32_t, DelegateList> mIncommingHandlers;
        // Messages that will be sent
        std::list<uint32_t> mOutgoingHandlers;
        // Message Factories
        std::map<uint32_t, Message* (*)(uint32_t)> mFactory;
        // Transports to send messages on
        std::list<ITransport*> mTransport;

        //! Every RouteConfig can route amoung its local members
        LocalTransport  mLocalTransport;
    };

} /* namespace pbb */
#endif /* __PBB_ROUTE_CONFIG_H__ */