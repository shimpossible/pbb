#ifndef __PBB_LINK_H__
#define __PBB_LINK_H__
#include <pbb/pbb.h>

namespace pbb {
namespace msg {
    /**
        Identify the source of a message
        */
    class Link
    {
    public:
        uint32_t type;
        bool     local;
    };

    typedef unsigned long long ObjectId;

} // namespace msg
} // namespace pbb
#endif /* __PBB_LINK_H__ */