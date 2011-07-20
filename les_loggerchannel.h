#ifndef LES_LOGGERCHANNEL_HH
#define LES_LOGGERCHANNEL_HH

#include "les_base.h"

class LES_LoggerChannel
{
public:
	LES_LoggerChannel();
	~LES_LoggerChannel();

	void Error(const char* const fmt, ...);
	void Print(const char* const fmt, ...);

	void SetFlags(void);
	unsigned int GetFlags(void);

private:
	LES_LoggerChannel(const LES_LoggerChannel& other);
	LES_LoggerChannel& operator=(const LES_LoggerChannel& rhs);

	const char* m_nickName;
	const char* m_prefixStr;
	const char* m_outputFileName;
	unsigned int m_flags;
	FILE* m_filePtr;

	friend class LES_Logger;
	bool Create( const char* const nickName, const char* const prefix, const char* const outputFileName, const unsigned int flags);
	void SetOutputFileName(const char* const fname);
};

#endif //#ifndef LES_LOGGERCHANNEL_HH
