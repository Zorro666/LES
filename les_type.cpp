#include "les_type.h"
#include "les_core.h"
#include "les_logger.h"
#include "les_loggerchannel.h"
#include "les_stringentry.h"
#include "les_struct.h"
#include "les_typedata.h"

#include <string.h>

#define LES_TYPE_DEBUG 0

static LES_TypeEntry* les_typeEntryArray = LES_NULL;
static int les_numTypeEntries = 0;

static const LES_TypeData* les_pTypeData = LES_NULL;
static int les_typeDataNumTypes = 0;

LES_Hash LES_TypeEntry::s_longlongHash = LES_GenerateHashCaseSensitive("long long int");
LES_Hash LES_TypeEntry::s_intHash = LES_GenerateHashCaseSensitive("int");
LES_Hash LES_TypeEntry::s_shortHash = LES_GenerateHashCaseSensitive("short");
LES_Hash LES_TypeEntry::s_charHash = LES_GenerateHashCaseSensitive("char");

LES_Hash LES_TypeEntry::s_uintHash = LES_GenerateHashCaseSensitive("unsigned int");
LES_Hash LES_TypeEntry::s_ushortHash = LES_GenerateHashCaseSensitive("unsigned short");
LES_Hash LES_TypeEntry::s_ucharHash = LES_GenerateHashCaseSensitive("unsigned char");

LES_Hash LES_TypeEntry::s_floatHash = LES_GenerateHashCaseSensitive("float");

extern int LES_AddStringEntry(const char* const str);

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Static Internal functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

static int LES_GetTypeEntrySlow(const LES_Hash hash)
{
	if (les_pTypeData)
	{
		const int index = les_pTypeData->GetTypeEntrySlow(hash);
		if (index >= 0)
		{
			return index;
		}
	}

	/* This is horribly slow - need hash lookup table */
	for (int i = 0; i < les_numTypeEntries; i++)
	{
		const LES_TypeEntry* const typeEntryPtr = &les_typeEntryArray[i];
		if (typeEntryPtr->m_hash == hash)
		{
			return (i + les_typeDataNumTypes);
		}
	}
	return -1;
}

