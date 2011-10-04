#ifndef LES_BASE_HH
#define LES_BASE_HH

typedef unsigned int LES_uint;

typedef signed char LES_int8;
typedef unsigned char LES_uint8;

typedef signed short LES_int16;
typedef unsigned short LES_uint16;

typedef signed int LES_int32;
typedef unsigned int LES_uint32;

typedef signed long long int LES_int64;
typedef unsigned long long int LES_uint64;

typedef float LES_float32;

#define LES_NULL (0)

#define LES_RETURN_OK (1)
#define LES_RETURN_ERROR (-1)

#define BIG_ENDIAN_MACHINE (0)

inline LES_int16 swapEndian16(const LES_int16 input)
{
	// Bytes: input = A:B, output = B:A
	const LES_uint32 value = input;
	const LES_uint32 A = (((value & 0xFF00) >> 8) & 0xFF);
	const LES_uint32 B = (((value & 0x00FF) >> 0) & 0xFF);
	const LES_uint32 swapValue = (A << 0) | (B << 8);
	const LES_uint16 output = (LES_uint16)swapValue;
	return output;
}

inline LES_uint16 swapEndian16(const LES_uint16 input)
{
	// Bytes: input = A:B, output = B:A
	const LES_uint32 value = input;
	const LES_uint32 A = (((value & 0xFF00) >> 8) & 0xFF);
	const LES_uint32 B = (((value & 0x00FF) >> 0) & 0xFF);
	const LES_uint32 swapValue = (A << 0) | (B << 8);
	const LES_uint16 output = (LES_uint16)swapValue;
	return output;
}

inline LES_int32 swapEndian32(const LES_int32 input)
{
	// Bytes: input = A:B:C:D, output = D:C:B:A
	const LES_uint32 value = input;
	const LES_uint32 A = (((value & 0xFF000000) >> 24) & 0xFF);
	const LES_uint32 B = (((value & 0x00FF0000) >> 16) & 0xFF);
	const LES_uint32 C = (((value & 0x0000FF00) >> 8) & 0xFF);
	const LES_uint32 D = (((value & 0x000000FF) >> 0) & 0xFF);
	const LES_uint32 swapValue = (A << 0) | (B << 8) | (C << 16) | (D << 24);
	const LES_uint32 output = swapValue;
	return output;
}

inline LES_uint32 swapEndian32(const LES_uint32 input)
{
	// Bytes: input = A:B:C:D, output = D:C:B:A
	const LES_uint32 value = input;
	const LES_uint32 A = (((value & 0xFF000000) >> 24) & 0xFF);
	const LES_uint32 B = (((value & 0x00FF0000) >> 16) & 0xFF);
	const LES_uint32 C = (((value & 0x0000FF00) >> 8) & 0xFF);
	const LES_uint32 D = (((value & 0x000000FF) >> 0) & 0xFF);
	const LES_uint32 swapValue = (A << 0) | (B << 8) | (C << 16) | (D << 24);
	const LES_uint32 output = swapValue;
	return output;
}

#if BIG_ENDIAN_MACHINE == 1
// Big Endian machine

// Swap from LittleEndian to BigEndian
inline LES_int16 fromLittleEndian16(const LES_int16 input)
{
	return swapEndian16(input);
}

inline LES_uint16 fromLittleEndian16(const LES_uint16 input)
{
	return swapEndian16(input);
}

inline LES_int32 fromLittleEndian32(const LES_int32 input)
{
	return swapEndian32(input);
}

inline LES_uint32 fromLittleEndian32(const LES_uint32 input)
{
	return swapEndian32(input);
}

// Swap to LittleEndian from BigEndian
inline LES_int16 toLittleEndian16(const LES_int16 input)
{
	return swapEndian16(input);
}

inline LES_uint16 toLittleEndian16(const LES_uint16 input)
{
	return swapEndian16(input);
}

inline LES_int32 toLittleEndian32(const LES_int32 input)
{
	return swapEndian32(input);
}

inline LES_uint32 toLittleEndian32(const LES_uint32 input)
{
	return swapEndian32(input);
}

// Swap from BigEndian to BigEndian
inline LES_int16 fromBigEndian16(const LES_int16 input)
{
	return input;
}

inline LES_uint16 fromBigEndian16(const LES_uint16 input)
{
	return input;
}

inline LES_int32 fromBigEndian32(const LES_int32 input)
{
	return input;
}

inline LES_uint32 fromBigEndian32(const LES_uint32 input)
{
	return input;
}

inline void fromBigEndian16(char* const pOutData, const char* const pBigData)
{
	// Bytes: input = A:B, output = A:B
	pOutData[0] = pBigData[0];
	pOutData[1] = pBigData[1];
}

inline void fromBigEndian32(char* const pOutData, const char* const pBigData)
{
	// Bytes: input = A:B:C:D, output = A:B:C:D
	pOutData[0] = pBigData[0];
	pOutData[1] = pBigData[1];
	pOutData[2] = pBigData[2];
	pOutData[3] = pBigData[3];
}

inline void fromBigEndian64(char* const pOutData, const char* const pBigData)
{
	// Bytes: input = A:B:C:D:E:F:G:H, output = A:B:C:D:E:F:G:H
	pOutData[0] = pBigData[0];
	pOutData[1] = pBigData[1];
	pOutData[2] = pBigData[2];
	pOutData[3] = pBigData[3];
	pOutData[4] = pBigData[4];
	pOutData[5] = pBigData[5];
	pOutData[6] = pBigData[6];
	pOutData[7] = pBigData[7];
}

