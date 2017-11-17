#include "pbb/Logging.h"
#include "pbb/arch.h"
#include <iostream>
#include <algorithm>
#include <map>

#define DELIM '.'

namespace pbb
{
namespace log {

/**
  Compare string and not pointer value
 */
struct cmp_str
{
    bool operator()(char const *a, char const *b) const
    {
        return std::strcmp(a, b) < 0;
    }
};

/**
  Global list of loggers
 */
static std::map<const char*, Logger*, cmp_str> mLoggers;
typedef std::map<const char*, Logger*, cmp_str>::iterator LoggerIterator;
static LEVEL log_level;

Logger Logger::mRoot("", nullptr);

Logger& Logger::root()
{
    return mRoot;
}
Logger& Logger::get(const char* name)
{
    // TODO: take lock

    Logger* result;
    // find logger by name, or create a new one
    LoggerIterator it = mLoggers.find(name);
    if (it != mLoggers.end())
    {
        result = it->second;
    }
    else
    {
        Logger* p = findParent(name);
        result = new Logger(name, p->getSink());
    }

    return *result;
}
/**
  Find parent base on name
 */
Logger* Logger::findParent(const char* name)
{
    char* parent_name = _strdup(name);
    size_t n = strlen(parent_name);
    Logger* result = &mRoot;
    for (size_t i = 0; i<n;i++)
    {
        if (parent_name[n-i] == DELIM)
        {
            parent_name[n-i] = 0;
            LoggerIterator it = mLoggers.find(parent_name);
            if (it != mLoggers.end())
            {
                result = it->second;
                break;
            }
        }
    }
    free(parent_name);
    return result;
}

Logger::Logger(const char* name, LoggerSink* sink)
    : mName(_strdup(name))
    , mSink(sink)
{
    // Add to master list..
    mLoggers[name] = this;
    mBuffer = new char[2048];
}

Logger::~Logger()
{
    if (mName) delete(mName);
    if (mBuffer) delete(mBuffer);

    mName = 0;
    mBuffer = 0;
}

void Logger::log(LEVEL level, const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    log(level, msg, args);
    va_end(args);
}
void Logger::log(LEVEL level, const char* msg, va_list args)
{
    vsprintf_s(mBuffer, 2048, msg, args);
    LoggerSink* s = mSink;
    if (s)
    {
        MessageHeader h;
        h.time = Arch::cycle_count(); 
        h.level = level;
        h.length = strlen(mBuffer);
        h.logger = this;

        s->Push(h, mBuffer);
    }
}

Message::Message(LEVEL level, const char* file, int line)
{
    out() << level << " : " << file << "@" << line << " ";
}

Message::~Message()
{
    
}

std::ostream& Message::out()
{
    return std::cout;
}

LEVEL getLevel()
{
    return (LEVEL)std::min(log_level, LEVEL_FATAL);
}

bool IsEnabled(LEVEL level)
{
    return level <= getLevel();
}

void setLevel(LEVEL level)
{
    log_level = level;
}

}
}