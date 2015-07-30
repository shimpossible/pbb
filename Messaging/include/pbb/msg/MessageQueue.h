#ifndef __PBB_MESSAGE_QUEUE_H__
#define __PBB_MESSAGE_QUEUE_H__

#include "Link.h"
#include "Message.h"

#include <queue>

namespace pbb {
namespace msg {

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

} /* namespace msg */
} /* namespace pbb */

#endif /* __PBB_MESSAGE_QUEUE_H__ */
