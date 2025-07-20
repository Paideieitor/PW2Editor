#include <cstdarg>
#include <iostream>
#include <chrono>

#include "System.h"

#include "Utils/FileUtils.h"

const char* logLevels[] = {
	"NONE",
	"CRITICAL",
	"WARNING",
	"INFO",
	"DEBUG",
};

string logStr = string();

string GetLogName()
{
	if (logStr.empty())
	{
		time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		logStr = "Logs\\" + to_string(currentTime) + ".log";
	}
	return logStr;
}

void Log(LogLevel logLevel, const char* format, ...)
{
	if (logLevel > MAX_LOG_LEVEL)
		return;
	
	FILE* logFile = nullptr;
	string logName = GetLogName();
	freopen_s(&logFile, logName.c_str(), "a", stdout);
	if (logFile == nullptr)
	{
		Log(CRITICAL, "Failed to log!");
		return;
	}
	fseek(logFile, 0, SEEK_END);

	printf_s("%s: ", logLevels[logLevel]);

	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	printf_s("\n");

	fclose(logFile);
}