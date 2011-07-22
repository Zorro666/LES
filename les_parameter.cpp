#include <memory.h>

#include "les_parameter.h"
#include "les_core.h"
#include "les_logger.h"
#include "les_type.h"
#include "les_stringentry.h"
#include "les_struct.h"

#define LES_PARAMETER_DEBUG 0

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Public External functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

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
		LES_WARNING("LES_FunctionParameterData::Read type:'%s' not found\n", typeStringEntry->m_str);
		return LES_RETURN_ERROR;
	}
	const LES_StringEntry* const aliasedStringEntryPtr = LES_GetStringEntryForID(typeEntryPtr->m_aliasedTypeID);
	if (aliasedStringEntryPtr == NULL)
	{
		LES_WARNING("LES_FunctionParameterData::Read type:'%s' aliasedStringEntry:%d not found\n", 
								typeStringEntry->m_str, typeEntryPtr->m_aliasedTypeID);
		return LES_RETURN_ERROR;
	}
	const LES_TypeEntry* const aliasedTypeEntryPtr = LES_GetTypeEntry(aliasedStringEntryPtr);
	if (aliasedTypeEntryPtr == NULL)
	{
		LES_WARNING("LES_FunctionParameterData::Read type:'%s' aliased type:'%s' not found\n", 
								typeStringEntry->m_str, aliasedStringEntryPtr->m_str);
		return LES_RETURN_ERROR;
	}
	if (parameterDataPtr == NULL)
	{
		LES_WARNING("LES_FunctionParameterData::Read type:'%s' parameterDataPtr is NULL\n", typeStringEntry->m_str);
		return LES_RETURN_ERROR;
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
		LES_WARNING("LES_FunctionParameterData::Read type:'%s' valueAddress is NULL\n", typeStringEntry->m_str);
		return LES_RETURN_ERROR;
	}

	if ((flags & LES_TYPE_POD) || (flags & LES_TYPE_STRUCT))
	{
#if LES_PARAMETER_DEBUG
		LES_LOG("Read type:'%s' size:%d %p -> %p\n", typeStringEntry->m_str, typeEntryPtr->m_dataSize, m_currentReadBufferPtr, parameterDataPtr);
#endif // #if LES_PARAMETER_DEBUG
		memcpy(parameterDataPtr, m_currentReadBufferPtr, parameterDataSize);
		m_currentReadBufferPtr += parameterDataSize;
	}

	return LES_RETURN_OK;
}

int LES_FunctionParameterData::Write(const LES_StringEntry* const typeStringEntry, 
																 		 const void* const parameterDataPtr, const unsigned int paramMode)
{
	const LES_TypeEntry* const typeEntryPtr = LES_GetTypeEntry(typeStringEntry);
	if (typeEntryPtr == NULL)
	{
		LES_WARNING("LES_FunctionParameterData::Write type:'%s' not found\n", typeStringEntry->m_str);
		return LES_RETURN_ERROR;
	}
	const unsigned int typeFlags = typeEntryPtr->m_flags;
	if ((typeFlags & paramMode) == 0)
	{
		LES_WARNING("LES_FunctionParameterData::Write type:'%s' flags incorrect for parameter mode typeFlags:0x%X paramMode:0x%X\n", 
								typeStringEntry->m_str, typeFlags, paramMode);
		return LES_RETURN_ERROR;
	}
	return WriteInternal(typeStringEntry, typeEntryPtr, parameterDataPtr);
}

