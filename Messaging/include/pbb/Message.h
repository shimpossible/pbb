#ifndef __PBB_MESSAGE_H__
#define __PBB_MESSAGE_H__
#include <pbb/pbb.h>
#include <pbb/Atomic.h>
#include <assert.h>

namespace pbb {
    class Message;

    class PBB_API IMessagePool
    {
    public:
        virtual Message* GetMessage(uint32_t protocol, uint32_t code)=0;
        virtual void ReleaseMessage(Message* msg)=0;
    };

    class PBB_API Message
    {
    public:
        Message();
        virtual ~Message();
        virtual uint32_t GetProtcolCRC() = 0;
        virtual uint32_t GetCode() = 0;

        //! Copy other to this
        virtual void Copy(Message* other) = 0;

        void AddToPool(IMessagePool& pool)
        {
            mRefCount = 0;
            mPool = &pool;
        }
        /**
        Add a reference to the object
        */
        void AddRef()
        {
            if (mPool)
            {
                ++mRefCount;
                assert(mRefCount > 0);
            }
        }

         /**
         Release a reference.  When count goes to 0
         it is released back to pool
         */
        void Release()
        {
            if (mPool)
            {
                assert(mRefCount > 0);
                if ((--mRefCount) == 0)
                {
                    // Release back into pool
                    mPool->ReleaseMessage(this);
                }
            }
        }

    protected:
        Atomic<int32_t> mRefCount;
        IMessagePool*   mPool;    // Do we have a message pool?
    };

#if 0
    /**
        Allow an object to be pooled
        @param BaseT  base class
        @param PoolT  ObectPool
     */
    template<typename BaseT, typename PoolT>
    class PooledObject : public BaseT
    {
    public:
        PooledObject(PoolT* pool)
            : mPool(pool)
            , mRefCount(0) // no references, must call AddRef after creation
        {
        }

        /**
        Add a reference to the object
        */
        virtual void AddRef()
        {
            ++mRefCount;
            assert(mRefCount > 0);
        }
        /**
        Release a reference.  When count goes to 0
        it is released back to pool
        */
        virtual void Release()
        {
            assert(mRefCount > 0);
            if ((--mRefCount) == 0)
            {
                // Release back into pool
                mPool->Release(this);
            }
        }

    protected:
        PoolT* mPool;
        Atomic<int32_t> mRefCount;
    };
#endif
} /* namespace pbb */
#endif /* __PBB_MESSAGE_H__ */