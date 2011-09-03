#include <string.h>

#include "les_logger.h"

#define LES_LOG_MAX_NUM_CHANNELS 128

#define DEFAULT_LOG_OUTPUT_FILE "log.txt"
LES_LoggerChannel** LES_Logger::s_channelPtrs = LES_NULL;
bool LES_Logger::s_errorFlag = false;

LES_LoggerChannel* LES_Logger::s_defaultChannels[LOGGER_NUM_DEFAULT_CHANNELS];

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Public data and functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

void LES_Logger::Init(void)
{
	s_channelPtrs = new LES_LoggerChannel*[LES_LOG_MAX_NUM_CHANNELS];
	if (s_channelPtrs == LES_NULL)
	{
		return;
	}
	memset(s_channelPtrs, LES_NULL, sizeof(LES_LoggerChannel*)*LES_LOG_MAX_NUM_CHANNELS);
	memset(s_defaultChannels, LES_NULL, sizeof(LES_LoggerChannel*)*LOGGER_NUM_DEFAULT_CHANNELS);

	//Create default channels
	unsigned int defaultFlags;
	defaultFlags = LES_LOGGERCHANNEL_FLAGS_DEFAULT;

	s_defaultChannels[CHANNEL_FATAL_ERROR] = CreateChannel("FATAL_ERROR", "FATAL_ERROR: ", DEFAULT_LOG_OUTPUT_FILE, 
										 																		 defaultFlags | LES_LOGGERCHANNEL_FLAGS_FATAL);

	s_defaultChannels[CHANNEL_ERROR] = CreateChannel("ERROR", "ERROR: ", DEFAULT_LOG_OUTPUT_FILE, defaultFlags);
	s_defaultChannels[CHANNEL_WARNING] = CreateChannel("WARNING", "WARNING: ", DEFAULT_LOG_OUTPUT_FILE, defaultFlags);
	s_defaultChannels[CHANNEL_LOG] = CreateChannel("LOG", "", DEFAULT_LOG_OUTPUT_FILE, defaultFlags);
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

LES_LoggerChannel* LES_Logger::GetDefaultChannel(const int channel)
{
	if ((channel < 0) || (channel >= LOGGER_NUM_DEFAULT_CHANNELS))
	{
		return LES_NULL;
	}
	return s_defaultChannels[channel];
}

unsigned int LES_Logger::GetChannelFlags(const int channel)
{
	LES_LoggerChannel* const channelPtr = GetDefaultChannel(channel);
	if (channelPtr)
	{
		return channelPtr->GetFlags();
	}
	return 0;
}

void LES_Logger::SetChannelFlags(const int channel, const int flags)
{
	LES_LoggerChannel* const channelPtr = GetDefaultChannel(channel);
	if (channelPtr)
	{
		channelPtr->SetFlags(flags);
	}
}

void LES_Logger::SetFatal(const int channel, const bool fatal)
{
	LES_LoggerChannel* const channelPtr = GetDefaultChannel(channel);
	if (channelPtr)
	{
		channelPtr->ChangeFlags(LES_LOGGERCHANNEL_FLAGS_FATAL, fatal);
	}
}

void LES_Logger::SetConsoleOutput(const int channel, const bool consoleOutput)
{
	LES_LoggerChannel* const channelPtr = GetDefaultChannel(channel);
	if (channelPtr)
	{
		channelPtr->ChangeFlags(LES_LOGGERCHANNEL_FLAGS_CONSOLE_OUTPUT, consoleOutput);
	}
}

void LES_Logger::SetFileOutput(const int channel, const bool fileOutput)
{
	LES_LoggerChannel* const channelPtr = GetDefaultChannel(channel);
	if (channelPtr)
	{
		channelPtr->ChangeFlags(LES_LOGGERCHANNEL_FLAGS_FILE_OUTPUT, fileOutput);
	}
}

void LES_Logger::SetChannelOutputFileName(const int channel, const char* const fname)
{
	LES_LoggerChannel* const channelPtr = GetDefaultChannel(channel);
	if (channelPtr)
	{
		channelPtr->SetOutputFileName(fname);
	}
}

void LES_Logger::FatalError(const char* const fmt, ...)
{
	va_list argPtr;
	va_start(argPtr, fmt);

	SetErrorStatus();
	LES_LoggerChannel* const channelPtr = GetDefaultChannel(CHANNEL_FATAL_ERROR);
	if (channelPtr)
	{
		channelPtr->InternalOutput(fmt, &argPtr);
	}
}

void LES_Logger::Error(const char* const fmt, ...)
{
	va_list argPtr;
	va_start(argPtr, fmt);

	SetErrorStatus();
	LES_LoggerChannel* const channelPtr = GetDefaultChannel(CHANNEL_ERROR);
	if (channelPtr)
	{
		channelPtr->InternalOutput(fmt, &argPtr);
	}
}

void LES_Logger::Warning(const char* const fmt, ...)
{
	va_list argPtr;
	va_start(argPtr, fmt);

	LES_LoggerChannel* const channelPtr = GetDefaultChannel(CHANNEL_WARNING);
	if (channelPtr)
	{
		channelPtr->InternalOutput(fmt, &argPtr);
	}
}

void LES_Logger::Log(const char* const fmt, ...)
{
	va_list argPtr;
	va_start(argPtr, fmt);

	LES_LoggerChannel* const channelPtr = GetDefaultChannel(CHANNEL_LOG);
	if (channelPtr)
	{
		channelPtr->InternalOutput(fmt, &argPtr);
	}
}

LES_LoggerChannel* LES_Logger::CreateChannel(const char* const nickName, const char* const prefix, 
																						 const char* const outputFileName, const unsigned int flags)
{
	for (int channel = 0; channel < LES_LOG_MAX_NUM_CHANNELS; channel++)
	{
		LES_LoggerChannel* const channelPtr = s_channelPtrs[channel];
		if (channelPtr == LES_NULL)
		{
			LES_LoggerChannel* const newChannel = new LES_LoggerChannel(nickName, prefix, outputFileName, flags);
			s_channelPtrs[channel] = newChannel;
			return newChannel;
		}
	}
	return LES_NULL;
}


