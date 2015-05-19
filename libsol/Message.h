#ifndef __MESSAGE_H__
#define __MESSAGE_H__
#include "oe_types.h"
#include "Atomic.h"
#include <assert.h>

class Message 
{
public:
    Message();
    virtual ~Message();
    virtual uint32_t GetProtcolCRC() = 0;
    virtual uint32_t GetCode() = 0;
    virtual void Copy(Message* other) = 0;


    /**
      Implement for pooled messages
     */
    virtual void AddRef() {}
    /**
     Implement for pooled messages
     */
    virtual void Release() {}
protected:
};

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
        assert(mRefCount>0)
        if((--mRefCount)==0)
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