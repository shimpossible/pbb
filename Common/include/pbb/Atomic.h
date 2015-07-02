#ifndef __ATOMIC_H__
#define __ATOMIC_H__
#include <pbb/pbb.h>

#ifdef PBB_OS_IS_WINDOWS
  #include <intrin.h>
#else
#endif
namespace pbb {

template<typename T>
class PBB_API Atomic
{
public:
    Atomic(const T& t)
        :mT(t)
    {}

    operator T() const { return mT; }

    inline Atomic& operator++();
    inline Atomic& operator--();

protected:
private:
    T mT;
};

template<> Atomic<int32_t>& Atomic<int32_t>::operator ++();
template<> Atomic<int32_t>& Atomic<int32_t>::operator--();

} /* namespace pbb */
#endif /* __ATOMIC_H__ */
