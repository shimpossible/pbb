#include "RouteConfig.h"

RouteConfig::RouteConfig()
{
}

RouteConfig::~RouteConfig()
{
}
/**
* Configure a Transport for this RouteConfig
* @param tport  Instance of a ITransport class
*/
void RouteConfig::ConfigureTransport(ITransport* tport)
{
    mTransport.push_back(tport);
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

/**
* Create a message given the protocol CRC and code
@param proto CRC of protocol
@param code  message CODE specific to protcol
@returns 0 if message can't be created
*/
Message* RouteConfig::CreateMessage(uint32_t proto, uint32_t code)
{
    std::map<uint32_t, Message* (*)(uint32_t)>::iterator it = mFactory.find(proto);
    if (it != mFactory.end())
    {
        return (it->second)(code);
    }
    else
    {
        return 0;
    }
}

void RouteConfig::OnReceive(Link& src, Message* msg)
{
    // Send a message to ALL clients that care about 
    uint32_t crc = msg->GetProtcolCRC();
    mIncommingHandlers[crc].ForEach(src, msg);
}