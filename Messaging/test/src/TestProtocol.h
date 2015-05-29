#ifndef __TEST_PROTOCOL_H__
#define __TEST_PROTOCOL_H__
#include <pbb/Message.h>

class TestMessage : public pbb::Message
{
public:
    virtual uint32_t GetProtcolCRC() { return 1; }
    virtual uint32_t GetCode() { return 2; }
    virtual void Copy(Message* other)
    {
        if (other == 0 || this == other) return;
        // not same message?
        if (other->GetProtcolCRC() != this->GetProtcolCRC() ||
            other->GetCode() != other->GetCode())
            return;

        TestMessage* otherMsg = (TestMessage*)other;
        // do nothing..
        this->data = otherMsg->data;
    }

    /// Start Data
    uint32_t data;
    /// End Data
};

class TEST_PROTOCOL
{
public:
    static const uint32_t CRC = 0x00000001;
    static TEST_PROTOCOL inst;

    pbb::PooledObject<TestMessage, TEST_PROTOCOL> mMsg;
    int releases;
    TEST_PROTOCOL()
        : mMsg(this)
    {
        releases = 0;
    }

    static pbb::Message* CreateMessage(uint32_t code)
    {
        // the only copy
        inst.releases = 0;
        // someone just asked for a copy, add a ref
        inst.mMsg.AddRef();
        return &inst.mMsg;
    }

    void Release(pbb::Message* msg)
    {
        // refcount SHOULD be 0
        releases++;

    }
};

#endif