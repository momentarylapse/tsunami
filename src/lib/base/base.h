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


#ifndef max
	#define max(a,b)	(((a) > (b)) ? (a) : (b))
#endif
#ifndef min
	#define min(a,b)	(((a) < (b)) ? (a) : (b))
#endif

#endif
