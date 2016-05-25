#include "../file/file.h"
#include "Mutex.h"

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

#ifdef OS_WINDOWS

Mutex::~Mutex()
{
	CloseHandle(&internal->mutex);
	delete(internal);
}

void Mutex::__init__()
{
	internal = new MutexInternal;
	internal->mutex = CreateMutex(NULL, false, NULL);
}

void Mutex::lock()
{
	WaitForSingleObject(internal->mutex, INFINITE);
}

void Mutex::unlock()
{
	ReleaseMutex(internal->mutex);
}

#endif
#ifdef OS_LINUX

Mutex::~Mutex()
{
	pthread_mutex_destroy(&internal->mutex);
	delete(internal);
}

void Mutex::__init__()
{
	internal = new MutexInternal;
	pthread_mutex_init(&internal->mutex, NULL);
}

void Mutex::lock()
{
	pthread_mutex_lock(&internal->mutex);
}

void Mutex::unlock()
{
	pthread_mutex_unlock(&internal->mutex);
}
#endif


void Mutex::__delete__()
{
	this->Mutex::~Mutex();
}

