#include "mutex.h"
#include "../file/file.h"

#ifdef OS_WINDOWS
	#include <windows.h>
#endif
#ifdef OS_LINUX
	#include <pthread.h>
#endif


//------------------------------------------------------------------------------
// mutexes


struct MutexInternal
{
#ifdef OS_WINDOWS
	HANDLE mutex;
#endif
#ifdef OS_LINUX
	pthread_mutex_t mutex;
#endif
};

Mutex::Mutex()
{
	__init__();
}

Mutex::~Mutex()
{
	__delete__();
}

#ifdef OS_WINDOWS

void Mutex::__init__()
{
	internal = new MutexInternal;
	internal->mutex = CreateMutex(NULL, false, NULL);
}

void Mutex::Lock()
{
	WaitForSingleObject(internal->mutex, INFINITE);
}

void Mutex::Unlock()
{
	ReleaseMutex(internal->mutex);
}

void Mutex::__delete__()
{
	CloseHandle(&internal->mutex);
	delete(internal);
}

#endif
#ifdef OS_LINUX

void Mutex::__init__()
{
	internal = new MutexInternal;
	pthread_mutex_init(&internal->mutex, NULL);
}

void Mutex::Lock()
{
	pthread_mutex_lock(&internal->mutex);
}

void Mutex::Unlock()
{
	pthread_mutex_unlock(&internal->mutex);
}

void Mutex::__delete__()
{
	pthread_mutex_destroy(&internal->mutex);
	delete(internal);
}
#endif

