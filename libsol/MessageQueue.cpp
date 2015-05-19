#include "MessageQueue.h"

bool MessageQueue::Empty()
{
    return mQueue.empty();
}
void MessageQueue::Enqueue(Link& link, Message* msg)
{
    msg->AddRef();
    Tuple t = { link, msg };
    mQueue.push(t);
}

void MessageQueue::Dequeue(Link& link, Message*& msg)
{
    Tuple t = mQueue.front();
    mQueue.pop();

    link = t.link;
    msg = t.msg;
}