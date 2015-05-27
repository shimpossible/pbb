#ifndef __MESSAGE_QUEUE_H__
#define __MESSAGE_QUEUE_H__

#include "Link.h"
#include "Message.h"

#include <queue>

namespace pbb {
    class MessageQueue
    {
    public:

        bool Empty();

        void Enqueue(Link& link, Message* msg);

        void Dequeue(Link& link, Message*& msg);

    protected:
        struct Tuple
        {
            Link& link;
            Message* msg;
        };

        std::queue<Tuple> mQueue;
    };
} /* namespace pbb */

#endif /* __MESSAGE_QUEUE_H__ */
