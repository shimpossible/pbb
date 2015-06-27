#ifndef __PBB_MESSAGE_HANDLER_COLLECTION_H__
#define __PBB_MESSAGE_HANDLER_COLLECTION_H__
#include <map>
#include <list>

namespace pbb
{

// TODO: make this a NOP in Doxugen
#ifdef PBB_OS_IS_WINDOWS
  #define CDECL __cdecl
#else
  #define CDECL __attribute__((cdecl))
#endif

    /**
        Handler for received messages
        @param ctx   Opaque data to pass to handler
        @param link  sender of message
        @param msg   The message itself
     */
    typedef void(CDECL *MessageHandler)(void* ctx, Link&, Message*);
    typedef Message* (CDECL *MessageFactory)(uint32_t code);

    class PBB_API MessageHandlerCollection
    {
    public:

        /**
            Add a new handler for the given protocol
            @param crc    crc of protocol
            @param ctx    Data to pass to fptr function
            @param fptr   Function to call
         */
        void Add(uint32_t crc, MessageFactory* factory, void* ctx, MessageHandler* fptr)
        {
            mFactory[crc] = factory;
            mHandlers[crc].Add(ctx, fptr);
        }

        /**
          Create a message based on protocol and message code
          @param protocol CRC of protocol
          @param code     message id in given protocol
          @returns 0 if there was no match
         */
        Message* Create(uint32_t protocol, uint32_t code) const
        {
            Message* msg = 0;
            std::map<uint32_t, MessageFactory*>::const_iterator it = mFactory.find(protocol);
            if (it != mFactory.end())
            {
                msg = ( *(it->second) )(code);
            }
            return msg;
        }
        void Dispatch(Link& link, Message* msg) const
        {
            // Send a message to ALL clients that care about 
            uint32_t crc = msg->GetProtcolCRC();
            std::map<uint32_t, DelegateList>::const_iterator it = mHandlers.find(crc);
            if (it != mHandlers.end())
            {
                (it->second).ForEach(link, msg);
            }
        }
    protected:
        /**
        List of handlers allowing for easy enumeration
        */
        class DelegateList
        {
        public:
            void Add(void* ctx, MessageHandler* fptr)
            {
                T t;
                t.ctx = ctx;
                t.fptr = fptr;
                mData.push_back(t);
            }

            void ForEach(Link& link, Message* msg) const
            {
                std::list<T>::const_iterator it;
                for (it = mData.begin();
                     it != mData.end();
                     it++)
                {
                    MessageHandler* f = it->fptr;
                    (*f)(it->ctx, link, msg);
                }
            }
        protected:

            struct T 
            {
                void* ctx;
                MessageHandler* fptr;
            };
            std::list<T> mData;
        };

        std::map<uint32_t, DelegateList> mHandlers;
        std::map<uint32_t, MessageFactory*> mFactory;
    private:
    };

} /* namespace pbb */
#endif
