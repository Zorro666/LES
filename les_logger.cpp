#include "les_logger.h"

#define LES_LOG_MAX_NUM_CHANNELS 8

#define DEFAULT_LOG_OUTPUT_FILE "log.txt"
LES_LoggerChannel* LES_Logger::s_channels = LES_NULL;
bool LES_Logger::s_errorFlag = false;

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
	defaultFlags = LES_LOGGERCHANNEL_FLAGS_CONSOLE_OUTPUT | LES_LOGGERCHANNEL_FLAGS_FILE_OUTPUT;
	s_channels[CHANNEL_FATAL_ERROR].Create("FATAL_ERROR", "FATAL_ERROR: ", 
																					DEFAULT_LOG_OUTPUT_FILE, defaultFlags | LES_LOGGERCHANNEL_FLAGS_FATAL);
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

LES_LoggerChannel* LES_Logger::GetChannel(const int channel)
{
	if ((channel < 0) || (channel >= LOG_NUM_CHANNELS))
	{
		return LES_NULL;
	}
	return &s_channels[channel];
}

unsigned int LES_Logger::GetChannelFlags(const int channel)
{
	LES_LoggerChannel* const channelPtr = GetChannel(channel);
	if (channelPtr)
	{
		return channelPtr->GetFlags();
	}
	return 0;
}

void LES_Logger::SetChannelFlags(const int channel, const int flags)
{
	LES_LoggerChannel* const channelPtr = GetChannel(channel);
	if (channelPtr)
	{
		channelPtr->SetFlags(flags);
	}
}

void LES_Logger::SetFatal(const int channel, const bool fatal)
{
	LES_LoggerChannel* const channelPtr = GetChannel(channel);
	if (channelPtr)
	{
		channelPtr->ChangeFlags(LES_LOGGERCHANNEL_FLAGS_FATAL, fatal);
	}
}

void LES_Logger::SetConsoleOutput(const int channel, const bool consoleOutput)
{
	LES_LoggerChannel* const channelPtr = GetChannel(channel);
	if (channelPtr)
	{
		channelPtr->ChangeFlags(LES_LOGGERCHANNEL_FLAGS_CONSOLE_OUTPUT, consoleOutput);
	}
}

void LES_Logger::SetFileOutput(const int channel, const bool fileOutput)
{
	LES_LoggerChannel* const channelPtr = GetChannel(channel);
	if (channelPtr)
	{
		channelPtr->ChangeFlags(LES_LOGGERCHANNEL_FLAGS_FILE_OUTPUT, fileOutput);
	}
}

void LES_Logger::SetChannelOutputFileName(const int channel, const char* const fname)
{
	LES_LoggerChannel* const channelPtr = GetChannel(channel);
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
	LES_LoggerChannel* const channelPtr = GetChannel(CHANNEL_FATAL_ERROR);
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
	LES_LoggerChannel* const channelPtr = GetChannel(CHANNEL_ERROR);
	if (channelPtr)
	{
		channelPtr->InternalOutput(fmt, &argPtr);
	}
}

void LES_Logger::Warning(const char* const fmt, ...)
{
	va_list argPtr;
	va_start(argPtr, fmt);

	LES_LoggerChannel* const channelPtr = GetChannel(CHANNEL_WARNING);
	if (channelPtr)
	{
		channelPtr->InternalOutput(fmt, &argPtr);
	}
}

void LES_Logger::Log(const char* const fmt, ...)
{
	va_list argPtr;
	va_start(argPtr, fmt);

	LES_LoggerChannel* const channelPtr = GetChannel(CHANNEL_LOG);
	if (channelPtr)
	{
		channelPtr->InternalOutput(fmt, &argPtr);
	}
}

