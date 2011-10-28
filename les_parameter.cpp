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
	if (typeEntryPtr == LES_NULL)
	{
		LES_WARNING("LES_FunctionParameterData::Read type:'%s' not found", typeStringEntry->m_str);
		return LES_RETURN_ERROR;
	}
	const LES_StringEntry* const aliasedStringEntryPtr = LES_GetStringEntryForID(typeEntryPtr->m_aliasedTypeID);
	if (aliasedStringEntryPtr == LES_NULL)
	{
		LES_WARNING("LES_FunctionParameterData::Read type:'%s' aliasedStringEntry:%d not found", 
								typeStringEntry->m_str, typeEntryPtr->m_aliasedTypeID);
		return LES_RETURN_ERROR;
	}
	const LES_TypeEntry* const aliasedTypeEntryPtr = LES_GetTypeEntry(aliasedStringEntryPtr);
	if (aliasedTypeEntryPtr == LES_NULL)
	{
		LES_WARNING("LES_FunctionParameterData::Read type:'%s' aliased type:'%s' not found", 
								typeStringEntry->m_str, aliasedStringEntryPtr->m_str);
		return LES_RETURN_ERROR;
	}
	if (parameterDataPtr == LES_NULL)
	{
		LES_WARNING("LES_FunctionParameterData::Read type:'%s' parameterDataPtr is NULL", typeStringEntry->m_str);
		return LES_RETURN_ERROR;
	}

	const void* valueAddress = parameterDataPtr;
	unsigned int flags = typeEntryPtr->m_flags;
	unsigned int parameterDataSize = typeEntryPtr->m_dataSize;
	if (flags & LES_TYPE_POINTER)
	{
		parameterDataSize = aliasedTypeEntryPtr->m_dataSize;
		flags = aliasedTypeEntryPtr->m_flags;
	}
	if (valueAddress == LES_NULL)
	{
		LES_WARNING("LES_FunctionParameterData::Read type:'%s' valueAddress is NULL", typeStringEntry->m_str);
		return LES_RETURN_ERROR;
	}

	if ((flags & LES_TYPE_POD) || (flags & LES_TYPE_STRUCT))
	{
#if LES_PARAMETER_DEBUG
		LES_LOG("Read type:'%s' size:%d %p -> %p", typeStringEntry->m_str, typeEntryPtr->m_dataSize, m_currentReadBufferPtr, parameterDataPtr);
#endif // #if LES_PARAMETER_DEBUG
		if (flags & LES_TYPE_ENDIANSWAP)
		{
			if (parameterDataSize == 2)
			{
				fromBigEndian16((char* const)parameterDataPtr, m_currentReadBufferPtr);
			}
			else if (parameterDataSize == 4)
			{
				fromBigEndian32((char* const)parameterDataPtr, m_currentReadBufferPtr);
			}
			else if (parameterDataSize == 8)
			{
				fromBigEndian64((char* const)parameterDataPtr, m_currentReadBufferPtr);
			}
			else
			{
				LES_FATAL_ERROR("Read type:'%s' marked for ENDIANSWAP but unknown size to swap:%d", typeStringEntry->m_str, parameterDataSize);
			}
		}
		else
		{
			memcpy(parameterDataPtr, m_currentReadBufferPtr, parameterDataSize);
		}
		m_currentReadBufferPtr += parameterDataSize;
	}

	return LES_RETURN_OK;
}

