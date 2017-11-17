#include <pbb/Atomic.h>

namespace pbb {

Atomic<int32_t>& Atomic<int32_t>::operator ++()
{
#ifdef PBB_OS_IS_WINDOWS
    _InterlockedIncrement((volatile long*)&mT);
#else
    __sync_add_and_fetch(&mT,1);
#endif
    return *this;
}

Atomic<int32_t>& Atomic<int32_t>::operator--()
{
#ifdef PBB_OS_IS_WINDOWS
    _InterlockedDecrement((volatile long*)&mT);
#else
    __sync_sub_and_fetch(&mT, 1);
#endif
    return *this;
}


Atomic<int32_t>& Atomic<int32_t>::operator ++(int)
{
#ifdef PBB_OS_IS_WINDOWS
    _InterlockedIncrement((volatile long*)&mT);
#else
    __sync_add_and_fetch(&mT, 1);
#endif
    return *this;
}

Atomic<int32_t>& Atomic<int32_t>::operator--(int)
{
#ifdef PBB_OS_IS_WINDOWS
    _InterlockedDecrement((volatile long*)&mT);
#else
    __sync_sub_and_fetch(&mT, 1);
#endif
    return *this;
}

} // namespace pbb
