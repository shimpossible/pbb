#include <pbb/Mutex.h>
#include <pbb/Thread.h>

pbb::Mutex::Mutex(pbb::Mutex::Type type)
{
#ifdef PBB_OS_IS_WINDOWS
    InitializeCriticalSection(&mMutex);
#else
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, type == TYPE_RECURSIVE ? PTHREAD_MUTEX_RECURSIVE 
                                                       : PTHREAD_MUTEX_NORMAL
                             );
    pthread_mutex_init(&mMutex, &attr);

    pthread_mutexattr_destroy(&attr);
#endif
}

pbb::Mutex::~Mutex()
{
#ifdef PBB_OS_IS_WINDOWS
    // TODO: check we own the thread
    DeleteCriticalSection(&mMutex);
#else
    pthread_mutex_destroy(&mMutex);
#endif
}

void pbb::Mutex::Lock()
{
#ifdef PBB_OS_IS_WINDOWS
    EnterCriticalSection(&mMutex);
    if (mType == pbb::Mutex::TYPE_NORMAL && mMutex.LockCount > 1)
    {
        // Wait for mutex to be free'd, which shouldn't
        // happen since the calling thread owns it
        // so we'll just block
        WaitForSingleObject( GetCurrentThread(), INFINITE );
    }
#else
    pthread_mutex_lock(&mMutex);
#endif
}
void pbb::Mutex::Unlock()
{
#ifdef PBB_OS_IS_WINDOWS
    LeaveCriticalSection(&mMutex);
#else
    pthread_mutex_unlock(&mMutex);
#endif
}
bool pbb::Mutex::TryLock()
{
#ifdef PBB_OS_IS_WINDOWS
    // returns non-zero on success
    return TryEnterCriticalSection(&mMutex)!=0;
#else
    // trylock returns 0 on success
    pthread_mutex_trylock(&mMutex)==0;
#endif
}
