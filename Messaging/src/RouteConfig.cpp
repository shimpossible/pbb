#include "pbb/msg/RouteConfig.h"

namespace pbb {
namespace msg {

RouteConfig::RouteConfig()
    : mLocalTransport(this->mIncommingHandlers)
{
    ConfigureTransport(mLocalTransport);
}

RouteConfig::~RouteConfig()
{
}
/**
* Configure a Transport for this RouteConfig
* @param tport  Instance of a ITransport class
*/
void RouteConfig::ConfigureTransport(ITransport& tport)
{
    mTransport.push_back(&tport);
}


/**
Send a message to all handlers registers
for the protocol of the messages
@param src  Source of message
@param msg  Message to send
*/
void RouteConfig::Send(Link& src, Message* msg)
{
    // Send a message to ALL clients that care about 
    std::list<ITransport*>::iterator it;
    for (it = mTransport.begin();
    it != mTransport.end();
        it++)
    {
        (*it)->Transmit(src, msg);
    }
}

} /* namespace msg */
} /* namespace pbb */