#ifndef __PBB_LINK_H__
#define __PBB_LINK_H__
#include <pbb/pbb.h>

namespace pbb {

    class Link
    {
    public:
        uint32_t type;
        bool     local;
    };
}

#endif /* __PBB_LINK_H__ */