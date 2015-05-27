#ifndef __ATOMIC_H__
#define __ATOMIC_H__

#include <intrin.h>


template<typename T>
class Atomic
{
public:
    Atomic(const T& t)
        :mT(t)
    {}

    operator T() const { return mT;  }

    inline Atomic& operator++();
    inline Atomic& operator--();

protected:
private:
    T mT;
};

template<>
Atomic<int32_t>& Atomic<int32_t>::operator ++()
{
    _InterlockedIncrement((volatile long*)&mT);
    return *this;
}
template<>
Atomic<int32_t>& Atomic<int32_t>::operator--()
{
    _InterlockedDecrement((volatile long*)&mT);
    return *this;
}

#endif
