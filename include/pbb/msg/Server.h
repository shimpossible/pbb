#ifndef __PBB_MSG_SERVER_H__
#define __PBB_MSG_SERVER_H__

#include "pbb/msg/RouteConfig.h"
#include "pbb/msg/MessageQueue.h"

namespace pbb {
namespace msg {

class PBB_API Server 
{
public:
    RouteConfig& GetRouteConfig();

    void Send(Link& link, Message* msg);
    void Broadcast(Link& link, Message* msg);

    void ProcessQueue(MessageQueue* queue, void* ctx);
protected:

    RouteConfig  mConfig;
private:

};

}
}
#endif /* __PBB_MSG_SERVER_H__ */