int LES_FunctionParameterData::Write(const LES_StringEntry* const typeStringEntry, 
																 		 const void* const parameterDataPtr, const unsigned int paramMode)
{
	const LES_TypeEntry* const typeEntryPtr = LES_GetTypeEntry(typeStringEntry);
	if (typeEntryPtr == LES_NULL)
	{
		LES_WARNING("LES_FunctionParameterData::Write type:'%s' not found", typeStringEntry->m_str);
		return LES_RETURN_ERROR;
	}
	const unsigned int typeFlags = typeEntryPtr->m_flags;
	if ((typeFlags & paramMode) == 0)
	{
		char typeFlagsDecoded[1024];
		LES_Type_DecodeFlags(typeFlagsDecoded, typeFlags);
		char paramModeDecoded[1024];
		LES_Type_DecodeFlags(paramModeDecoded, paramMode);
		LES_WARNING("LES_FunctionParameterData::Write type:'%s' flags incorrect for parameter mode typeFlags:0x%X %s paramMode:0x%X %s", 
								typeStringEntry->m_str, typeFlags, typeFlagsDecoded, paramMode, paramModeDecoded);
		return LES_RETURN_ERROR;
	}
	const void* valueAddress = parameterDataPtr;
	if (typeFlags & LES_TYPE_POINTER)
	{
#if LES_PARAMETER_DEBUG
		LES_LOG("%s ptr address changing %p", typeStringEntry->m_str, parameterDataPtr);
#endif // #if LES_PARAMETER_DEBUG
		const void** pointerAddress = (const void**)parameterDataPtr;
		valueAddress = *pointerAddress;
	}
	return WriteInternal(typeStringEntry, typeEntryPtr, valueAddress);
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
	if (inputTypeEntryPtr == LES_NULL)
	{
		LES_WARNING("LES_FunctionParameterData::WriteInternal type:'%s' not found", typeStringEntry->m_str);
		return LES_RETURN_ERROR;
	}
	if (parameterDataPtr == LES_NULL)
	{
		LES_WARNING("LES_FunctionParameterData::WriteInternal type:'%s' parameterDataPtr is NULL", typeStringEntry->m_str);
		return LES_RETURN_ERROR;
	}

	unsigned int inputTypeFlags = inputTypeEntryPtr->m_flags;
	if ((inputTypeFlags & LES_TYPE_ARRAY) == 0)
	{
		return WriteItem(typeStringEntry, inputTypeEntryPtr, parameterDataPtr);
	}

	// Loop over the elements of the array writing each item 1 by 1
	const int numElements = inputTypeEntryPtr->m_numElements;
	const LES_TypeEntry* typeEntryPtr = inputTypeEntryPtr;

	typeEntryPtr = typeEntryPtr->GetRootType();

	unsigned int parameterDataSize = inputTypeEntryPtr->m_dataSize;
	parameterDataSize = typeEntryPtr->m_dataSize;

	const LES_StringEntry* const itemTypeStringEntry = typeStringEntry;
	const LES_TypeEntry* const itemTypeEntryPtr = inputTypeEntryPtr;
	//const unsigned int parameterDataSize = itemTypeEntryPtr->m_dataSize; 
	const void** pointerAddress = (const void**)parameterDataPtr;
	const void* paramDataPtr = parameterDataPtr;
	if ((inputTypeFlags & LES_TYPE_REFERENCE) == 0)
	{
		pointerAddress = (const void**)parameterDataPtr;
		paramDataPtr = *pointerAddress;
	}
	const char* itemParameterDataPtr = (const char*)paramDataPtr;
	for (int element = 0; element < numElements; element++)
	{
		const void* const voidParameterDataPtr = (const void*)itemParameterDataPtr;

		const int retError = WriteItem(itemTypeStringEntry, itemTypeEntryPtr, voidParameterDataPtr);
#if LES_PARAMETER_DEBUG
		LES_LOG("WriteItem[%d] %p 0x%X", element, voidParameterDataPtr, *itemParameterDataPtr);
#endif // #if LES_PARAMETER_DEBUG
		if (retError != LES_RETURN_OK)
		{
			return LES_RETURN_ERROR;
		}
		itemParameterDataPtr += parameterDataSize;
	}
	return LES_RETURN_OK;
}

