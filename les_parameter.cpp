#include <stdio.h>
#include <memory.h>

#include "les_parameter.h"
#include "les_type.h"
#include "les_stringentry.h"
#include "les_struct.h"

#define LES_PARAMETER_DEBUG 0

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
	const LES_StringEntry* const aliasedStringEntryPtr = LES_GetStringEntryForID(typeEntryPtr->m_aliasedTypeID);
	if (aliasedStringEntryPtr == NULL)
	{
		fprintf(stderr, "LES ERROR: LES_FunctionParameterData::Read type:'%s' aliasedStringEntry:%d not found\n", 
						typeStringEntry->m_str, typeEntryPtr->m_aliasedTypeID);
		return LES_ERROR;
	}
	const LES_TypeEntry* const aliasedTypeEntryPtr = LES_GetTypeEntry(aliasedStringEntryPtr);
	if (aliasedTypeEntryPtr == NULL)
	{
		fprintf(stderr, "LES ERROR: LES_FunctionParameterData::Read type:'%s' aliased type:'%s' not found\n", 
						typeStringEntry->m_str, aliasedStringEntryPtr->m_str);
		return LES_ERROR;
	}
	if (parameterDataPtr == NULL)
	{
		fprintf(stderr, "LES ERROR: LES_FunctionParameterData::Read type:'%s' parameterDataPtr is NULL\n", typeStringEntry->m_str);
		return LES_ERROR;
	}

	const void* valueAddress = parameterDataPtr;
	const unsigned int flags = typeEntryPtr->m_flags;
	unsigned int parameterDataSize = typeEntryPtr->m_dataSize;
	if (flags & LES_TYPE_POINTER)
	{
		parameterDataSize = aliasedTypeEntryPtr->m_dataSize;
	}
	if (valueAddress == NULL)
	{
		fprintf(stderr, "LES ERROR: LES_FunctionParameterData::Read type:'%s' valueAddress is NULL\n", typeStringEntry->m_str);
		return LES_ERROR;
	}

	if ((flags & LES_TYPE_POD) || (flags & LES_TYPE_STRUCT))
	{
#if LES_PARAMETER_DEBUG
		printf("Read type:'%s' size:%d %p -> %p\n", typeStringEntry->m_str, typeEntryPtr->m_dataSize, m_currentReadBufferPtr, parameterDataPtr);
#endif // #if LES_PARAMETER_DEBUG
		memcpy(parameterDataPtr, m_currentReadBufferPtr, parameterDataSize);
		m_currentReadBufferPtr += parameterDataSize;
	}

	return LES_OK;
}

