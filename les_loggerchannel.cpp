#include <stdio.h>
#include <string.h>

#include "les_loggerchannel.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Private functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

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

void LES_LoggerChannel::SetOutputFileName(const char* const fname)
{
	if (strcmp(m_outputFileName, fname) != 0)
	{
		m_outputFileName = fname;
		m_filePtr = fopen(m_outputFileName, "w");
		if (m_filePtr == LES_NULL)
		{
			return;
		}
		fclose(m_filePtr);
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
	m_filePtr = LES_NULL;
}

LES_LoggerChannel::~LES_LoggerChannel()
{
	m_nickName = LES_NULL;
	m_prefixStr = LES_NULL;
	m_outputFileName = LES_NULL;
	m_flags = 0;
	if (m_filePtr)
	{
		fclose(m_filePtr);
	}
	m_filePtr = LES_NULL;
}
