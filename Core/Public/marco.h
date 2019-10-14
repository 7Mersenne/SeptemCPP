/*
	Copyright (c) 2013-2019 7Mersenne All Rights Reserved.

	LICENSE:	GNU General Public License V3.0

	As a special exception,  you may use this file  as part of a free software library without
	restriction.  Specifically,  if other files instantiate templates  or use macros or inline
	functions from this file, or you compile this file and link it with other files to produce
	an executable,  this file does not by itself cause the resulting executable to be covered
	by the GNU General Public License. This exception does not however invalidate any other
	reasons why the executable file might be covered by the GNU General Public License.

	Support Email:	guij@sari.ac.cn
*/

#pragma once

#include <assert.h>
#include <cstddef>

#define LINUX 1

/// build ue style container
#define UE_STYLE_CONTAINER 0

#ifndef check
#define check(x) assert(x)
#endif // !check

#ifndef CROSSPLATFORMVALUE
#define CROSSPLATFORMVALUE

#ifndef DEFAULT_RECYCLE_POOL_SIZE
#define DEFAULT_RECYCLE_POOL_SIZE 1024
#endif // !DEFAULT_RECYCLE_POOL_SIZE


namespace Septem
{
	// Unsigned base types.
	typedef unsigned char 		uint8;		// 8-bit  unsigned.
	typedef unsigned short int	uint16;		// 16-bit unsigned.
	typedef unsigned int		uint32;		// 32-bit unsigned.
	typedef unsigned long long	uint64;		// 64-bit unsigned.

	// Signed base types.
	typedef	signed char			int8;		// 8-bit  signed.
	typedef signed short int	int16;		// 16-bit signed.
	typedef signed int	 		int32;		// 32-bit signed.
	typedef signed long long	int64;		// 64-bit signed.

	typedef std::size_t SIZE_T;
#if 0
	// TODO: build failed
	// User-defined literals 
	uint64 operator"" _ui64(uint64 n)
	{
		return n;
	}
#endif
}

#endif // !CROSSPLATFORMVALUE

// Whether the CPU is AArch32/AArch64 (i.e. both 32 and 64-bit variants)
#ifndef PLATFORM_CPU_ARM_FAMILY
#if (defined(__arm__) || defined(_M_ARM) || defined(__aarch64__) || defined(_M_ARM64))
#define PLATFORM_CPU_ARM_FAMILY	1
#else
#define PLATFORM_CPU_ARM_FAMILY	0
#endif
#endif

/** Default behavior. */
#define FORCE_THREADSAFE_SHAREDPTRS PLATFORM_CPU_ARM_FAMILY
#define THREAD_SANITISE_UNSAFEPTR 0

/**
* ESPMode is used select between either 'fast' or 'thread safe' shared pointer types.
* This is only used by templates at compile time to generate one code path or another.
*/
enum class ESPMode
{
	/** Forced to be not thread-safe. */
	NotThreadSafe = 0,

	/**
		*	Fast, doesn't ever use atomic interlocks.
		*	Some code requires that all shared pointers are thread-safe.
		*	It's better to change it here, instead of replacing ESPMode::Fast to ESPMode::ThreadSafe throughout the code.
		*/
	Fast = FORCE_THREADSAFE_SHAREDPTRS ? 1 : 0,

	/** Conditionally thread-safe, never spin locks, but slower */
	ThreadSafe = 1
};
