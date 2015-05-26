#include <pbb/thread.h>
#include <assert.h>

#define LEAN_AND_MEAN
#include <Windows.h>
#include <process.h>

/// See <http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx>
const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void SetThreadName(DWORD dwThreadID, const char* threadName)
{
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = threadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;

    __try
    {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
    }
}

class TLSHolder
{
public:
    TLSHolder()
        : mSlot( TlsAlloc() )
    {        
        assert(mSlot != TLS_OUT_OF_INDEXES);
    }
    ~TLSHolder()
    {
        TlsFree(mSlot);
    }

    void set(pbb::Thread* thread) const
    {
        TlsSetValue(mSlot, thread);
    }
    pbb::Thread* get() const
    {
        return reinterpret_cast<pbb::Thread*>(TlsGetValue(mSlot));
    }
protected:
    DWORD mSlot;
};

const TLSHolder sHolder;

pbb::Thread* pbb::Thread::Current()
{
    return sHolder.get();    
}


pbb::Thread::Thread()
    : mThread(0)
{
    
}
pbb::Thread::~Thread()
{
    if(mThread) CloseHandle(mThread);
    mThread = 0;
}
void pbb::Thread::StartThread(uint32_t(__stdcall *start)(void*),
    uint32_t stack, int32_t priority, uint32_t affinity, const char* name)
{
    // http://www.viva64.com/en/d/0102/print/
    mThread = (HANDLE)_beginthreadex(NULL, stack, start, this, 0, &mThreadId);
}

const char* pbb::Thread::GetName()
{
    return mName;
}

uint32_t pbb::Thread::Join()
{
    assert(mThread);
    
    WaitForSingleObject(mThread, INFINITE);
    //DWORD exitCode = 0;
    //GetExitCodeThread(mThread, &exitCode);
    CloseHandle(mThread);
    mThread = 0;
    return 0;
}

void pbb::Thread::SetPriority(int32_t priority)
{
    SetThreadPriority(mThread, priority);
}

void pbb::Thread::SetName(const char* name)
{
    assert(mThreadId);

    SetThreadName(mThreadId, name);
}