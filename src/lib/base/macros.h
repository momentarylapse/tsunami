/*
 * macros.h
 *
 *  Created on: 08.06.2016
 *      Author: michi
 */

#ifndef LIB_BASE_MACROS_H_
#define LIB_BASE_MACROS_H_



// which operating system?

#if defined(WIN32) || defined(WIN64)
	#define OS_WINDOWS
#elif defined(__MINGW32__) || defined(__MINGW64__)
	#define OS_MINGW
#else
	#define OS_LINUX
#endif



// which compiler?

#if defined(_MSC_VER)
	#define COMPILER_VISUAL_STUDIO
#elif defined(__GNUC__)
	#define COMPILER_GCC
#endif



// which cpu?

#if defined(__ARM_ARCH_6__) || defined(__arm__) || defined(__ARM_EABI__) || defined(_M_ARM)
	#define CPU_ARM
#elif defined(__amd64__) || defined(__x86_64__) || defined(_M_AMD64) || defined(_M_X64)
	#define CPU_AMD64
#else // _M_IX86
	#define CPU_X86
#endif


//#ifdef OS_WINDOWS
	#define _cdecl
/*#endif
#ifdef OS_LINUX
	#define _cdecl
#endif*/

typedef int int32;
typedef long long int64;

#ifdef CPU_AMD64
typedef int64 int_p;
#else
typedef int int_p;
#endif





#endif /* LIB_BASE_MACROS_H_ */
