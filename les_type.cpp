#include "les_type.h"
#include "les_core.h"
#include "les_logger.h"
#include "les_stringentry.h"
#include "les_struct.h"

#include <string.h>

#define LES_TYPE_DEBUG 0

static LES_TypeEntry* les_typeEntryArray = LES_NULL;
static int les_numTypeEntries = 0;

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
	/* This is horribly slow - need hash lookup table */
	for (int i=0; i<les_numTypeEntries; i++)
	{
		const LES_TypeEntry* const typeEntryPtr = &les_typeEntryArray[i];
		if (typeEntryPtr->m_hash == hash)
		{
			return i;
		}
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Public External functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

const LES_TypeEntry* LES_GetTypeEntry(const LES_StringEntry* const typeStringEntry)
{
	const LES_Hash hash = typeStringEntry->m_hash;
	int index = LES_GetTypeEntrySlow(hash);
	if ((index >= 0) && (index < les_numTypeEntries))
	{
		const LES_TypeEntry* const typeEntryPtr = &les_typeEntryArray[index];
		return typeEntryPtr;
	}
	return LES_NULL;
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
		LES_LOG("Type 0x%X alias:%d flags:0x%X\n", typeEntryPtr->m_hash, aliasedTypeID, flags);
#endif // #if LES_TYPE_DEBUG
		const LES_StringEntry* const aliasedStringTypeEntry = LES_GetStringEntryForID(aliasedTypeID);
		if (aliasedStringTypeEntry == LES_NULL)
		{
			LES_WARNING("ComputeDataStorageSize aliased type:%d entry can't be found\n", aliasedTypeID);
			return -1;
		}
		const LES_TypeEntry* const aliasedTypeEntryPtr = LES_GetTypeEntry(aliasedStringTypeEntry);
		if (aliasedTypeEntryPtr == LES_NULL)
		{
			LES_WARNING("ComputeDataStorageSize aliased type not found aliased type:'%s'\n", aliasedStringTypeEntry->m_str);
			return -1;
		}
		typeEntryPtr = (const LES_TypeEntry*)aliasedTypeEntryPtr;
		flags = typeEntryPtr->m_flags;
#if LES_TYPE_DEBUG
		LES_LOG("Alias Type 0x%X flags:0x%X\n", typeEntryPtr->m_hash, flags);
#endif // #if LES_TYPE_DEBUG
	}
	if (flags & LES_TYPE_STRUCT)
	{
		const LES_StructDefinition* const structDefinition = LES_GetStructDefinition(typeEntryPtr->m_hash);
		if (structDefinition == LES_NULL)
		{
			LES_WARNING("ComputeDataStorage type:0x%X is a struct but can't be found\n", typeEntryPtr->m_hash);
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
			LES_LOG("Struct Type:0x%X Size:%d NumELements:%d\n", typeEntryPtr->m_hash, totalDataSize, numElements);
#endif // #if LES_TYPE_DEBUG
			totalDataSize *= numElements;
		}
		return totalDataSize;
	}
	int dataSize = typeEntryPtr->m_dataSize;
	if (numElements > 1)
	{
#if LES_TYPE_DEBUG
		LES_LOG("POD Type:0x%X Size:%d NumELements:%d\n", typeEntryPtr->m_hash, dataSize, numElements);
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
			LES_LOG("Type 0x%X alias:%d flags:0x%X\n", typeEntryPtr->m_hash, aliasedTypeID, flags);
#endif // #if LES_TYPE_DEBUG
			const LES_StringEntry* const aliasedStringTypeEntry = LES_GetStringEntryForID(aliasedTypeID);
			if (aliasedStringTypeEntry == LES_NULL)
			{
				LES_FATAL_ERROR("ComputeAlignment aliased type:%d entry can't be found\n", aliasedTypeID);
				return -1;
			}
			const LES_TypeEntry* const aliasedTypeEntryPtr = LES_GetTypeEntry(aliasedStringTypeEntry);
			if (aliasedTypeEntryPtr == LES_NULL)
			{
				LES_FATAL_ERROR("ComputeAlignment aliased type not found aliased type:'%s'\n", aliasedStringTypeEntry->m_str);
				return -1;
			}
#if LES_TYPE_DEBUG
			LES_LOG("Compute Alignment Type 0x%X alias:%s\n", typeEntryPtr->m_hash, aliasedStringTypeEntry->m_str);
#endif // #if LES_TYPE_DEBUG
			typeEntryPtr = (const LES_TypeEntry*)aliasedTypeEntryPtr;
			flags = typeEntryPtr->m_flags;
#if LES_TYPE_DEBUG
			LES_LOG("Alias Type 0x%X flags:0x%X\n", typeEntryPtr->m_hash, flags);
#endif // #if LES_TYPE_DEBUG
		}
	}

	if (flags & LES_TYPE_STRUCT)
	{
		while (flags & LES_TYPE_ALIAS)
		{
			const int aliasedTypeID = typeEntryPtr->m_aliasedTypeID;
#if LES_TYPE_DEBUG
			LES_LOG("Type 0x%X alias:%d flags:0x%X\n", typeEntryPtr->m_hash, aliasedTypeID, flags);
#endif // #if LES_TYPE_DEBUG
			const LES_StringEntry* const aliasedStringTypeEntry = LES_GetStringEntryForID(aliasedTypeID);
			if (aliasedStringTypeEntry == LES_NULL)
			{
				LES_FATAL_ERROR("ComputeAlignment aliased type:%d entry can't be found\n", aliasedTypeID);
				return -1;
			}
			const LES_TypeEntry* const aliasedTypeEntryPtr = LES_GetTypeEntry(aliasedStringTypeEntry);
			if (aliasedTypeEntryPtr == LES_NULL)
			{
				LES_FATAL_ERROR("ComputeAlignment aliased type not found aliased type:'%s'\n", aliasedStringTypeEntry->m_str);
				return -1;
			}
#if LES_TYPE_DEBUG
			LES_LOG("Compute Alignment Type 0x%X alias:%s\n", typeEntryPtr->m_hash, aliasedStringTypeEntry->m_str);
#endif // #if LES_TYPE_DEBUG
			typeEntryPtr = (const LES_TypeEntry*)aliasedTypeEntryPtr;
			flags = typeEntryPtr->m_flags;
#if LES_TYPE_DEBUG
			LES_LOG("Alias Type 0x%X flags:0x%X\n", typeEntryPtr->m_hash, flags);
#endif // #if LES_TYPE_DEBUG
		}

		const LES_StructDefinition* const structDefinition = LES_GetStructDefinition(typeEntryPtr->m_hash);
		if (structDefinition == LES_NULL)
		{
			LES_FATAL_ERROR("ComputeAlignment type:0x%X is a struct but can't be found\n", typeEntryPtr->m_hash);
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
		LES_LOG("Compute Alignment Type 0x%X Struct:%d alignment:%d\n", typeEntryPtr->m_hash, structDefinition->GetNameID(), maxAlignment);
#endif // #if LES_TYPE_DEBUG
		return maxAlignment;
	}
	const int dataSize = typeEntryPtr->m_dataSize;
	const int alignment = dataSize;
#if LES_TYPE_DEBUG
	LES_LOG("Compute Alignment Type 0x%X alignment:%d\n", typeEntryPtr->m_hash, alignment);
#endif // #if LES_TYPE_DEBUG
	return alignment;
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
		LES_WARNING("AddType '%s' : aliasedEntry '%s' not found\n", name, aliasedName);
		return LES_RETURN_ERROR;
	}
	const int aliasedTypeID = LES_AddStringEntry(aliasedName);
