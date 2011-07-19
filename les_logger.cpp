#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "les_logger.h"

unsigned int LES_Logger::s_channelFlags[LOG_NUM_CHANNELS] = { 0, 0, 0 };

unsigned int LES_Logger::GetChannelFlags(const int channel)
{
	if ((channel < 0) || (channel >= LOG_NUM_CHANNELS))
	{
		return 0;
	}
	return s_channelFlags[channel];
}

void LES_Logger::SetChannelFlags(const int channel, const int flags)
{
	if ((channel < 0) || (channel >= LOG_NUM_CHANNELS))
	{
		return;
	}
	s_channelFlags[channel] = flags;
}

void LES_Logger::SetFatal(const int channel, const bool fatal)
{
	const unsigned int currentFlags = GetChannelFlags(channel);
	const unsigned int newFlags = (currentFlags & ~CHANNEL_FLAGS_FATAL) | (fatal ? CHANNEL_FLAGS_FATAL : 0);
	SetChannelFlags(channel, newFlags);
}

void LES_Logger::SetConsoleOutput(const int channel, const bool consoleOutput)
{
	const unsigned int currentFlags = GetChannelFlags(channel);
	const unsigned int newFlags = (currentFlags & ~CHANNEL_FLAGS_CONSOLE_OUTPUT) | (consoleOutput ? CHANNEL_FLAGS_CONSOLE_OUTPUT : 0);
	SetChannelFlags(channel, newFlags);
}

void LES_Logger::Init(void)
{
	SetChannelFlags(CHANNEL_FATAL_ERROR, CHANNEL_FLAGS_FATAL | CHANNEL_FLAGS_CONSOLE_OUTPUT);
	SetChannelFlags(CHANNEL_WARNING, CHANNEL_FLAGS_CONSOLE_OUTPUT);
	SetChannelFlags(CHANNEL_LOG, CHANNEL_FLAGS_CONSOLE_OUTPUT);
}

void LES_Logger::Shutdown(void)
{
}

void LES_Logger::FatalError(const char* const fmt, ...)
{
	va_list argPtr;
	va_start(argPtr, fmt);

	const unsigned int flags = GetChannelFlags(CHANNEL_FATAL_ERROR);
	InternalOutput(flags, "FATAL_ERROR: ", fmt, &argPtr);
}

void LES_Logger::Warning(const char* const fmt, ...)
{
	va_list argPtr;
	va_start(argPtr, fmt);

	const unsigned int flags = GetChannelFlags(CHANNEL_WARNING);
	InternalOutput(flags, "WARNING: ", fmt, &argPtr);
}

void LES_Logger::Log(const char* const fmt, ...)
{
	va_list argPtr;
	va_start(argPtr, fmt);

	const unsigned int flags = GetChannelFlags(CHANNEL_LOG);
	InternalOutput(flags, "", fmt, &argPtr);
}

void LES_Logger::InternalOutput(const unsigned int flags, const char* const prefix, const char* const fmt, va_list* pArgPtr)
{
	char outputBuffer[1024];
	va_list argPtr = *pArgPtr;

	vsnprintf(outputBuffer, sizeof(outputBuffer), fmt, argPtr);
	va_end(argPtr);

	if (flags & CHANNEL_FLAGS_CONSOLE_OUTPUT)
	{
		fprintf(stdout, "%s%s", prefix, outputBuffer);
	}
	if (flags & CHANNEL_FLAGS_FATAL)
	{
		exit(-1);
	}
}

