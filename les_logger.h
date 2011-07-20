#ifndef LES_LOG_HH
#define LES_LOG_HH

#include <stdarg.h>
#include <stdio.h>

#include "les_base.h"
#include "les_loggerchannel.h"

#define LES_FATAL_ERROR LES_Logger::FatalError
#define LES_ERROR LES_Logger::Error
#define LES_WARNING LES_Logger::Warning
#define LES_LOG LES_Logger::Log

class LES_Logger
{
public:
	enum LogChannel { CHANNEL_FATAL_ERROR = 0,
										CHANNEL_ERROR = 1,
										CHANNEL_WARNING = 2,
										CHANNEL_LOG = 3,
										LOGGER_NUM_DEFAULT_CHANNELS = 4,
									};

public:
	static void Init(void);
	static void Shutdown(void);

	static bool GetErrorStatus(void);
	static void ClearErrorStatus(void);

	static LES_LoggerChannel* CreateChannel(const char* const nickName, const char* const prefix, 
																					const char* const outputFileName, const unsigned int flags);

	static LES_LoggerChannel* GetDefaultChannel(const enum LogChannel channel);

	static void SetChannelOutputFileName(const int channel, const char* const fname);
	static void SetChannelFlags(const int channel, const int flags);
	static unsigned int GetChannelFlags(const int channel);

	static void SetFatal(const int channel, const bool fatal);
	static void SetConsoleOutput(const int channel, const bool consoleOutput);
	static void SetFileOutput(const int channel, const bool fileOutput);

	static void FatalError(const char* const fmt, ...);
	static void Error(const char* const fmt, ...);
	static void Warning(const char* const fmt, ...);
	static void Log(const char* const fmt, ...);
private:
	static void InternalOutput(const LES_LoggerChannel* const channelPtr, const char* const fmt, va_list* pArgPtr);
	static void SetErrorStatus(void);

	static bool s_errorFlag;
	static LES_LoggerChannel** s_channelPtrs;

	friend class LES_LoggerChannel;

	static LES_LoggerChannel* s_defaultChannels[LOGGER_NUM_DEFAULT_CHANNELS];
};

#endif // #ifdef LES_LOG_HH
