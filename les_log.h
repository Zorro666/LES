#ifdef LES_LOG_HH
#define LES_LOG_HH

#define LES_FATAL_ERROR LES_Log::FatalError
#define LES_WARNING LES_Log:Warning
#define LES_LOG LES_Log:Log

class LES_Log
{
public:
	static void FatalError(...);
	static void Warning(...);
	static void Log(...);
};

#endif // #ifdef LES_LOG_HH
