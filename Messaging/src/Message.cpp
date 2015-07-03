#include "pbb/Message.h"

pbb::msg::Message::Message()
    : mRefCount(0)
    , mPool(0)
{
}

pbb::msg::Message::~Message()
{
}
