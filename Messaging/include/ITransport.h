#ifndef __ITRANSPORT_H__
#define __ITRANSPORT_H__
#include "oe_types.h"

class ITransport
{
public:
    virtual void Transmit(Link&, Message*) = 0;
    virtual void ConfigureOutbound(uint32_t crc) = 0;

protected:
    virtual ~ITransport() {}
};
#endif
