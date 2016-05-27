#if !defined(BASE_H__INCLUDED_)
#define BASE_H__INCLUDED_


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



/*#ifndef __cplusplus
	typedef unsigned char bool;
	enum{
		false,
		true
	};
	#define int(x)		(int)(x)
	#define float(x)	(float)(x)
#endif*/

#ifdef OS_LINUX
	#define _cdecl
#endif


#include "array.h"
#include "strings.h"

#ifdef OS_WINDOWS
	#define and &&
	#define or ||
#endif

#ifndef max
	#define max(a,b)	(((a) > (b)) ? (a) : (b))
#endif
#ifndef min
	#define min(a,b)	(((a) < (b)) ? (a) : (b))
#endif

// base class for classes with virtual functions
//  -> compatibility with kaba
class VirtualBase
{
public:
	virtual ~VirtualBase(){}
	virtual void _cdecl __delete__(){}
	
#ifdef OS_WINDOWS
	void __thiscall __delete_external__(){ __delete__(); }
#else
	void __delete_external__(){ __delete__(); }
#endif
};

// implement Derived.~Derived()  AND  Derived.__delete__()
//    Derived.__delete__() should call this->Derived::~Derived()

// instances in the main program can be delete()ed

// classes derived by kaba override __delete__()
//    they also override ~() with __delete_external__()


#endif
