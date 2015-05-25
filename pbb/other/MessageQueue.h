#ifndef __MESSAGE_QUEUE_H__
#define __MESSAGE_QUEUE_H__

#include "oe_types.h"
#include "message.h"

#include <queue>

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


#endif /* __MESSAGE_QUEUE_H__ */
