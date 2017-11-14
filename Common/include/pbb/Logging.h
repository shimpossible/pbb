#ifndef __PBB_LOGGING_H__
#define __PBB_LOGGING_H__
#include "pbb/pbb.h"
#include <stdarg.h>

#include <ostream>
#include <algorithm>

namespace pbb {
namespace log {

typedef uint16_t LEVEL;

const static LEVEL LEVEL_FATAL = 1000;
const static LEVEL LEVEL_ERROR = 2000;
const static LEVEL LEVEL_WARN  = 3000;
const static LEVEL LEVEL_INFO  = 4000;
const static LEVEL LEVEL_DEBUG = 5000;


bool IsEnabled(LEVEL level);
LEVEL getLevel();
void  setLevel(LEVEL level);

class Logger;
class PBB_API Message
{
public:

    Message(LEVEL level, const char* file, int line);
    ~Message();

    std::ostream& out();
};


struct MessageHeader
{
    /**
    When it was written
    */
    uint64_t time;
    /**
    Who wrote it
    */
    Logger* logger;
    /**
    Level it was written at
    */
    LEVEL level;
    /**
    Number of bytes in message
    */
    size_t length;
};

/**
  A circular buffer to hold log messages
 */
class PBB_API LoggerSink
{
    friend class LogReader;
public:
    LoggerSink(char* buffer, size_t len)
    {
        mBuffer = buffer;
        mCapacity = len;
        mIndex = 0;
    }

    int Push(MessageHeader& h, const char* msg)
    {
        // lock
        WriteLock();
        this->copy(&h, sizeof(h));
        this->copy(msg, h.length);
        //unlock
        WriteUnlock();

        return 0;
    }

    void ReadLock() {}
    void ReadUnlock() {}
protected:

    void WriteLock() {}
    void WriteUnlock() {}

    void copy(const void* src, size_t len)
    {
        uint32_t n = std::min(len, mCapacity - mIndex);
        memcpy(&mBuffer[mIndex], src, n);

        // copy remaining
        if (n < len)
        {
            memcpy(&mBuffer[0], (char*)src + n, len - n);
            mIndex = len - n;
        }
        else
        {
            mIndex += n;
        }
    }

    /**
    Where to store log messages
    */
    char* mBuffer;

    /**
    Next location to write
    */
    uint32_t mIndex;
    /**
    Size of mBuffer
    */
    uint32_t mCapacity;

    void* mReadLock;
    void* mWriteLock;
private:
};


class PBB_API LogReader
{
public:
    LogReader(LoggerSink& sink)
        : mBuffer(sink)
    {
        // TODO: make this the oldest valid index instead
        mOffset = sink.mIndex;
    }
    int pop(MessageHeader& h, char* dst)
    {
        mBuffer.ReadLock();
        // nothing to see
        if (mBuffer.mIndex == mOffset)
        {
            mBuffer.ReadUnlock();
            return 0;
        }

        // copy header
        copy(&h, sizeof(h));
        // copy text
        copy(dst, h.length);

        mBuffer.ReadUnlock();
        return 1;
    }
protected:

    void copy(void* dst, size_t len)
    {
        size_t n = std::min(len, mBuffer.mCapacity - mOffset);
        memcpy(dst, &mBuffer.mBuffer[mOffset], n);
        if (n < len) // remaining
        {
            mOffset = len - n;
            memcpy((char*)dst + n, &mBuffer.mBuffer[0], mOffset);
        }
        else
        {
            mOffset += len;
        }
    }
    LoggerSink& mBuffer;
    // Where to read from next
    uint32_t mOffset;
};


class PBB_API Logger
{
public:
    static Logger& root();
    static Logger& get(const char* name);


    void fatal(const char* msg, ...)
    {
        va_list args;
        va_start(args, msg);
        log(LEVEL_FATAL, msg, args);
        va_end(args);
    }
    void error(const char* msg, ...)
    {
        va_list args;
        va_start(args, msg);
        log(LEVEL_ERROR, msg, args);
        va_end(args);
    }
    void info(const char* msg, ...)
    {
        va_list args;
        va_start(args, msg);
        log(LEVEL_INFO, msg, args);
        va_end(args);
    }
    void warn(const char* msg, ...)
    {
        va_list args;
        va_start(args, msg);
        log(LEVEL_WARN, msg, args);
        va_end(args);
    }
    void debug(const char* msg, ...)
    {
        va_list args;
        va_start(args, msg);
        log(LEVEL_DEBUG, msg, args);
        va_end(args);
    }

    void log(LEVEL level, const char* msg, ...);
    void log(LEVEL level, const char* msg, va_list args);

    LoggerSink* getSink() const { return mSink;}

    void setSink(LoggerSink* s)
    {
        mSink = s;
    }
    LoggerSink* getSink()
    {
        return mSink;
    }
protected:


    static Logger* findParent(const char* name);

    Logger(const char* name, LoggerSink* sink);
    ~Logger();

    static Logger mRoot;
    const char* mName;
    LoggerSink* mSink;
    char*       mBuffer;

};
/**
    Returns true if log level is enabled
    */
#define PBB_LOG_ENABLED(level) ::pbb::log::Message::IsEnabled(::pbb::log::Message::LEVEL_##level)

/**
Returns a stream for the given log level
*/
#define PBB_LOG_STREAM(level) ::pbb::log::Message(::pbb::log::LEVEL_##level, __FILE__, __LINE__).out()

#define PBB_LOG(level) if(PBB_LOG_ENABLED(level)) PBB_LOG_STREAM(level)
}
}
#endif /* __PBB_LOGGEING_H__*/