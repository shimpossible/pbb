#include "pbb/Message.h"

pbb::Message::Message()
    : mRefCount(0)
    , mPool(0)
{
}

pbb::Message::~Message()
{
}