// Swap to BigEndian from BigEndian
inline LES_int16 toBigEndian16(const LES_int16 input)
{
	return input;
}

inline LES_uint16 toBigEndian16(const LES_uint16 input)
{
	return input;
}

inline LES_int32 toBigEndian32(const LES_int32 input)
{
	return input;
}

inline LES_uint32 toBigEndian32(const LES_uint32 input)
{
	return input;
}

inline void toBigEndian16(char* const pBigData, const char* const pInData)
{
	// Bytes: input = A:B, output = A:B
	pBigData[0] = pInData[0];
	pBigData[1] = pInData[1];
}

inline void toBigEndian32(char* const pBigData, const char* const pInData)
{
	// Bytes: input = A:B:C:D, output = A:B:C:D
	pBigData[0] = pInData[0];
	pBigData[1] = pInData[1];
	pBigData[2] = pInData[2];
	pBigData[3] = pInData[3];
}

inline void toBigEndian64(char* const pBigData, const char* const pInData)
{
	// Bytes: input = A:B:C:D:E:F:G:H, output = A:B:C:D:E:F:G:H
	pBigData[0] = pInData[0];
	pBigData[1] = pInData[1];
	pBigData[2] = pInData[2];
	pBigData[3] = pInData[3];
	pBigData[4] = pInData[4];
	pBigData[5] = pInData[5];
	pBigData[6] = pInData[6];
	pBigData[7] = pInData[7];
}

#else // #if BIG_ENDIAN_MACHINE == 1
// Little Endian Machine

// swap from LittleEndian to LittleEndian
inline LES_int16 fromLittleEndian16(const LES_int16 input)
{
	return input;
}

inline LES_uint16 fromLittleEndian16(const LES_uint16 input)
{
	return input;
}

inline LES_int32 fromLittleEndian32(const LES_int32 input)
{
	return input;
}

inline LES_uint32 fromLittleEndian32(const LES_uint32 input)
{
	return input;
}

// swap to LittleEndian from LittleEndian
inline LES_int16 toLittleEndian16(const LES_int16 input)
{
	return input;
}

inline LES_uint16 toLittleEndian16(const LES_uint16 input)
{
	return input;
}

inline LES_int32 toLittleEndian32(const LES_int32 input)
{
	return input;
}

inline LES_uint32 toLittleEndian32(const LES_uint32 input)
{
	return input;
}

// Swap from BigEndian to LittleEndian
inline LES_int16 fromBigEndian16(const LES_int16 input)
{
	return swapEndian16(input);
}

inline LES_uint16 fromBigEndian16(const LES_uint16 input)
{
	return swapEndian16(input);
}

inline LES_int32 fromBigEndian32(const LES_int32 input)
{
	return swapEndian32(input);
}

inline LES_uint32 fromBigEndian32(const LES_uint32 input)
{
	return swapEndian32(input);
}

inline void fromBigEndian16(char* const pOutData, const char* const pBigData)
{
	// Bytes: input = A:B, output = B:A
	pOutData[0] = pBigData[1];
	pOutData[1] = pBigData[0];
}

inline void fromBigEndian32(char* const pOutData, const char* const pBigData)
{
	// Bytes: input = A:B:C:D, output = D:C:B:A
	pOutData[0] = pBigData[3];
	pOutData[1] = pBigData[2];
	pOutData[2] = pBigData[1];
	pOutData[3] = pBigData[0];
}

inline void fromBigEndian64(char* const pOutData, const char* const pBigData)
{
	// Bytes: input = A:B:C:D:E:F:G:H, output = H:G:F:E:D:C:B:A
	pOutData[0] = pBigData[7];
	pOutData[1] = pBigData[6];
	pOutData[2] = pBigData[5];
	pOutData[3] = pBigData[4];
	pOutData[4] = pBigData[3];
	pOutData[5] = pBigData[2];
	pOutData[6] = pBigData[1];
	pOutData[7] = pBigData[0];
}

// Swap to BigEndian from LittleEndian
inline LES_int16 toBigEndian16(const LES_int16 input)
{
	return swapEndian16(input);
}

inline LES_uint16 toBigEndian16(const LES_uint16 input)
{
	return swapEndian16(input);
}

inline LES_int32 toBigEndian32(const LES_int32 input)
{
	return swapEndian32(input);
}

inline LES_uint32 toBigEndian32(const LES_uint32 input)
{
	return swapEndian32(input);
}

inline void toBigEndian16(char* const pBigData, const char* const pInData)
{
	// Bytes: input = A:B, output = B:A
	pBigData[0] = pInData[1];
	pBigData[1] = pInData[0];
}

inline void toBigEndian32(char* const pBigData, const char* const pInData)
{
	// Bytes: input = A:B:C:D, output = D:C:B:A
	pBigData[0] = pInData[3];
	pBigData[1] = pInData[2];
	pBigData[2] = pInData[1];
	pBigData[3] = pInData[0];
}

inline void toBigEndian64(char* const pBigData, const char* const pInData)
{
	// Bytes: input = A:B:C:D:E:F:G:H, output = H:G:F:E:D:C:B:A
	pBigData[0] = pInData[7];
	pBigData[1] = pInData[6];
	pBigData[2] = pInData[5];
	pBigData[3] = pInData[4];
	pBigData[4] = pInData[3];
	pBigData[5] = pInData[2];
	pBigData[6] = pInData[1];
	pBigData[7] = pInData[0];
}

#endif // #if BIG_ENDIAN_MACHINE == 1

#endif // #ifndef LES_BASE_HH
