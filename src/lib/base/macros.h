/*
 * macros.h
 *
 *  Created on: 08.06.2016
 *      Author: michi
 */

#ifndef LIB_BASE_MACROS_H_
#define LIB_BASE_MACROS_H_



// which operating system?

#if defined(_WIN32) || defined(_WIN64)
	#define OS_WINDOWS
#elif defined(__MINGW32__) || defined(__MINGW64__)
	#define OS_MINGW
#elif defined(__APPLE__)
	#define OS_MAC
#elif defined(__linux__)
	#define OS_LINUX
#else
	#define OS_UNKNOWN
	#warning "failed to determine your operating system"
#endif



// which compiler?

#if defined(_MSC_VER)
#define COMPILER_VISUAL_STUDIO
#elif defined(__clang__)
#define COMPILER_CLANG
#elif defined(__GNUC__)
	#define COMPILER_GCC
#endif



// which cpu?

#if defined(__aarch64__) || defined(_M_ARM64)
	#define CPU_ARM64
#elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_7__) || defined(__arm__) || defined(__ARM_EABI__) || defined(_M_ARM)
	#define CPU_ARM32
#elif defined(__amd64__) || defined(__x86_64__) || defined(_M_AMD64) || defined(_M_X64)
	#define CPU_AMD64
#elif defined(_M_IX86)
	#define CPU_X86
#elif defined(__powerpc) || defined(__powerpc__) || defined(__POWERPC__) || defined(__ppc__) || defined(__PPC__) || defined(_ARCH_PPC)
	#define CPU_POWERPC
#elif defined(__PPC64__) || defined(__ppc64__) || defined(_ARCH_PPC64) || defined(__powerpc64__)
	#define CPU_POWERPC64
#else
	#define CPU_UNKNOWN
	#warning "CPU architecture unknown"
#endif


//#ifdef OS_WINDOWS
	#define _cdecl
/*#endif
#ifdef OS_LINUX
	#define _cdecl
#endif*/

using int32 = int;
using int64 = long long;
using int8 = signed char;
using uint8 = unsigned char;

#if defined(CPU_AMD64) || defined(CPU_ARM64)
using int_p = int64;
#else
using int_p = int32;
#endif

// did cpu detection work correctly?
static_assert(sizeof(int32) == 4);
static_assert(sizeof(int64) == 8);
static_assert(sizeof(int_p) == sizeof(void*));




#endif /* LIB_BASE_MACROS_H_ */
