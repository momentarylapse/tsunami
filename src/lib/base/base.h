#if !defined(BASE_H__INCLUDED_)
#define BASE_H__INCLUDED_


// which operating system?

#ifdef WIN32
	#define OS_WINDOWS
#else
	#define OS_LINUX
#endif



// which developing environment?

#ifdef _MSC_VER
	#if _MSC_VER >= 1400
		#define IDE_VCS8
	#else
		#define IDE_VCS6
	#endif
#else
	#define IDE_DEVCPP
#endif
//#define IDE_KDEVELOP ...?



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
//    make sure element destructors may be called twice without causing errors

// instances in the main program can be delete()ed

// classes derived by kaba overwrite __delete__()
//    they also overwrite ~() with __delete_external__()


#endif
