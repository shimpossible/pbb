#ifndef __OE_TYPES_H__
#define __OE_TYPES_H__

#include <stdint.h>


class Message;

class Link
{
public:
    uint32_t type;
    bool     local;
};

typedef void(*MessageCallback)(void* ctx, Link&, Message*);

#endif /* __OE_TYPES_H__ */