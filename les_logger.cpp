#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "les_logger.h"

unsigned int LES_Logger::s_channelFlags[LOG_NUM_CHANNELS];
bool LES_Logger::s_errorFlag = false;
FILE* LES_Logger::s_filePtr = LES_NULL;

#define LOG_OUTPUT_FILE "log.txt"


bool LES_Logger::GetErrorStatus(void)
{
	return s_errorFlag;
}

void LES_Logger::ClearErrorStatus(void)
{
	s_errorFlag = false;
}

void LES_Logger::SetErrorStatus(void)
{
	s_errorFlag = true;
}

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

void LES_Logger::SetFileOutput(const int channel, const bool fileOutput)
{
	const unsigned int currentFlags = GetChannelFlags(channel);
	const unsigned int newFlags = (currentFlags & ~CHANNEL_FLAGS_FILE_OUTPUT) | (fileOutput ? CHANNEL_FLAGS_FILE_OUTPUT : 0);
	SetChannelFlags(channel, newFlags);
}

void LES_Logger::Init(void)
{
	if (s_filePtr == LES_NULL)
	{
		s_filePtr = fopen(LOG_OUTPUT_FILE, "w");
	}
	for (int i = 0; i < LOG_NUM_CHANNELS; i++)
	{
		s_channelFlags[i] = 0;
	}
	SetChannelFlags(CHANNEL_FATAL_ERROR, CHANNEL_FLAGS_FATAL | CHANNEL_FLAGS_CONSOLE_OUTPUT | CHANNEL_FLAGS_FILE_OUTPUT);
	SetChannelFlags(CHANNEL_ERROR, CHANNEL_FLAGS_CONSOLE_OUTPUT | CHANNEL_FLAGS_FILE_OUTPUT);
	SetChannelFlags(CHANNEL_WARNING, CHANNEL_FLAGS_CONSOLE_OUTPUT | CHANNEL_FLAGS_FILE_OUTPUT);
	SetChannelFlags(CHANNEL_LOG, CHANNEL_FLAGS_CONSOLE_OUTPUT | CHANNEL_FLAGS_FILE_OUTPUT);
}

void LES_Logger::Shutdown(void)
{
	if (s_filePtr)
	{
		fclose(s_filePtr);
	}
}

void LES_Logger::FatalError(const char* const fmt, ...)
{
	va_list argPtr;
	va_start(argPtr, fmt);

	const unsigned int flags = GetChannelFlags(CHANNEL_FATAL_ERROR);
	SetErrorStatus();
	InternalOutput(flags, "FATAL_ERROR: ", fmt, &argPtr);
}

void LES_Logger::Error(const char* const fmt, ...)
{
	va_list argPtr;
	va_start(argPtr, fmt);

	const unsigned int flags = GetChannelFlags(CHANNEL_ERROR);
	SetErrorStatus();
	InternalOutput(flags, "ERROR: ", fmt, &argPtr);
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
	if (flags & CHANNEL_FLAGS_FILE_OUTPUT)
	{
		if (s_filePtr)
		{
			fprintf(s_filePtr, "%s%s", prefix, outputBuffer);
			fflush(s_filePtr);
		}
	}
	if (flags & CHANNEL_FLAGS_FATAL)
	{
		SetErrorStatus();
		exit(-1);
	}
}

