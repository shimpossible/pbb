#include "pbb/Logging.h"
#include <iostream>
#include <algorithm>

namespace pbb
{

static Log::LEVEL log_level = Log::LEVEL_INFO;

Log::Log(Log::LEVEL level, const char* file, int line)
{

}
std::ostream& Log::out()
{
	return std::cout;
}

Log::LEVEL Log::getLevel()
{
	return (Log::LEVEL)std::min(log_level, Log::LEVEL_FATAL);
}

bool Log::IsEnabled(Log::LEVEL level)
{
	return level <= Log::getLevel();
}

void Log::setLevel(LEVEL level)
{
    log_level = level;
}

}
