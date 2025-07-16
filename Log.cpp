#include <cstdarg>
#include <iostream>

#include "Log.h"

const char* logLevels[] = {
	"NONE",
	"CRITICAL",
	"WARNING",
	"INFO",
	"DEBUG",
};

void Log(LogLevel logLevel, const char* format, ...)
{
	if (logLevel > MAX_LOG_LEVEL)
		return;

	printf_s("%s: ", logLevels[logLevel]);

	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	printf_s("\n");
}