#if LES_TYPE_DEBUG
	LES_LOG("aliasedTypeID:%d name:'%s' aliasedName:'%s'\n", aliasedTypeID, name, aliasedName);
#endif // #if LES_TYPE_DEBUG

	const LES_Hash hash = LES_GenerateHashCaseSensitive(name);

	unsigned int flags = inputFlags;
	if (numElements >= 1)
	{
		if ((flags & LES_TYPE_ARRAY) == 0)
		{
			LES_ERROR("Type '%s' numElements:%d but not an array\n", name, numElements);
		}
		flags |= LES_TYPE_ARRAY;
	}
	const bool isArray = flags & LES_TYPE_ARRAY;
	const LES_Hash aliasedHash = LES_GenerateHashCaseSensitive(aliasedName);
	if (aliasedHash != hash)
	{
		flags |= LES_TYPE_ALIAS;
#if LES_TYPE_DEBUG
		LES_LOG("Type '%s' has an alias to '%s'\n", name, aliasedName);
#endif // #if LES_TYPE_DEBUG
	}
	if (isArray)
	{
		if (numElements <= 0)
		{
			LES_WARNING("AddType '%s' NumELements:%d Invalid number of elements must be > 0\n", name, numElements);
			return LES_RETURN_ERROR;
		}
	}

	int index = LES_GetTypeEntrySlow(hash);
	if ((index < 0) || (index >= les_numTypeEntries))
	{
		/* Not found so add it */
		index = les_numTypeEntries;
		LES_TypeEntry* const typeEntryPtr = &les_typeEntryArray[index];
		typeEntryPtr->m_hash = hash;
		typeEntryPtr->m_dataSize = dataSize;
		typeEntryPtr->m_flags = flags;
		typeEntryPtr->m_aliasedTypeID = aliasedTypeID;
		typeEntryPtr->m_numElements = numElements;

		if (isArray)
		{
			// Array types must be aliased to something
			if ((typeEntryPtr->m_flags & LES_TYPE_ALIAS) == 0)
			{
				LES_WARNING("AddType '%s' hash 0x%X is an array with %d elements but not aliased, arrays must be aliased\n",
										name, hash, numElements);
				return LES_RETURN_ERROR;
			}
			const int aliasedIndex = LES_GetTypeEntrySlow(aliasedHash);
			if (aliasedIndex == -1)
			{
				LES_WARNING("AddType '%s' hash 0x%X can't find its aliased type '%s'\n", name, hash, aliasedName);
				return LES_RETURN_ERROR;
			}
			LES_TypeEntry* const aliasedTypeEntryPtr = &les_typeEntryArray[aliasedIndex];
			const unsigned int aliasedFlags = aliasedTypeEntryPtr->m_flags;
			// Arrays of non-references must not be aliased to pointer or reference
			if (((typeEntryPtr->m_flags & LES_TYPE_REFERENCE) == 0) && ((aliasedFlags & LES_TYPE_POINTER) || (aliasedFlags & LES_TYPE_REFERENCE)))
			{
				LES_WARNING("AddType '%s' hash 0x%X non-reference array types must be aliased to a non-pointer, non-reference type Alias:'%s' Flags:0x%X\n", 
										name, hash, aliasedName, aliasedFlags);
				return LES_RETURN_ERROR;
			}
			// Arrays of references must be aliased to pointer 
			if (((typeEntryPtr->m_flags & LES_TYPE_REFERENCE) == 1) && (((aliasedFlags & LES_TYPE_POINTER) == 0) || (aliasedFlags & LES_TYPE_REFERENCE)))
			{
				LES_WARNING("AddType '%s' hash 0x%X non-reference array types must be aliased to a pointer type Alias:'%s' Flags:0x%X\n", 
										name, hash, aliasedName, aliasedFlags);
				return LES_RETURN_ERROR;
			}
		}

		les_numTypeEntries++;
	}
	else
	{
		/* Check the type data matches */
		LES_TypeEntry* const typeEntryPtr = &les_typeEntryArray[index];
		if (typeEntryPtr->m_dataSize != dataSize)
		{
			LES_WARNING("AddType '%s' hash 0x%X already in list and dataSize doesn't match Existing:%d New:%d\n",
							name, hash, typeEntryPtr->m_dataSize, dataSize);
			return LES_RETURN_ERROR;
		}
		if (typeEntryPtr->m_flags != flags)
		{
			LES_WARNING("AddType '%s' hash 0x%X already in list and flags doesn't match Existing:0x%X New:0x%X\n",
							name, hash, typeEntryPtr->m_flags, flags);
			return LES_RETURN_ERROR;
		}
		if (typeEntryPtr->m_aliasedTypeID != aliasedTypeID)
		{
			LES_WARNING("AddType '%s' hash 0x%X already in list and aliasedTypeID doesn't match Existing:%d New:%d\n",
							name, hash, typeEntryPtr->m_aliasedTypeID, aliasedTypeID);
			return LES_RETURN_ERROR;
		}
		if (typeEntryPtr->m_numElements != numElements)
		{
			LES_WARNING("AddType '%s' hash 0x%X already in list and numElements doesn't match Existing:%d New:%d\n",
							name, hash, typeEntryPtr->m_numElements, numElements);
			return LES_RETURN_ERROR;
		}
	}
	return index;
}

void LES_DebugOutputTypes(void)
{
	for (int i=0; i<les_numTypeEntries; i++)
	{
		const LES_TypeEntry* const typeEntryPtr = &les_typeEntryArray[i];
		const LES_StringEntry* nameEntry = LES_GetStringEntryByHash(typeEntryPtr->m_hash);
		const char* const name = nameEntry ? nameEntry->m_str : "NULL";
		const int dataSize = typeEntryPtr->m_dataSize;
		const unsigned int flags = typeEntryPtr->m_flags;
		const LES_StringEntry* const aliasedStringTypeEntry = LES_GetStringEntryForID(typeEntryPtr->m_aliasedTypeID);
		const char* const aliasedName = aliasedStringTypeEntry ? aliasedStringTypeEntry->m_str : "NULL";
		const int numElements = typeEntryPtr->m_numElements;

		char flagsDecoded[1024];
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
		LES_LOG("Type '%s' size:%d flags:0x%X %s aliasedName:'%s' numElements:%d\n",
			 			name, dataSize, flags, flagsDecoded, aliasedName, numElements);
	}
}
