#ifndef __PBB_REF_COUNTED_H__
#define __PBB_REF_COUNTED_H__
#include <stdint.h>
#include "pbb/Atomic.h"
#include <assert.h>

namespace pbb {
    class RefCounted
    {
    public:
        void AddRef()
        {
            // Started below 0?
            assert(mRefCount >= 0);

            ++mRefCount;
        }
        void Release()
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

} /* namespace pbb */
#endif /* __PBB_REF_COUNTED_H__ */