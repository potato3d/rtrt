#pragma once
#ifndef _RTU_PLATFORM_H_
#define _RTU_PLATFORM_H_

#if defined(_MSC_VER)
	#pragma warning( disable : 4244 )
	#pragma warning( disable : 4251 )
	#pragma warning( disable : 4267 )
	#pragma warning( disable : 4275 )
	#pragma warning( disable : 4290 )
	#pragma warning( disable : 4786 )
	#pragma warning( disable : 4305 )
	#pragma warning( disable : 4996 )
#endif

// Define NULL pointer value.
#ifndef NULL
	#ifdef  __cplusplus
		#define NULL	0
	#else
		#define NULL	((void *)0)
	#endif
#endif

// Portable macro to 'force' inlining of a function
#if defined(_MSC_VER)
	#define RTU_FORCEINLINE	__forceinline
#elif defined(__GNUG__)
	#define RTU_FORCEINLINE	inline __attribute__((always_inline))
#else
	#error Oops! The RTU_FORCEINLINE macro was not defined for this compiler!
#endif

// Portable macro to force data cache alignment
#if defined(_MSC_VER)
	#define RTU_CACHE_ALIGN(bytes)	__declspec(align(bytes))
#elif defined(__GNUG__)
	#define RTU_CACHE_ALIGN(bytes)	__attribute__((aligned(bytes)))
#else
	#error Oops! The RTU_CACHE_ALIGN macro was not defined for this compiler!
#endif

#if defined(__GNUG__)
	#include <sys/types.h>
#endif

namespace rtu {

// Portable integer types:
#if defined(_MSC_VER)
	typedef __int8				int8;
	typedef __int16				int16;
	typedef __int32				int32;
	typedef __int64				int64;
	typedef unsigned __int8		uint8;
	typedef unsigned __int16	uint16;
	typedef unsigned __int32	uint32;
	typedef unsigned __int64	uint64;
#elif defined(__GNUG__)
	typedef int8_t				int8;
	typedef int16_t				int16;
	typedef int32_t				int32;
	typedef int64_t				int64;
	typedef u_int8_t			uint8;
	typedef u_int16_t			uint16;
	typedef u_int32_t			uint32;
	typedef u_int64_t			uint64;
#else
	#error Oops! The portable integer types were not defined for this platform!
#endif

// Portable limits for integer types:
// (current definitions are only valid for x86 platforms)
const int8		INT8_MIN	= -127 - 1;
const int8		INT8_MAX	= 127;
const uint8		UINT8_MAX	= 0xFF;

const int16		INT16_MIN	= -32767 - 1;
const int16		INT16_MAX	= 32767;
const uint16	UINT16_MAX	= 0xFFFF;

const int32		INT32_MIN	= -2147483647 - 1;
const int32		INT32_MAX	= 2147483647;
const uint32	UINT32_MAX	= 0xFFFFFFFF;

const int64		INT64_MIN	= -9223372036854775807 - 1;
const int64		INT64_MAX	= 9223372036854775807;
const uint64	UINT64_MAX	= 0xFFFFFFFFFFFFFFFF;

} // namespace rtu

#endif
