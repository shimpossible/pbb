#ifndef __PBB_LOGGING_H__
#define __PBB_LOGGING_H__
#include "pbb/pbb.h"

#include <ostream>
namespace pbb
{
class PBB_API Log
{
public:

	enum LEVEL
	{
		LEVEL_FATAL,
		LEVEL_ERROR,
		LEVEL_WARNING,
		LEVEL_INFO,
		LEVEL_DEBUG,
	};

	Log(LEVEL level, const char* file, int line)
	{

	}

	std::ostream& out();

	static bool IsEnabled(LEVEL level);
	static LEVEL getLevel();
};

/**
  To prevent a weird error with the IFF logic..
  https://stackoverflow.com/questions/25827924/function-cannot-be-referenced-as-it-is-a-deleted-function
 */
class RefStream {
public:
	void operator&(std::ostream&) {}
};

/**
  Returns true if log level is enabled
 */
#define PBB_LOG_ENABLED(level) (1)

/**
  Returns a stream for the given log level
 */
#define PBB_LOG_STREAM(level) ::pbb::Log(::pbb::Log::LEVEL_##level, __FILE__, __LINE__).out()

#define PBB_LOG(level) (!PBB_LOG_ENABLED(level))? ::pbb::Log::IsEnabled(::pbb::Log::LEVEL_##level) : ::pbb::RefStream() & PBB_LOG_STREAM(level)
}
#endif /* __PBB_LOGGEING_H__*/