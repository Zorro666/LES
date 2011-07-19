#ifndef LES_LOG_HH
#define LES_LOG_HH

#include <stdarg.h>

#define LES_FATAL_ERROR LES_Logger::FatalError
#define LES_ERROR LES_Logger::FatalError
#define LES_WARNING LES_Logger::Warning
#define LES_LOG LES_Logger::Log

class LES_Logger
{
public:
	enum LogChannel { CHANNEL_FATAL_ERROR = 0, 
										CHANNEL_WARNING = 1, 
										CHANNEL_LOG = 2, 
										LOG_NUM_CHANNELS = 3 
									};

	enum ChannelFlags { CHANNEL_FLAGS_FATAL = (1<<0),
											CHANNEL_FLAGS_CONSOLE_OUTPUT = (1<<1)
											//CHANNEL_FILE_OUTPUT = (1<<2),
										};

public:
	static void Init(void);
	static void Shutdown(void);

	static void SetChannelFlags(const int channel, const int flags);
	static unsigned int GetChannelFlags(const int channel);
	static void SetFatal(const int channel, const bool fatal);
	static void SetConsoleOutput(const int channel, const bool consoleOutput);
	//static void SetFileOutput(const int channel, const bool fileOutput);

	static void FatalError(const char* const fmt, ...);
	static void Warning(const char* const fmt, ...);
	static void Log(const char* const fmt, ...);
private:
	static unsigned int s_channelFlags[LOG_NUM_CHANNELS];
	static void InternalOutput(const unsigned int flags, const char* const prefix, const char* const fmt, va_list* pArgPtr);
};

#endif // #ifdef LES_LOG_HH
