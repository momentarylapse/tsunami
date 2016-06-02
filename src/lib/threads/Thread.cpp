#include "Thread.h"

#include "../file/file.h"

#ifdef OS_WINDOWS
	#include <windows.h>
#endif
#ifdef OS_LINUX
	#include <pthread.h>
	#include <unistd.h>
#endif

struct ThreadInternal
{
#ifdef OS_WINDOWS
	HANDLE thread;
#endif
#ifdef OS_LINUX
	pthread_t thread;
#endif
};


static Array<Thread*> _Thread_List_;


//------------------------------------------------------------------------------
// auxiliary

int Thread::getNumCores()
{
#ifdef OS_WINDOWS
	SYSTEM_INFO siSysInfo;
	GetSystemInfo(&siSysInfo);
	return siSysInfo.dwNumberOfProcessors;
#endif
#ifdef OS_LINUX
	return ::sysconf(_SC_NPROCESSORS_ONLN);
#endif
	return 1;
}




//------------------------------------------------------------------------------
// low level

Thread::Thread()
{
	internal = NULL;
	running = false;
	_Thread_List_.add(this);
}

Thread::~Thread()
{
	__delete__();
}


void Thread::__init__()
{
	new(this) Thread;
}



void Thread::__delete__()
{
	kill();
	for (int i=0;i<_Thread_List_.num;i++)
		if (_Thread_List_[i] == this)
			_Thread_List_.erase(i);
	if (internal)
		delete(internal);
}

#ifdef OS_WINDOWS



static DWORD WINAPI thread_start_func(__in LPVOID p)
{
	Thread *t = (Thread*)p;
	t->onRun();
	t->running = false;
	return 0;
}


// create and run a new thread
void Thread::run()
{
	if (!internal)
		internal = new ThreadInternal;
	running = true;
	internal->thread = CreateThread(NULL, 0, &thread_start_func, (void*)this, 0, NULL);

	if (!internal->thread)
		running = false;
}


void Thread::kill()
{
	if (running)
		TerminateThread(internal->thread, 0);
	running = false;
}

void Thread::join()
{
	if (running)
		WaitForSingleObject(internal->thread, INFINITE);
	running = false;
}

void Thread::exit()
{
	ExitThread(0);
}

Thread *Thread::getSelf()
{
	HANDLE h = GetCurrentThread();
	foreach(Thread *t, _Thread_List_)
		if (h == t->internal->thread)
			return t;
	return NULL;
}


#endif
#ifdef OS_LINUX


static void *thread_start_func(void *p)
{
	Thread *t = (Thread*)p;
	t->onRun();
	t->running = false;
	return NULL;
}


// create and run a new thread
void Thread::run()
{
	if (!internal)
		internal = new ThreadInternal;
	running = true;
	int ret = pthread_create(&internal->thread, NULL, &thread_start_func, (void*)this);

	if (ret != 0)
		running = false;
}


void Thread::kill()
{
	if (running){
		pthread_cancel(internal->thread);
		pthread_join(internal->thread, NULL);
	}
	running = false;
}

void Thread::join()
{
	if (running)
		pthread_join(internal->thread, NULL);
	running = false;
}

void Thread::exit()
{
	pthread_exit(NULL);
}

Thread *Thread::getSelf()
{
	pthread_t s = pthread_self();
	for (Thread *t : _Thread_List_)
		if (t->internal->thread == s)
			return t;
	return NULL;
}


#endif


bool Thread::isDone()
{
	return !running;
}

