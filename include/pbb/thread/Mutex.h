#ifndef __PBB_MUTEX_H__
#define __PBB_MUTEX_H__
#include <pbb/pbb.h>
#include <pbb/platform.h>

#ifdef PBB_OS_IS_WINDOWS
    #include <Windows.h>
#else
    #include <pthread.h>
#endif

namespace pbb
{
    class PBB_API Mutex
    {
    public:
        enum Type
        {
            TYPE_RECURSIVE,
            TYPE_NORMAL,    //!< non recusive
        };
        Mutex(Type type);
        ~Mutex();

        void Lock();
        void Unlock();

        /**
            returns false if another thread owns the lock
        */
        bool TryLock();
    protected:
#ifdef PBB_OS_IS_WINDOWS
        CRITICAL_SECTION  mMutex;
        Type              mType;
#else
        pthread_mutex_t   mMutex;
#endif
    private:
        // No Copy
        Mutex(const Mutex&);
        Mutex& operator=(const Mutex&);
    };
}

#endif /* __PBB_MUTEX_H__ */
