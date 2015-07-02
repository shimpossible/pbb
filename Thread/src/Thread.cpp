#include <pbb/Thread.h>
#include <assert.h>
#include <stdio.h>
#ifdef PBB_OS_IS_WINDOWS
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
#else
    #include <pthread.h>
    // linux
    #define SetThreadName pthread_setname_np
#endif

#ifdef PBB_OS_IS_WINDOWS
	#define TLSKEY DWORD
	int TLS_CREATE_KEY(TLSKEY& x)
        {
           x = TlsAlloc();
           return x == TLS_OUT_OF_INDEXES;
        }
	#define TLS_RELEASE_KEY(x) TlsFree(x)
	#define TLS_SET_KEY(x,y)   TlsSetValue((x), (y))
	#define TLS_GET_KEY(x)   TlsGetValue((x))
#else
	#define TLSKEY pthread_key_t
	int TLS_CREATE_KEY(TLSKEY& x)
        {
           return pthread_key_create(&x, NULL);
        }
	#define TLS_RELEASE_KEY(x) pthread_key_delete( x )
	#define TLS_SET_KEY(x,y)   pthread_setspecific((x), (y))
	#define TLS_GET_KEY(x)   pthread_getspecific((x))
#endif

class TLSHolder
{
public:
    TLSHolder()
    {
	// success
        assert( TLS_CREATE_KEY(mSlot) == 0);
    }
    ~TLSHolder()
    {
        TLS_RELEASE_KEY(mSlot);
    }

    void set(pbb::Thread* thread) const
    {
        TLS_SET_KEY(mSlot, thread);
    }
    pbb::Thread* get() const
    {
        return reinterpret_cast<pbb::Thread*>(TLS_GET_KEY(mSlot));
    }
protected:
    TLSKEY mSlot;
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
    if (mThread)
    {
        // Wait for thread to end
        Join();
#ifdef PBB_OS_IS_WINDOWS
        CloseHandle(mThread);
#else
        pthread_detach(mThread);
#endif
    }
    mThread = 0;
}
void pbb::Thread::StartThread(uint32_t(__stdcall *start)(void*),
    uint32_t stack, int32_t priority, uint32_t affinity, const char* name)
{
    this->mName = name;
    this->mPriority = priority;
    // http://www.viva64.com/en/d/0102/print/
#ifdef PBB_OS_IS_WINDOWS
    mThread = (pbb_thread_t)_beginthreadex(NULL, stack, start, this, 0, &mThreadId);
#else
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    int r =pthread_create(&mThread, &attr, (void* (*)(void*))start, this);
#endif
}

const char* pbb::Thread::GetName()
{
    return mName;
}

uint32_t pbb::Thread::Join()
{
    assert(mThread);
 
#ifdef PBB_OS_IS_WINDOWS   
    WaitForSingleObject(mThread, INFINITE);
    //DWORD exitCode = 0;
    //GetExitCodeThread(mThread, &exitCode);
    CloseHandle(mThread);
#else
    pthread_join(mThread, 0);
#endif
    mThread = 0;
    return 0;
}

void pbb::Thread::SetPriority(int32_t priority)
{
#ifdef PBB_OS_IS_WINDOWS
    SetThreadPriority(mThread, priority);
#else
    pthread_setschedprio(mThread, priority);
#endif
}

void pbb::Thread::SetName(const char* name)
{
    assert(mThreadId);
    if(name==0) name = "";
#ifdef PBB_OS_IS_WINDOWS
    SetThreadName(mThreadId, name);
#else
    SetThreadName(mThread, name)
#endif
}
