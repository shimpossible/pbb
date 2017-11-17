#ifndef __PBB_MESSAGE_H__
#define __PBB_MESSAGE_H__
#include <pbb/pbb.h>
#include <pbb/Atomic.h>
#include <assert.h>

#include <pbb/DataChain.h>

namespace pbb {
namespace msg {

/* Message IDs are 2 bytes */
typedef uint16_t MessageID;

class Message;

class PBB_API IMessagePool
{
public:
    virtual Message* CreateMessage(uint32_t protocol, uint32_t code)=0;
    virtual void ReleaseMessage(Message* msg)=0;
};

class PBB_API Message
{
public:
    Message();
    virtual ~Message();
    virtual uint32_t GetProtcolCRC() = 0;
    virtual MessageID GetCode() = 0;

	virtual bool Get(DataChain& data, uint32_t protocolType) = 0;
	virtual bool Put(DataChain& data, uint32_t protocolType) = 0;

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
    uint32_t AddRef()
    {
        if (mPool)
        {
            ++mRefCount;
            assert(mRefCount > 0);
        }
        return mRefCount;
    }

        /**
        Release a reference.  When count goes to 0
        it is released back to pool
        */
    uint32_t Release()
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
        return mRefCount;
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

} /* namespace msg */
} /* namespace pbb */
#endif /* __PBB_MESSAGE_H__ */