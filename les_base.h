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

#define BIG_ENDIAN_MACHINE (1)

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

#if BIG_ENDIAN_MACHINE

// Swap LittleEndian to BigEndian
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

// Swap BigEndian to LittleEndian
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

#else

// swap LittleEndian to LittleEndian
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

// swap LittleEndian to LittleEndian
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

#endif // #if BIG_ENDIAN_MACHINE

#endif // #ifndef LES_BASE_HH
