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
#else
	#define OS_LINUX
#endif



// which compiler?

#ifdef _MSC_VER
	#if _MSC_VER >= 1400
		#define COMPILER_VCS8
	#else
		#define COMPULER_VCS6
	#endif
#else // __GNUC__
	#define COMPILER_GCC
#endif



// which cpu?

#ifdef OS_WINDOWS
	#if defined(_M_AMD64) || defined(_M_X64)
		#define CPU_AMD64
	#elif defined(_M_ARM)
		#define CPU_ARM
	#else // _M_IX86
		#define CPU_X86
	#endif
#endif
#ifdef OS_LINUX
	#if defined(__ARM_ARCH_6__) || defined(__arm__) || defined(__ARM_EABI__)
		#define CPU_ARM
	#elif defined(__amd64__) || defined(__x86_64__)
		#define CPU_AMD64
	#else
		#define CPU_X86
	#endif
#endif


#ifdef OS_LINUX
	#define _cdecl
#endif




#ifdef OS_WINDOWS
	#define and &&
	#define or ||
#endif



#endif /* LIB_BASE_MACROS_H_ */