static const LES_TypeEntry* LES_GetTypeEntryForID(const int id)
{
	if (id < 0)
	{
		return LES_NULL;
	}
	const int index = (id - les_typeDataNumTypes);
	if (index < 0)
	{
		// Get it from definition file type data
		const LES_TypeEntry* const typeEntryPtr = les_pTypeData->GetTypeEntry(id);
		return typeEntryPtr;
	}
	if (index >= 0)
	{
		const LES_TypeEntry* const typeEntryPtr = &les_typeEntryArray[index];
		return typeEntryPtr;
	}
	return LES_NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Public External functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

const LES_TypeEntry* LES_GetTypeEntry(const LES_StringEntry* const typeStringEntry)
{
	const LES_Hash hash = typeStringEntry->m_hash;
	const int id = LES_GetTypeEntrySlow(hash);
	const LES_TypeEntry* const pTypeEntry = LES_GetTypeEntryForID(id);
	return pTypeEntry;
}

int LES_TypeEntry::ComputeDataStorageSize(void) const
{
	const LES_TypeEntry* typeEntryPtr = this;
	unsigned int flags = typeEntryPtr->m_flags;
	const int numElements = typeEntryPtr->m_numElements;
	while (flags & LES_TYPE_ALIAS)
	{
		const int aliasedTypeID = typeEntryPtr->m_aliasedTypeID;
#if LES_TYPE_DEBUG
		LES_LOG("Type 0x%X alias:%d flags:0x%X", typeEntryPtr->m_hash, aliasedTypeID, flags);
#endif // #if LES_TYPE_DEBUG
		const LES_StringEntry* const aliasedStringTypeEntry = LES_GetStringEntryForID(aliasedTypeID);
		if (aliasedStringTypeEntry == LES_NULL)
		{
			LES_WARNING("ComputeDataStorageSize aliased type:%d entry can't be found", aliasedTypeID);
			return -1;
		}
		const LES_TypeEntry* const aliasedTypeEntryPtr = LES_GetTypeEntry(aliasedStringTypeEntry);
		if (aliasedTypeEntryPtr == LES_NULL)
		{
			LES_WARNING("ComputeDataStorageSize aliased type not found aliased type:'%s'", aliasedStringTypeEntry->m_str);
			return -1;
		}
		typeEntryPtr = (const LES_TypeEntry*)aliasedTypeEntryPtr;
		flags = typeEntryPtr->m_flags;
#if LES_TYPE_DEBUG
		LES_LOG("Alias Type 0x%X flags:0x%X", typeEntryPtr->m_hash, flags);
#endif // #if LES_TYPE_DEBUG
	}
	if (flags & LES_TYPE_STRUCT)
	{
		const LES_StructDefinition* const structDefinition = LES_GetStructDefinition(typeEntryPtr->m_hash);
		if (structDefinition == LES_NULL)
		{
			LES_WARNING("ComputeDataStorage type:0x%X is a struct but can't be found", typeEntryPtr->m_hash);
			return -1;
		}
		const int numMembers = structDefinition->GetNumMembers();
		int totalDataSize = 0;
		for (int i = 0; i < numMembers; i++)
		{
			const LES_StructMember* const structMember = structDefinition->GetMemberByIndex(i);
			const int memberTypeID = structMember->m_typeID;
			const LES_StringEntry* const memberTypeStringEntry = LES_GetStringEntryForID(memberTypeID);
			const LES_TypeEntry* const memberTypeEntryPtr = LES_GetTypeEntry(memberTypeStringEntry);
			const int dataSize = memberTypeEntryPtr->ComputeDataStorageSize();
			if (dataSize == -1)
			{
				return -1;
			}
			totalDataSize += dataSize;
		}
		if (numElements > 1)
		{
#if LES_TYPE_DEBUG
			LES_LOG("Struct Type:0x%X Size:%d NumELements:%d", typeEntryPtr->m_hash, totalDataSize, numElements);
#endif // #if LES_TYPE_DEBUG
			totalDataSize *= numElements;
		}
		return totalDataSize;
	}
	int dataSize = typeEntryPtr->m_dataSize;
	if (numElements > 1)
	{
#if LES_TYPE_DEBUG
		LES_LOG("POD Type:0x%X Size:%d NumELements:%d", typeEntryPtr->m_hash, dataSize, numElements);
#endif // #if LES_TYPE_DEBUG
		dataSize *= numElements;
	}
	return dataSize;
}

int LES_TypeEntry::ComputeAlignment(void) const
{
	const LES_TypeEntry* typeEntryPtr = this;
	unsigned int flags = typeEntryPtr->m_flags;
	if (flags & LES_TYPE_ARRAY)
	{
		while (flags & LES_TYPE_ALIAS)
		{
			const int aliasedTypeID = typeEntryPtr->m_aliasedTypeID;
#if LES_TYPE_DEBUG
			LES_LOG("Type 0x%X alias:%d flags:0x%X", typeEntryPtr->m_hash, aliasedTypeID, flags);
#endif // #if LES_TYPE_DEBUG
			const LES_StringEntry* const aliasedStringTypeEntry = LES_GetStringEntryForID(aliasedTypeID);
			if (aliasedStringTypeEntry == LES_NULL)
			{
				LES_FATAL_ERROR("ComputeAlignment aliased type:%d entry can't be found", aliasedTypeID);
				return -1;
			}
			const LES_TypeEntry* const aliasedTypeEntryPtr = LES_GetTypeEntry(aliasedStringTypeEntry);
			if (aliasedTypeEntryPtr == LES_NULL)
			{
				LES_FATAL_ERROR("ComputeAlignment aliased type not found aliased type:'%s'", aliasedStringTypeEntry->m_str);
				return -1;
			}
#if LES_TYPE_DEBUG
			LES_LOG("Compute Alignment Type 0x%X alias:%s", typeEntryPtr->m_hash, aliasedStringTypeEntry->m_str);
#endif // #if LES_TYPE_DEBUG
			typeEntryPtr = (const LES_TypeEntry*)aliasedTypeEntryPtr;
			flags = typeEntryPtr->m_flags;
#if LES_TYPE_DEBUG
			LES_LOG("Alias Type 0x%X flags:0x%X", typeEntryPtr->m_hash, flags);
#endif // #if LES_TYPE_DEBUG
		}
	}

	if (flags & LES_TYPE_STRUCT)
	{
		while (flags & LES_TYPE_ALIAS)
		{
			const int aliasedTypeID = typeEntryPtr->m_aliasedTypeID;
#if LES_TYPE_DEBUG
			LES_LOG("Type 0x%X alias:%d flags:0x%X", typeEntryPtr->m_hash, aliasedTypeID, flags);
#endif // #if LES_TYPE_DEBUG
			const LES_StringEntry* const aliasedStringTypeEntry = LES_GetStringEntryForID(aliasedTypeID);
			if (aliasedStringTypeEntry == LES_NULL)
			{
				LES_FATAL_ERROR("ComputeAlignment aliased type:%d entry can't be found", aliasedTypeID);
				return -1;
			}
			const LES_TypeEntry* const aliasedTypeEntryPtr = LES_GetTypeEntry(aliasedStringTypeEntry);
			if (aliasedTypeEntryPtr == LES_NULL)
			{
				LES_FATAL_ERROR("ComputeAlignment aliased type not found aliased type:'%s'", aliasedStringTypeEntry->m_str);
				return -1;
			}
#if LES_TYPE_DEBUG
			LES_LOG("Compute Alignment Type 0x%X alias:%s", typeEntryPtr->m_hash, aliasedStringTypeEntry->m_str);
#endif // #if LES_TYPE_DEBUG
			typeEntryPtr = (const LES_TypeEntry*)aliasedTypeEntryPtr;
			flags = typeEntryPtr->m_flags;
#if LES_TYPE_DEBUG
			LES_LOG("Alias Type 0x%X flags:0x%X", typeEntryPtr->m_hash, flags);
#endif // #if LES_TYPE_DEBUG
		}

		const LES_StructDefinition* const structDefinition = LES_GetStructDefinition(typeEntryPtr->m_hash);
		if (structDefinition == LES_NULL)
		{
			LES_FATAL_ERROR("ComputeAlignment type:0x%X is a struct but can't be found", typeEntryPtr->m_hash);
			return -1;
		}
		const int numMembers = structDefinition->GetNumMembers();
		int maxAlignment = 0;
		for (int i = 0; i < numMembers; i++)
		{
			const LES_StructMember* const structMember = structDefinition->GetMemberByIndex(i);
			const int memberTypeID = structMember->m_typeID;
			const LES_StringEntry* const memberTypeStringEntry = LES_GetStringEntryForID(memberTypeID);
			const LES_TypeEntry* const memberTypeEntryPtr = LES_GetTypeEntry(memberTypeStringEntry);
			const int alignment = memberTypeEntryPtr->ComputeAlignment();
			if (alignment > maxAlignment)
			{
				maxAlignment = alignment;
			}
		}
#if LES_TYPE_DEBUG
		LES_LOG("Compute Alignment Type 0x%X Struct:%d alignment:%d", typeEntryPtr->m_hash, structDefinition->GetNameID(), maxAlignment);
#endif // #if LES_TYPE_DEBUG
		return maxAlignment;
	}
	const int dataSize = typeEntryPtr->m_dataSize;
	const int alignment = dataSize;
#if LES_TYPE_DEBUG
	LES_LOG("Compute Alignment Type 0x%X alignment:%d", typeEntryPtr->m_hash, alignment);
#endif // #if LES_TYPE_DEBUG
	return alignment;
}

void LES_DebugOutputTypes(LES_LoggerChannel* const pLogChannel)
{
	const int numTypes = les_typeDataNumTypes + les_numTypeEntries;
	for (int i = 0; i < numTypes; i++)
	{
		const LES_TypeEntry* const typeEntryPtr = LES_GetTypeEntryForID(i);
		const LES_StringEntry* nameEntry = LES_GetStringEntryByHash(typeEntryPtr->m_hash);
		const char* const name = nameEntry ? nameEntry->m_str : "NULL";
		const unsigned int dataSize = typeEntryPtr->m_dataSize;
		const unsigned int flags = typeEntryPtr->m_flags;
		const LES_StringEntry* const aliasedStringTypeEntry = LES_GetStringEntryForID(typeEntryPtr->m_aliasedTypeID);
		const char* const aliasedName = aliasedStringTypeEntry ? aliasedStringTypeEntry->m_str : "NULL";
		const int numElements = typeEntryPtr->m_numElements;

		char flagsDecoded[1024];
		LES_Type_DecodeFlags(flagsDecoded, flags);
		pLogChannel->Print("Type '%s' size:%d flags:0x%X %s aliasedName:'%s' numElements:%d",
												name, dataSize, flags, flagsDecoded, aliasedName, numElements);
	}
}

void LES_Type_DecodeFlags(char* const flagsDecoded, const LES_uint flags)
{
	flagsDecoded[0] = '\0';
	bool needsPipe = false;
	if (flags & LES_TYPE_INPUT)
	{
		strcat(flagsDecoded, "INPUT");
		needsPipe = true;
	}
	if (flags & LES_TYPE_OUTPUT)
	{
		if (needsPipe)
		{
			strcat(flagsDecoded, "|");
		}
		strcat(flagsDecoded, "OUTPUT");
		needsPipe = true;
	}
	if (flags & LES_TYPE_POD)
	{
		if (needsPipe)
		{
			strcat(flagsDecoded, "|");
		}
		strcat(flagsDecoded, "POD");
		needsPipe = true;
	}
	if (flags & LES_TYPE_STRUCT)
	{
		if (needsPipe)
		{
			strcat(flagsDecoded, "|");
		}
		strcat(flagsDecoded, "STRUCT");
		needsPipe = true;
	}
	if (flags & LES_TYPE_POINTER)
	{
		if (needsPipe)
		{
			strcat(flagsDecoded, "|");
		}
		strcat(flagsDecoded, "POINTER");
		needsPipe = true;
	}
	if (flags & LES_TYPE_REFERENCE)
	{
		if (needsPipe)
		{
			strcat(flagsDecoded, "|");
		}
		strcat(flagsDecoded, "REFERENCE");
		needsPipe = true;
	}
	if (flags & LES_TYPE_ALIAS)
	{
		if (needsPipe)
		{
			strcat(flagsDecoded, "|");
		}
		strcat(flagsDecoded, "ALIAS");
		needsPipe = true;
	}
	if (flags & LES_TYPE_ARRAY)
	{
		if (needsPipe)
		{
			strcat(flagsDecoded, "|");
		}
		strcat(flagsDecoded, "ARRAY");
		needsPipe = true;
	}
}

void LES_Type_SetTypeDataPtr(const LES_TypeData* const pTypeData)
{
	les_pTypeData = pTypeData;
	const int numTypes = pTypeData->GetNumTypes();
	les_typeDataNumTypes = numTypes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Private External functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

void LES_TypeInit(void)
{
	les_typeEntryArray = new LES_TypeEntry[1024];
	les_numTypeEntries = 0;
}

void LES_TypeShutdown(void)
{
	les_numTypeEntries = 0;
	delete[] les_typeEntryArray;
}

int LES_AddType(const char* const name, const unsigned int dataSize, const unsigned int inputFlags, 
								const char* const aliasedName, const int numElements)
{
	LES_AddStringEntry(name);

	const LES_StringEntry* const aliasedEntryPtr = LES_GetStringEntry(aliasedName);
	if (aliasedEntryPtr == LES_NULL)
	{
		LES_WARNING("AddType '%s' : aliasedEntry '%s' not found", name, aliasedName);
		return LES_RETURN_ERROR;
	}
	const int aliasedTypeID = LES_AddStringEntry(aliasedName);
#if LES_TYPE_DEBUG
	LES_LOG("aliasedTypeID:%d name:'%s' aliasedName:'%s'", aliasedTypeID, name, aliasedName);
#endif // #if LES_TYPE_DEBUG

	const LES_Hash hash = LES_GenerateHashCaseSensitive(name);

	unsigned int flags = inputFlags;
	if (numElements >= 1)
	{
		if ((flags & LES_TYPE_ARRAY) == 0)
		{
			LES_ERROR("Type '%s' numElements:%d but not an array", name, numElements);
		}
		flags |= LES_TYPE_ARRAY;
	}
	const bool isArray = flags & LES_TYPE_ARRAY;
	const LES_Hash aliasedHash = LES_GenerateHashCaseSensitive(aliasedName);
	if (aliasedHash != hash)
	{
		flags |= LES_TYPE_ALIAS;
#if LES_TYPE_DEBUG
		LES_LOG("Type '%s' has an alias to '%s'", name, aliasedName);
#endif // #if LES_TYPE_DEBUG
	}
	if (isArray)
	{
		if (numElements <= 0)
		{
			LES_WARNING("AddType '%s' NumELements:%d Invalid number of elements must be > 0", name, numElements);
			return LES_RETURN_ERROR;
		}
	}

	int index = LES_GetTypeEntrySlow(hash);
	if (index < 0)
	{
		/* Not found so make a new type and run error checks on it - needed to make some tests work */
		LES_TypeEntry typeEntry;
		typeEntry.m_dataSize = dataSize;
		typeEntry.m_flags= flags;
		typeEntry.m_aliasedTypeID = aliasedTypeID;
		typeEntry.m_numElements = numElements;

		if (isArray)
		{
			// Array types must be aliased to something
			if ((flags & LES_TYPE_ALIAS) == 0)
			{
				LES_WARNING("AddType '%s' hash 0x%X is an array with %d elements but not aliased, arrays must be aliased",
										name, hash, numElements);
				return LES_RETURN_ERROR;
			}
			const int aliasedIndex = LES_GetTypeEntrySlow(aliasedHash);
			if (aliasedIndex == -1)
			{
				LES_WARNING("AddType '%s' hash 0x%X can't find its aliased type '%s'", name, hash, aliasedName);
				return LES_RETURN_ERROR;
			}
			const LES_TypeEntry* const aliasedTypeEntryPtr = LES_GetTypeEntryForID(aliasedIndex);
			const unsigned int aliasedFlags = aliasedTypeEntryPtr->m_flags;
			// Arrays of non-references must not be aliased to pointer or reference
			if (((flags & LES_TYPE_REFERENCE) == 0) && ((aliasedFlags & LES_TYPE_POINTER) || (aliasedFlags & LES_TYPE_REFERENCE)))
			{
				LES_WARNING("AddType '%s' hash 0x%X non-reference array types must be aliased to a non-pointer, non-reference type Alias:'%s' Flags:0x%X", 
										name, hash, aliasedName, aliasedFlags);
				return LES_RETURN_ERROR;
			}
			// Arrays of references must be aliased to pointer 
			if (((flags & LES_TYPE_REFERENCE) == 1) && (((aliasedFlags & LES_TYPE_POINTER) == 0) || (aliasedFlags & LES_TYPE_REFERENCE)))
			{
				LES_WARNING("AddType '%s' hash 0x%X non-reference array types must be aliased to a pointer type Alias:'%s' Flags:0x%X", 
										name, hash, aliasedName, aliasedFlags);
				return LES_RETURN_ERROR;
			}
		}

		if (les_pTypeData)
		{
			LES_ERROR("AddType '%s' hash 0x%X not found in type data definition file", name, hash);
			return LES_RETURN_ERROR;
		}

		/* Add the new type */
		index = les_numTypeEntries;
		LES_TypeEntry* const typeEntryPtr = &les_typeEntryArray[index];
		*typeEntryPtr = typeEntry;

		index += les_typeDataNumTypes;
		les_numTypeEntries++;
	}
	else
	{
		/* Check the type data matches */
		const LES_TypeEntry* const typeEntryPtr = LES_GetTypeEntryForID(index);
		if (typeEntryPtr->m_dataSize != dataSize)
		{
			LES_WARNING("AddType '%s' hash 0x%X already in list and dataSize doesn't match Existing:%d New:%d",
							name, hash, typeEntryPtr->m_dataSize, dataSize);
			return LES_RETURN_ERROR;
		}
		if (typeEntryPtr->m_flags != flags)
		{
			LES_WARNING("AddType '%s' hash 0x%X already in list and flags doesn't match Existing:0x%X New:0x%X",
							name, hash, typeEntryPtr->m_flags, flags);
			return LES_RETURN_ERROR;
		}
		if (typeEntryPtr->m_aliasedTypeID != aliasedTypeID)
		{
			LES_WARNING("AddType '%s' hash 0x%X already in list and aliasedTypeID doesn't match Existing:%d New:%d",
							name, hash, typeEntryPtr->m_aliasedTypeID, aliasedTypeID);
			return LES_RETURN_ERROR;
		}
		if (typeEntryPtr->m_numElements != numElements)
		{
			LES_WARNING("AddType '%s' hash 0x%X already in list and numElements doesn't match Existing:%d New:%d",
							name, hash, typeEntryPtr->m_numElements, numElements);
			return LES_RETURN_ERROR;
		}
	}
	return index;
}

