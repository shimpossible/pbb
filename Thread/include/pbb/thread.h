#ifndef __PBB_THREAD_H__
#define __PBB_THREAD_H__
#include <pbb/pbb.h>
#include <pbb/delegate.h>

namespace pbb
{

    class PBB_API IRunnable
    {
    public:
        virtual ~IRunnable() {}
        // Must be implemented
        virtual void Run() = 0;
        // Implement if thread supports cancelation
        virtual void Cancel() {};
    };


    class PBB_API Thread
    {
    public:

        Thread();
        ~Thread();
        void SetName(const char* name);
        const char* GetName();

        /**
        Wait for thread to finish
        @returns 0 on success
        */
        uint32_t Join();

        void SetPriority(int32_t priority);

        void Run();

        template<typename RunnableT>
        void Start(RunnableT& runnable, uint32_t stack=0, uint32_t priority=0, uint32_t affinity=-1, const char* name = NULL)
        {
            mDelegate = delegate::create<RunnableT>(runnable, &RunnableT::Run);
            StartThread(ThreadStart, stack, priority, affinity, name);
        }
        static Thread* Current();
    protected:

        bool mIsStarted;
        bool mIsRunning;
    private:
        delegate*   mDelegate;
        uint32_t    mPriority;
        void*       mThread;  // Opaque thread handle
        uint32_t    mThreadId;
        const char* mName;

        void StartThread(uint32_t(__stdcall *start)(void*),
            uint32_t stack, int32_t priority, uint32_t affinity, const char* name = NULL);

        static unsigned __stdcall ThreadStart(void* data)
        {
            Thread* self = (Thread*)data;
            self->SetPriority(self->mPriority);

            self->SetName(self->mName);
            self->mIsStarted = true;
            self->mIsRunning = true;
            self->mDelegate->invoke();
            self->mIsRunning = false;
            return 0;
        }

    };
} // namespace pbb
#endif /* __PBB_THREAD_H__ */