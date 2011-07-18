#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "les_log.h"

void LES_Log::FatalError(const char* const fmt, ...)
{
	char outputBuffer[1024];
	va_list argPtr;
	va_start(argPtr, fmt);
	vsnprintf(outputBuffer, sizeof(outputBuffer), fmt, argPtr);
	va_end(argPtr);
	fprintf(stderr, "FATAL_ERROR: %s\n", outputBuffer);
	exit(-1);
}

void LES_Log::Warning(const char* const fmt, ...)
{
	char outputBuffer[1024];
	va_list argPtr;
	va_start(argPtr, fmt);
	vsnprintf(outputBuffer, sizeof(outputBuffer), fmt, argPtr);
	va_end(argPtr);
	fprintf(stderr, "WARNING: %s\n", outputBuffer);
}

void LES_Log::Log(const char* const fmt, ...)
{
	char outputBuffer[1024];
	va_list argPtr;
	va_start(argPtr, fmt);
	vsnprintf(outputBuffer, sizeof(outputBuffer), fmt, argPtr);
	va_end(argPtr);
	fprintf(stderr, "LOG: %s\n", outputBuffer);
}
