#include "../os/msg.h"
#include "Mutex.h"
#include "Thread.h"

#ifdef OS_WINDOWS
	#include <windows.h>
#endif
#if defined(OS_LINUX) || defined(OS_MAC) || defined(OS_MINGW)
	#include <pthread.h>
#endif


//------------------------------------------------------------------------------
// mutexes


struct MutexInternal
{
#ifdef OS_WINDOWS
	HANDLE mutex;
#endif
#if defined(OS_LINUX) || defined(OS_MAC) || defined(OS_MINGW)
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
	internal->mutex = CreateMutex(nullptr, false, nullptr);
}

void Mutex::lock()
{
	WaitForSingleObject(internal->mutex, INFINITE);
}

void Mutex::unlock()
{
	ReleaseMutex(internal->mutex);
}


bool Mutex::tryLock()
{
	// argh
	return true;
}

#endif
#if defined(OS_LINUX) || defined(OS_MAC) || defined(OS_MINGW)

Mutex::~Mutex()
{
	pthread_mutex_destroy(&internal->mutex);
	delete(internal);
}

void Mutex::__init__()
{
	internal = new MutexInternal;
	pthread_mutex_init(&internal->mutex, nullptr);
}

void Mutex::lock()
{
	//msg_write("lock " + p2s(Thread::getSelf()));
	pthread_mutex_lock(&internal->mutex);
	//msg_write("   ok " + p2s(Thread::getSelf()));
}

bool Mutex::tryLock()
{
	if (pthread_mutex_trylock(&internal->mutex) == 0){
		//msg_write("lock " + p2s(Thread::getSelf()));
		//msg_write("   ok " + p2s(Thread::getSelf()));
		return true;
	}
	return false;
}

void Mutex::unlock()
{
	//msg_write("unlock " + p2s(Thread::getSelf()));
	pthread_mutex_unlock(&internal->mutex);
}
#endif


void Mutex::__delete__()
{
	this->Mutex::~Mutex();
}

