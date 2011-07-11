#include <stdio.h>
#include <memory.h>

#include "les_parameter.h"
#include "les_type.h"
#include "les_stringentry.h"

LES_FunctionParameterData::LES_FunctionParameterData(char* const bufferPtr) : m_bufferPtr(bufferPtr)
{
	m_currentWriteBufferPtr = m_bufferPtr;
	m_currentReadBufferPtr = m_bufferPtr;
}

LES_FunctionParameterData::~LES_FunctionParameterData()
{
	delete[] m_bufferPtr;
	m_currentWriteBufferPtr = LES_NULL;
	m_currentReadBufferPtr = LES_NULL;
}

int LES_FunctionParameterData::Read(const LES_StringEntry* const typeStringEntry, void* const parameterDataPtr ) const
{
	const LES_TypeEntry* const typeEntryPtr = LES_GetTypeEntry(typeStringEntry);
	if (typeEntryPtr == NULL)
	{
		fprintf(stderr, "LES ERROR: LES_FunctionParameterData::Read type:'%s' not found\n", typeStringEntry->m_str);
		return LES_ERROR;
	}
	if (parameterDataPtr == NULL)
	{
		fprintf(stderr, "LES ERROR: LES_FunctionParameterData::Read type:'%s' parameterDataPtr is NULL\n", typeStringEntry->m_str);
		return LES_ERROR;
	}

	const void* valueAddress = parameterDataPtr;
	const unsigned int flags = typeEntryPtr->m_flags;
	if (flags & LES_TYPE_POINTER)
	{
	}
	if (valueAddress == NULL)
	{
		fprintf(stderr, "LES ERROR: LES_FunctionParameterData::Read type:'%s' valueAddress is NULL\n", typeStringEntry->m_str);
		return LES_ERROR;
	}

	const unsigned int parameterDataSize = typeEntryPtr->m_dataSize;
	memcpy(parameterDataPtr, m_currentReadBufferPtr, parameterDataSize);
	m_currentReadBufferPtr += parameterDataSize;

	return LES_OK;
}

int LES_FunctionParameterData::Write(const LES_StringEntry* const typeStringEntry, 
																 const void* const parameterDataPtr, const unsigned int paramMode)
{
	const LES_TypeEntry* const typeEntryPtr = LES_GetTypeEntry(typeStringEntry);
	if (typeEntryPtr == NULL)
	{
		fprintf(stderr, "LES ERROR: LES_FunctionParameterData::Write type:'%s' not found\n", typeStringEntry->m_str);
		return LES_ERROR;
	}
	if (parameterDataPtr == NULL)
	{
		fprintf(stderr, "LES ERROR: LES_FunctionParameterData::Write type:'%s' parameterDataPtr is NULL\n", typeStringEntry->m_str);
		return LES_ERROR;
	}
	const unsigned int flags = typeEntryPtr->m_flags;
	if ((flags & paramMode) == 0)
	{
		fprintf(stderr, "LES ERROR: LES_FunctionParameterData::Write type:'%s' flags incorrect for parameter mode typeFlags:0x%X paramMode:0x%X\n", 
						typeStringEntry->m_str, flags, paramMode);
		return LES_ERROR;
	}

	const unsigned int parameterDataSize = typeEntryPtr->m_dataSize;
	const void* valueAddress = parameterDataPtr;
	if (flags & LES_TYPE_POINTER)
	{
		const void** pointerAddress = (const void**)parameterDataPtr;
		valueAddress = *pointerAddress;
	}
	if (valueAddress == NULL)
	{
		fprintf(stderr, "LES ERROR: LES_FunctionParameterData::Write type:'%s' valueAddress is NULL\n", typeStringEntry->m_str);
		return LES_ERROR;
	}

	memcpy(m_currentWriteBufferPtr, valueAddress, parameterDataSize);
	m_currentWriteBufferPtr += parameterDataSize;

	return LES_OK;
}

