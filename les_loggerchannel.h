#ifndef LES_LOGGERCHANNEL_HH
#define LES_LOGGERCHANNEL_HH

#include <stdarg.h>

#include "les_base.h"

enum LES_LoggerChannelFlags { LES_LOGGERCHANNEL_FLAGS_FATAL = (1<<0),
															LES_LOGGERCHANNEL_FLAGS_CONSOLE_OUTPUT = (1<<1),
															LES_LOGGERCHANNEL_FLAGS_FILE_OUTPUT = (1<<2),
															LES_LOGGERCHANNEL_FLAGS_DEFAULT = LES_LOGGERCHANNEL_FLAGS_CONSOLE_OUTPUT | 
																																LES_LOGGERCHANNEL_FLAGS_FILE_OUTPUT,
														};

class LES_LoggerChannel
{
public:
	LES_LoggerChannel();
	LES_LoggerChannel(const char* const nickName, const char* const prefix, const char* const outputFileName, const unsigned int flags);
	~LES_LoggerChannel();

	void Error(const char* const fmt, ...);
	void Print(const char* const fmt, ...);

	void SetFlags(unsigned int flags);
	void ChangeFlags(const int flags, const bool enable);
	unsigned int GetFlags(void);

private:
	LES_LoggerChannel(const LES_LoggerChannel& other);
	LES_LoggerChannel& operator=(const LES_LoggerChannel& rhs);

	const char* m_nickName;
	const char* m_prefixStr;
	const char* m_outputFileName;
	unsigned int m_flags;

	friend class LES_Logger;
	void SetOutputFileName(const char* const fname);
	void InternalOutput(const char* const fmt, va_list* pArgPtr);
};

#endif //#ifndef LES_LOGGERCHANNEL_HH
