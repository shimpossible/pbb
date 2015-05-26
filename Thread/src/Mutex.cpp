#include <pbb/Mutex.h>
#include <pbb/thread.h>
#include <Windows.h>

pbb::Mutex::Mutex(pbb::Mutex::Type type)
{
    InitializeCriticalSection(&mMutex);
}

pbb::Mutex::~Mutex()
{
    // TODO: check we own the thread
    DeleteCriticalSection(&mMutex);
}

void pbb::Mutex::Lock()
{
    EnterCriticalSection(&mMutex);
    if (mType == pbb::Mutex::TYPE_NORMAL && mMutex.LockCount > 1)
    {
        // Wait for mutex to be free'd, which shouldn't
        // happen since the calling thread owns it
        // so we'll just block
        WaitForSingleObject( GetCurrentThread(), INFINITE );
    }
}
void pbb::Mutex::Unlock()
{
    LeaveCriticalSection(&mMutex);
}
bool pbb::Mutex::TryLock()
{
    return TryEnterCriticalSection(&mMutex)!=0;
}