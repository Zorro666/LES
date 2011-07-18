#ifndef LES_LOG_HH
#define LES_LOG_HH

#define LES_FATAL_ERROR LES_Log::FatalError
#define LES_WARNING LES_Log::Warning
#define LES_LOG LES_Log::Log

class LES_Log
{
public:
	static void FatalError(const char* const fmt, ...);
	static void Warning(const char* const fmt, ...);
	static void Log(const char* const fmt, ...);
};

#endif // #ifdef LES_LOG_HH