int LES_FunctionParameterData::Write(const LES_StringEntry* const typeStringEntry, 
																 		 const void* const parameterDataPtr, const unsigned int paramMode)
{
	const LES_TypeEntry* const rawTypeEntryPtr = LES_GetTypeEntry(typeStringEntry);
	if (rawTypeEntryPtr == NULL)
	{
		fprintf(stderr, "LES ERROR: LES_FunctionParameterData::Write type:'%s' not found\n", typeStringEntry->m_str);
		return LES_ERROR;
	}
	const LES_StringEntry* const aliasedStringEntryPtr = LES_GetStringEntryForID(rawTypeEntryPtr->m_aliasedTypeID);
	if (aliasedStringEntryPtr == NULL)
	{
		fprintf(stderr, "LES ERROR: LES_FunctionParameterData::Write type:'%s' aliasedStringEntry:%d not found\n", 
						typeStringEntry->m_str, rawTypeEntryPtr->m_aliasedTypeID);
		return LES_ERROR;
	}
	const LES_TypeEntry* const aliasedTypeEntryPtr = LES_GetTypeEntry(aliasedStringEntryPtr);
	if (aliasedTypeEntryPtr == NULL)
	{
		fprintf(stderr, "LES ERROR: LES_FunctionParameterData::Write type:'%s' aliased type:'%s' not found\n", 
						typeStringEntry->m_str, aliasedStringEntryPtr->m_str);
		return LES_ERROR;
	}
	if (parameterDataPtr == NULL)
	{
		fprintf(stderr, "LES ERROR: LES_FunctionParameterData::Write type:'%s' parameterDataPtr is NULL\n", typeStringEntry->m_str);
		return LES_ERROR;
	}
	const unsigned int rawTypeFlags = rawTypeEntryPtr->m_flags;
	if ((rawTypeFlags & paramMode) == 0)
	{
		fprintf(stderr, "LES ERROR: LES_FunctionParameterData::Write type:'%s' flags incorrect for parameter mode typeFlags:0x%X paramMode:0x%X\n", 
						typeStringEntry->m_str, rawTypeFlags, paramMode);
		return LES_ERROR;
	}
	const LES_TypeEntry* typeEntryPtr = rawTypeEntryPtr;
	if (rawTypeFlags & LES_TYPE_ALIAS)
	{
		typeEntryPtr = aliasedTypeEntryPtr;
	}
	const void* valueAddress = parameterDataPtr;
	if (rawTypeFlags & LES_TYPE_POINTER)
	{
		const void** pointerAddress = (const void**)parameterDataPtr;
		valueAddress = *pointerAddress;
	}

	const int typeFlags = typeEntryPtr->m_flags;
	const int typeDataSize = typeEntryPtr->m_dataSize;

	unsigned int parameterDataSize = typeDataSize;
	if (valueAddress == NULL)
	{
		fprintf(stderr, "LES ERROR: LES_FunctionParameterData::Write type:'%s' valueAddress is NULL\n", typeStringEntry->m_str);
		return LES_ERROR;
	}

	if (typeFlags & LES_TYPE_STRUCT)
	{
#if LES_PARAMETER_DEBUG
		printf("Write type:'%s' size:%d STRUCT\n", typeStringEntry->m_str, typeEntryPtr->m_dataSize);
#endif // #if LES_PARAMETER_DEBUG
		int returnCode = LES_OK;
		//JAKE NEED TO USE typeEntryPtr to struct definition
		const LES_StructDefinition* const structDefinition = LES_GetStructDefinition(typeEntryPtr->m_hash);
		if (structDefinition == LES_NULL)
		{
			fprintf(stderr, "LES ERROR: Write type:'%s' is a struct but can't be found\n", typeStringEntry->m_str);
			return LES_ERROR;
		}
		const int numMembers = structDefinition->GetNumMembers();
		const char* memberDataPtr = (const char*)valueAddress;
		for (int i = 0; i < numMembers; i++)
		{
			const LES_StructMember* const structMember = structDefinition->GetMemberByIndex(i);
			const int memberTypeID = structMember->m_typeID;
			const LES_StringEntry* const memberTypeStringEntry = LES_GetStringEntryForID(memberTypeID);
			const int memberAlignmentPadding = structMember->m_alignmentPadding;
			const int memberParamMode = paramMode | (rawTypeFlags & LES_TYPE_INPUT) | (rawTypeFlags & LES_TYPE_OUTPUT);
			memberDataPtr += memberAlignmentPadding;
			returnCode = Write(memberTypeStringEntry, (const void* const)memberDataPtr, memberParamMode);
			if (returnCode == LES_ERROR)
			{
				return LES_ERROR;
			}
//			const LES_TypeEntry* const memberTypeEntryPtr = LES_GetTypeEntry(memberTypeStringEntry);
//			memberDataPtr += memberTypeEntryPtr->m_dataSize;
				memberDataPtr += structMember->m_dataSize;
		}
		return returnCode;
	}

	if (typeFlags & LES_TYPE_POD)
	{
#if LES_PARAMETER_DEBUG
		printf("Write type:'%s' size:%d %p -> %p\n", typeStringEntry->m_str, typeEntryPtr->m_dataSize, valueAddress, m_currentWriteBufferPtr);
#endif // #if LES_PARAMETER_DEBUG
		memcpy(m_currentWriteBufferPtr, valueAddress, parameterDataSize);
		m_currentWriteBufferPtr += parameterDataSize;
	}

	return LES_OK;
}