int LES_FunctionParameterData::GetNumBytesWritten(void) const
{
	unsigned int start = (unsigned int)m_bufferPtr;
	unsigned int current = (unsigned int)m_currentWriteBufferPtr;
	unsigned int numBytesWritten = current - start;
	return numBytesWritten;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Private Internal functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

int LES_FunctionParameterData::WriteInternal(const LES_StringEntry* const typeStringEntry, const LES_TypeEntry* const inputTypeEntryPtr, 
																						 const void* const parameterDataPtr)
{
	if (inputTypeEntryPtr == NULL)
	{
		LES_WARNING("LES_FunctionParameterData::WriteInternal type:'%s' not found\n", typeStringEntry->m_str);
		return LES_RETURN_ERROR;
	}
	if (parameterDataPtr == NULL)
	{
		LES_WARNING("LES_FunctionParameterData::WriteInternal type:'%s' parameterDataPtr is NULL\n", typeStringEntry->m_str);
		return LES_RETURN_ERROR;
	}

	unsigned int typeFlags = inputTypeEntryPtr->m_flags;
	if ((typeFlags & LES_TYPE_ARRAY) == 0)
	{
		return WriteItem(typeStringEntry, inputTypeEntryPtr, parameterDataPtr);
	}

	// Loop over the elements of the array writing each item 1 by 1
	const int numElements = inputTypeEntryPtr->m_numElements;
	LES_WARNING("WriteInternal: '%s[%d]' ARRAY TYPES NOT SUPPORTED YET\n", typeStringEntry->m_str, numElements);

	const int aliasedTypeID = inputTypeEntryPtr->m_aliasedTypeID;
	const LES_StringEntry* const aliasedTypeStringEntry = LES_GetStringEntryForID(aliasedTypeID);
	if (aliasedTypeStringEntry == LES_NULL)
	{
		LES_WARNING("WriteInternal: Type:'%s' aliased type:%d entry can't be found\n", typeStringEntry->m_str, aliasedTypeID);
		return LES_RETURN_ERROR;
	}
#if LES_PARAMETER_DEBUG
	LES_LOG("WriteInternal: Type:'%s' aliased to '%s'\n", typeStringEntry->m_str, aliasedTypeStringEntry->m_str);
#endif // #if LES_PARAMETER_DEBUG
	const LES_TypeEntry* const aliasedTypeEntryPtr = LES_GetTypeEntry(aliasedTypeStringEntry);
	if (aliasedTypeEntryPtr == LES_NULL)
	{
		LES_WARNING("WriteInternal: Type:'%s' type can't be found\n", aliasedTypeStringEntry->m_str);
		return LES_RETURN_ERROR;
	}
	LES_FATAL_ERROR("JAKE - THE PASSED IN parameterDataPtr for an array type is of a pointer type");
	LES_FATAL_ERROR("JAKE - MOVE ALONG THE PARAMETER DATA PTR BY CORRECT NUMBER OF BYTES - WHICH IS sizeof(type) which isn't the same as ComputeStorageDataSize()");
	const LES_StringEntry* const itemTypeStringEntry = typeStringEntry;
	const LES_TypeEntry* const itemTypeEntryPtr = inputTypeEntryPtr;
	for (int element = 0; element < numElements; element++)
	{
		const void* const itemParameterDataPtr = parameterDataPtr;

		const int retError = WriteItem(itemTypeStringEntry, itemTypeEntryPtr, itemParameterDataPtr);
		if (retError == LES_RETURN_ERROR)
		{
			return LES_RETURN_ERROR;
		}
	}
	return LES_RETURN_ERROR;
}

int LES_FunctionParameterData::WriteItem(const LES_StringEntry* const typeStringEntry, const LES_TypeEntry* const inputTypeEntryPtr, 
																				 const void* const parameterDataPtr)
{
#if LES_PARAMETER_DEBUG
	LES_LOG("WriteItem type:'%s'\n", typeStringEntry->m_str);
#endif // #if LES_PARAMETER_DEBUG
	const unsigned int inputTypeFlags = inputTypeEntryPtr->m_flags;
	const LES_TypeEntry* typeEntryPtr = inputTypeEntryPtr;
	unsigned int typeFlags = typeEntryPtr->m_flags;

	const char* typeNameStr = typeStringEntry->m_str;
	while (typeFlags & LES_TYPE_ALIAS)
	{
		const int aliasedTypeID = typeEntryPtr->m_aliasedTypeID;
		const LES_StringEntry* const aliasedTypeStringEntry = LES_GetStringEntryForID(aliasedTypeID);
		if (aliasedTypeStringEntry == LES_NULL)
		{
			LES_WARNING("WriteItem type:'%s' aliased type:%d entry can't be found\n", typeNameStr, aliasedTypeID);
			return LES_RETURN_ERROR;
		}
#if LES_PARAMETER_DEBUG
		LES_LOG("WriteItem: Type:'%s' aliased to '%s'\n", typeNameStr, aliasedTypeStringEntry->m_str);
#endif // #if LES_PARAMETER_DEBUG
		typeEntryPtr = LES_GetTypeEntry(aliasedTypeStringEntry);
		if (typeEntryPtr == LES_NULL)
		{
			LES_WARNING("WriteItem type:'%s' type can't be found\n", aliasedTypeStringEntry->m_str);
			return LES_RETURN_ERROR;
		}
#if LES_PARAMETER_DEBUG
		LES_LOG("WriteItem: Type:'%s' aliased to 0x%X\n", typeNameStr, aliasedTypeStringEntry->m_hash);
#endif // #if LES_PARAMETER_DEBUG
		typeFlags = typeEntryPtr->m_flags;
		typeNameStr = (const char*)aliasedTypeStringEntry->m_str;
	}

	const void* valueAddress = parameterDataPtr;
	if (inputTypeFlags & LES_TYPE_POINTER)
	{
#if LES_PARAMETER_DEBUG
		LES_LOG("%s ptr address changing %p\n", typeStringEntry->m_str, parameterDataPtr);
#endif // #if LES_PARAMETER_DEBUG
		const void** pointerAddress = (const void**)parameterDataPtr;
		valueAddress = *pointerAddress;
	}

	const int typeDataSize = typeEntryPtr->m_dataSize;

	unsigned int parameterDataSize = typeDataSize;
	if (valueAddress == NULL)
	{
		LES_WARNING("LES_FunctionParameterData::WriteItem type:'%s' valueAddress is NULL\n", typeStringEntry->m_str);
		return LES_RETURN_ERROR;
	}

	if (typeFlags & LES_TYPE_STRUCT)
	{
#if LES_PARAMETER_DEBUG
		LES_LOG("Write type:'%s' size:%d hash:0x%X STRUCT\n", typeStringEntry->m_str, typeDataSize, typeEntryPtr->m_hash);
#endif // #if LES_PARAMETER_DEBUG
		int returnCode = LES_RETURN_OK;
		const LES_StructDefinition* const structDefinition = LES_GetStructDefinition(typeEntryPtr->m_hash);
		if (structDefinition == LES_NULL)
		{
			LES_WARNING("Write type:'%s' is a struct but can't be found\n", typeStringEntry->m_str);
			return LES_RETURN_ERROR;
		}
		const int numMembers = structDefinition->GetNumMembers();
		const char* memberDataPtr = (const char*)valueAddress;
		for (int i = 0; i < numMembers; i++)
		{
			const LES_StructMember* const structMember = structDefinition->GetMemberByIndex(i);
			const int memberTypeID = structMember->m_typeID;
			const LES_StringEntry* const memberTypeStringEntry = LES_GetStringEntryForID(memberTypeID);
			const LES_TypeEntry* const memberTypeEntryPtr = LES_GetTypeEntry(memberTypeStringEntry);
			const int memberAlignmentPadding = structMember->m_alignmentPadding;
			memberDataPtr += memberAlignmentPadding;
			//struct members which are references are really pointers to the data
			const void* paramDataPtr = (void*)memberDataPtr;
			if (memberTypeEntryPtr->m_flags & LES_TYPE_POINTER)
			{
#if LES_PARAMETER_DEBUG
				LES_LOG("Member[%d] %s %d %p\n", i, memberTypeStringEntry->m_str, structMember->m_alignmentPadding, memberDataPtr);
#endif // #if LES_PARAMETER_DEBUG
			}
			if (memberTypeEntryPtr->m_flags & LES_TYPE_REFERENCE)
			{
				const void** pointerAddress = (const void**)memberDataPtr;
				paramDataPtr = *pointerAddress;
#if LES_PARAMETER_DEBUG
				LES_LOG("Member[%d] %d %s %p ptr address -> %p\n", i, 
								structMember->m_alignmentPadding, memberTypeStringEntry->m_str, memberDataPtr, paramDataPtr);
#endif // #if LES_PARAMETER_DEBUG
			}
			returnCode = WriteInternal(memberTypeStringEntry, memberTypeEntryPtr, paramDataPtr);
			if (returnCode == LES_RETURN_ERROR)
			{
				return LES_RETURN_ERROR;
			}
			memberDataPtr += structMember->m_dataSize;
		}
		return returnCode;
	}

	if (typeFlags & LES_TYPE_POD)
	{
#if LES_PARAMETER_DEBUG
		LES_LOG("Write type:'%s' size:%d %p -> %p\n", typeStringEntry->m_str, typeEntryPtr->m_dataSize, valueAddress, m_currentWriteBufferPtr);
#endif // #if LES_PARAMETER_DEBUG
		memcpy(m_currentWriteBufferPtr, valueAddress, parameterDataSize);
		m_currentWriteBufferPtr += parameterDataSize;
	}

	return LES_RETURN_OK;
}

