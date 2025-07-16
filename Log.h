#ifndef _LOG_H
#define _LOG_H

#include "Globals.h"

enum LogLevel : u8
{
	NONE = 0,
	CRITICAL = 1,
	WARNING = 2,
	INFO = 3,
	DEBUG = 4,
};
#define MAX_LOG_LEVEL DEBUG

void Log(LogLevel logLevel, const char* format, ...);

#endif // _LOG_H
