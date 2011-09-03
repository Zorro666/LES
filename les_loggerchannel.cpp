#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "les_loggerchannel.h"
#include "les_logger.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Private functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

void LES_LoggerChannel::SetOutputFileName(const char* const fname)
{
	if (strcmp(m_outputFileName, fname) != 0)
	{
		m_outputFileName = fname;
		FILE* const filePtr = fopen(m_outputFileName, "wb");
		if (filePtr == LES_NULL)
		{
			return;
		}
		fclose(filePtr);
	}
}

void LES_LoggerChannel::InternalOutput(const char* const fmt, va_list* pArgPtr)
{
	char outputBuffer[1024];
	va_list argPtr = *pArgPtr;

	vsnprintf(outputBuffer, sizeof(outputBuffer), fmt, argPtr);
	va_end(argPtr);

	const unsigned int flags = m_flags;
	const char* const prefix = m_prefixStr;
	if (flags & LES_LOGGERCHANNEL_FLAGS_CONSOLE_OUTPUT)
	{
		fprintf(stdout, "%s%s\n", prefix, outputBuffer);
	}
	if (flags & LES_LOGGERCHANNEL_FLAGS_FILE_OUTPUT)
	{
		const char* const fileName = m_outputFileName;
		FILE* const filePtr = fopen(fileName, "ab");
		if (filePtr)
		{
			fprintf(filePtr, "%s%s\n", prefix, outputBuffer);
			fflush(filePtr);
			fclose(filePtr);
		}
	}
	if (flags & LES_LOGGERCHANNEL_FLAGS_FATAL)
	{
		LES_Logger::SetErrorStatus();
		exit(-1);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Public functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

LES_LoggerChannel::LES_LoggerChannel()
{
	m_nickName = LES_NULL;
	m_prefixStr = LES_NULL;
	m_outputFileName = LES_NULL;
	m_flags = 0;
}

LES_LoggerChannel::LES_LoggerChannel( const char* const nickName, const char* const prefix, const char* const outputFileName, 
																		  const unsigned int flags)
{
	m_nickName = nickName;
	m_prefixStr = prefix;
	m_flags = flags;
	m_outputFileName = outputFileName;
	FILE* const filePtr = fopen(m_outputFileName, "wb");
	if (filePtr)
	{
		fclose(filePtr);
	}
}

LES_LoggerChannel::~LES_LoggerChannel()
{
	m_nickName = LES_NULL;
	m_prefixStr = LES_NULL;
	m_outputFileName = LES_NULL;
	m_flags = 0;
}

void LES_LoggerChannel::SetFlags(unsigned int flags)
{
	m_flags = flags;
}

unsigned int LES_LoggerChannel::GetFlags(void)
{
	return m_flags;
}

void LES_LoggerChannel::ChangeFlags(const int flags, const bool enable)
{
	const unsigned int currentFlags = m_flags;
	const unsigned int newFlags = (currentFlags & ~flags) | (enable ? flags : 0);
	m_flags = newFlags;
}

void LES_LoggerChannel::Error(const char* const fmt, ...)
{
	va_list argPtr;
	va_start(argPtr, fmt);

	LES_Logger::SetErrorStatus();
	InternalOutput(fmt, &argPtr);
}

void LES_LoggerChannel::Print(const char* const fmt, ...)
{
	va_list argPtr;
	va_start(argPtr, fmt);

	InternalOutput(fmt, &argPtr);
}

