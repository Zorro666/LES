#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "les_logger.h"

#define LES_LOG_MAX_NUM_CHANNELS 8

#define DEFAULT_LOG_OUTPUT_FILE "log.txt"
LES_LoggerChannel* LES_Logger::s_channels = LES_NULL;
bool LES_Logger::s_errorFlag = false;

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Private internal data and functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

struct LES_LoggerChannel
{
	const char* m_nickName;
	const char* m_prefixStr;
	const char* m_outputFileName;
	unsigned int m_flags;
	FILE* m_filePtr;
	bool Create( const char* const nickName, const char* const prefix, const char* const outputFileName, const unsigned int flags);
};

bool LES_LoggerChannel::Create( const char* const nickName, const char* const prefix, const char* const outputFileName, 
																const unsigned int flags)
{
	m_nickName = nickName;
	m_prefixStr = prefix;
	m_flags = flags;
	m_outputFileName = outputFileName;
	m_filePtr = fopen(m_outputFileName, "w");
	if (m_filePtr == LES_NULL)
	{
		return false;
	}
	fclose(m_filePtr);

	return true;
}

void LES_Logger::InternalOutput(const LES_LoggerChannel* const channelPtr, const char* const fmt, va_list* pArgPtr)
{
	char outputBuffer[1024];
	va_list argPtr = *pArgPtr;

	vsnprintf(outputBuffer, sizeof(outputBuffer), fmt, argPtr);
	va_end(argPtr);

	const unsigned int flags = channelPtr->m_flags;
	const char* const prefix = channelPtr->m_prefixStr;
	if (flags & CHANNEL_FLAGS_CONSOLE_OUTPUT)
	{
		fprintf(stdout, "%s%s", prefix, outputBuffer);
	}
	if (flags & CHANNEL_FLAGS_FILE_OUTPUT)
	{
		const char* const fileName = channelPtr->m_outputFileName;
		FILE* const filePtr = fopen(fileName, "a");
		if (filePtr)
		{
			fprintf(filePtr, "%s%s", prefix, outputBuffer);
			fflush(filePtr);
			fclose(filePtr);
		}
	}
	if (flags & CHANNEL_FLAGS_FATAL)
	{
		SetErrorStatus();
		exit(-1);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Public data and functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

void LES_Logger::Init(void)
{
	s_channels = new LES_LoggerChannel[LES_LOG_MAX_NUM_CHANNELS];
	if (s_channels == LES_NULL)
	{
		return;
	}

	//Create default channels
	unsigned int defaultFlags;
	defaultFlags = CHANNEL_FLAGS_CONSOLE_OUTPUT | CHANNEL_FLAGS_FILE_OUTPUT;
	s_channels[CHANNEL_FATAL_ERROR].Create("FATAL_ERROR", "FATAL_ERROR: ", DEFAULT_LOG_OUTPUT_FILE, defaultFlags | CHANNEL_FLAGS_FATAL);
	s_channels[CHANNEL_ERROR].Create("ERROR", "ERROR: ", DEFAULT_LOG_OUTPUT_FILE, defaultFlags);
	s_channels[CHANNEL_WARNING].Create("WARNING", "WARNING: ", DEFAULT_LOG_OUTPUT_FILE, defaultFlags);
	s_channels[CHANNEL_LOG].Create("LOG", "", DEFAULT_LOG_OUTPUT_FILE, defaultFlags);
}

void LES_Logger::Shutdown(void)
{
	//Loop over channels closing them down
}

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
	return s_channels[channel].m_flags;
}

void LES_Logger::SetChannelFlags(const int channel, const int flags)
{
	if ((channel < 0) || (channel >= LOG_NUM_CHANNELS))
	{
		return;
	}
	s_channels[channel].m_flags = flags;
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

void LES_Logger::FatalError(const char* const fmt, ...)
{
	va_list argPtr;
	va_start(argPtr, fmt);

	SetErrorStatus();
	InternalOutput(&s_channels[CHANNEL_FATAL_ERROR], fmt, &argPtr);
}

void LES_Logger::Error(const char* const fmt, ...)
{
	va_list argPtr;
	va_start(argPtr, fmt);

	SetErrorStatus();
	InternalOutput(&s_channels[CHANNEL_ERROR], fmt, &argPtr);
}

void LES_Logger::Warning(const char* const fmt, ...)
{
	va_list argPtr;
	va_start(argPtr, fmt);

	InternalOutput(&s_channels[CHANNEL_WARNING], fmt, &argPtr);
}

void LES_Logger::Log(const char* const fmt, ...)
{
	va_list argPtr;
	va_start(argPtr, fmt);

	InternalOutput(&s_channels[CHANNEL_LOG], fmt, &argPtr);
}

