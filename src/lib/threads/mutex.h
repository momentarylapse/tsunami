/*----------------------------------------------------------------------------*\
| Threads (mutex)                                                              |
|                                                                              |
| last update: 2011.02.19 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/
#if !defined(MUTEX_H_INCLUDED)
#define MUTEX_H_INCLUDED

#include "../base/base.h"

struct MutexInternal;

class Mutex
{
public:
	Mutex();
	~Mutex();
	void _cdecl Lock();
	void _cdecl Unlock();

	void _cdecl __init__();
	void _cdecl __delete__();
private:
	MutexInternal *internal;
};


#endif