int LES_FunctionParameterData::WriteItem(const LES_StringEntry* const typeStringEntry, const LES_TypeEntry* const inputTypeEntryPtr, 
																				 const void* const parameterDataPtr)
{
#if LES_PARAMETER_DEBUG
	LES_LOG("WriteItem type:'%s'", typeStringEntry->m_str);
#endif // #if LES_PARAMETER_DEBUG
	//const unsigned int inputTypeFlags = inputTypeEntryPtr->m_flags;
	const LES_TypeEntry* typeEntryPtr = inputTypeEntryPtr;
	unsigned int typeFlags = typeEntryPtr->m_flags;

	typeEntryPtr = typeEntryPtr->GetRootType();
	typeFlags = typeEntryPtr->m_flags;

	const void* valueAddress = parameterDataPtr;
#if 0
	if (inputTypeFlags & LES_TYPE_POINTER)
	{
#if LES_PARAMETER_DEBUG
		LES_LOG("%s ptr address changing %p", typeStringEntry->m_str, parameterDataPtr);
#endif // #if LES_PARAMETER_DEBUG
		const void** pointerAddress = (const void**)parameterDataPtr;
		valueAddress = *pointerAddress;
	}
#endif // #if 0

	const int typeDataSize = typeEntryPtr->m_dataSize;

	unsigned int parameterDataSize = typeDataSize;
	if (valueAddress == LES_NULL)
	{
		LES_WARNING("LES_FunctionParameterData::WriteItem type:'%s' valueAddress is NULL", typeStringEntry->m_str);
		return LES_RETURN_ERROR;
	}

	if (typeFlags & LES_TYPE_STRUCT)
	{
#if LES_PARAMETER_DEBUG
		LES_LOG("Write type:'%s' size:%d hash:0x%X STRUCT", typeStringEntry->m_str, typeDataSize, typeEntryPtr->m_hash);
#endif // #if LES_PARAMETER_DEBUG
		int returnCode = LES_RETURN_OK;
		const LES_StructDefinition* const structDefinition = LES_GetStructDefinition(typeEntryPtr->m_hash);
		if (structDefinition == LES_NULL)
		{
			LES_WARNING("Write type:'%s' is a struct but can't be found", typeStringEntry->m_str);
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

			const unsigned int memberFlags = memberTypeEntryPtr->m_flags;
			const bool isArray = (memberFlags & LES_TYPE_ARRAY);
			const bool isPointer = (memberFlags & LES_TYPE_POINTER);
			const bool isReference = (memberFlags & LES_TYPE_REFERENCE);
			bool deferencePtr = (!isArray & (isPointer | isReference));
			if (isArray && isReference)
			{
				deferencePtr = true;
			}
			if (isArray && (deferencePtr==false))
			{
				// The function below expects ptr to the array e.g. it dereferences it (if it isn't a reference)
				const void* pointerAddress = (const void*)memberDataPtr;
				paramDataPtr = (const void*)&pointerAddress;
			}
			else if (deferencePtr)
			{
				const void** pointerAddress = (const void**)memberDataPtr;
				paramDataPtr = *pointerAddress;
#if LES_PARAMETER_DEBUG
				LES_LOG("Member[%d] %d %s %p ptr address -> %p", i, 
								structMember->m_alignmentPadding, memberTypeStringEntry->m_str, memberDataPtr, paramDataPtr);
#endif // #if LES_PARAMETER_DEBUG
			}
#if LES_PARAMETER_DEBUG
			LES_LOG("Member[%d] Type:%s 0x%X Padding:%d Name:%s Member Ptr:%p Datasize:%d", i, 
								memberTypeStringEntry->m_str, memberTypeStringEntry->m_hash, 
								structMember->m_alignmentPadding, memberTypeStringEntry->m_str, 
								memberDataPtr, structMember->m_dataSize);
#endif // #if LES_PARAMETER_DEBUG
			returnCode = WriteInternal(memberTypeStringEntry, memberTypeEntryPtr, paramDataPtr);
			if (returnCode != LES_RETURN_OK)
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
		LES_LOG("Write type:'%s' size:%d %p -> %p", typeStringEntry->m_str, typeEntryPtr->m_dataSize, valueAddress, m_currentWriteBufferPtr);
#endif // #if LES_PARAMETER_DEBUG
		if (typeFlags & LES_TYPE_ENDIANSWAP)
		{
			if (parameterDataSize == 2)
			{
				toBigEndian16(m_currentWriteBufferPtr, (const char*)valueAddress);
			}
			else if (parameterDataSize == 4)
			{
				toBigEndian32(m_currentWriteBufferPtr, (const char*)valueAddress);
			}
			else if (parameterDataSize == 8)
			{
				toBigEndian64(m_currentWriteBufferPtr, (const char*)valueAddress);
			}
			else
			{
				LES_FATAL_ERROR("Write type:'%s' marked for ENDIANSWAP but unknown size to swap:%d", typeStringEntry->m_str, parameterDataSize);
			}
		}
		else
		{
			memcpy(m_currentWriteBufferPtr, valueAddress, parameterDataSize);
		}
		m_currentWriteBufferPtr += parameterDataSize;
	}

	return LES_RETURN_OK;
}

