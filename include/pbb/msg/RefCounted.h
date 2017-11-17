#ifndef __PBB_REF_COUNTED_H__
#define __PBB_REF_COUNTED_H__
#include <stdint.h>
#include "pbb/Atomic.h"
#include <assert.h>

namespace pbb {
namespace msg {

template<typename BaseT>
class RefCounted : public BaseT
{
public:
    virtual void AddRef()
    {
        // Started below 0?
        assert(mRefCount >= 0);

        ++mRefCount;
    }
    virtual void Release()
    {
        --mRefCount;
        // Too many calls to Release
        assert(mRefCount >= 0);
        if (mRefCount == 0)
        {
            delete this;
        }
    }
protected:
    RefCounted()
        : mRefCount(0)
    {
    }
    virtual ~RefCounted() = 0 {}

    RefCounted(const RefCounted&) : mRefCount(0)
    {
    }
    RefCounted& operator=(const RefCounted&)
    {
        return *this;
    }
private:

    Atomic<int32_t> mRefCount;
};

} /* namespace msg */
} /* namespace pbb */
#endif /* __PBB_REF_COUNTED_H__ */