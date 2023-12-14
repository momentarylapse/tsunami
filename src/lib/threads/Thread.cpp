#include "Thread.h"

#include "../os/msg.h"


#ifdef OS_WINDOWS
	#include <windows.h>
#endif
#if defined(OS_LINUX) || defined(OS_MAC) || defined(OS_MINGW)
	#include <pthread.h>
	#include <unistd.h>
#endif

struct ThreadInternal
{
#ifdef OS_WINDOWS
	HANDLE thread;
	ThreadInternal()
	{	thread = nullptr;	}
#endif
#if defined(OS_LINUX) || defined(OS_MAC) || defined(OS_MINGW)
	pthread_t thread;
	ThreadInternal()
	{
		thread = 0;
	}
#endif
};


static Array<Thread*> _Thread_List_;


//------------------------------------------------------------------------------
// auxiliary

int Thread::get_num_cores()
{
#ifdef OS_WINDOWS
	SYSTEM_INFO siSysInfo;
	GetSystemInfo(&siSysInfo);
	return siSysInfo.dwNumberOfProcessors;
#endif
#if defined(OS_LINUX) || defined(OS_MAC)
	return ::sysconf(_SC_NPROCESSORS_ONLN);
#endif
	return 1;
}




//------------------------------------------------------------------------------
// low level

Thread::Thread()
{
	internal = nullptr;
	running = false;
	done = false;
	_Thread_List_.add(this);
}

Thread::~Thread()
{
	kill();
	for (int i=0;i<_Thread_List_.num;i++)
		if (_Thread_List_[i] == this)
			_Thread_List_.erase(i);
	if (internal)
		delete(internal);

	//__delete__();
}


void Thread::__init__()
{
	new(this) Thread;
}



void Thread::__delete__()
{
	this->~Thread();
}

#ifdef OS_WINDOWS



//static DWORD WINAPI thread_start_func(__in LPVOID p)
static DWORD WINAPI thread_start_func(LPVOID p)
{
	Thread *t = (Thread*)p;
	t->on_run();
	t->running = false;
	return 0;
}


// create and run a new thread
void Thread::run()
{
	if (!internal)
		internal = new ThreadInternal;
	running = true;
	internal->thread = CreateThread(nullptr, 0, &thread_start_func, (void*)this, 0, nullptr);

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

void Thread::cancelation_point()
{
	// ARGH
}

Thread *Thread::get_self()
{
	HANDLE h = GetCurrentThread();
	for (Thread *t : _Thread_List_)
		if (h == t->internal->thread)
			return t;
	return nullptr;
}


#else //OS_LINUX/MAC/MINGW

[[maybe_unused]] static void __thread_cleanup_func(void *p)
{
	Thread *t = (Thread*)p;
	t->on_cancel();
}

static void *__thread_start_func(void *p)
{
	Thread *t = (Thread*)p;

	//pthread_cleanup_push(&__thread_cleanup_func, p);

	t->done = false;
	t->on_run();

	//std::lock_guard<std::mutex> lock(t->control_mutex);
	t->done = true;

    //pthread_cleanup_pop(0);
	return nullptr;
}


// create and run a new thread
void Thread::run()
{
	if (internal){
		msg_error("multiple Thread.run()");
	}

	if (!internal)
		internal = new ThreadInternal;
	running = true;
	int ret = pthread_create(&internal->thread, nullptr, &__thread_start_func, (void*)this);

	if (ret != 0)
		running = false;
}


void Thread::kill()
{
	if (running){
		pthread_cancel(internal->thread);
		pthread_join(internal->thread, nullptr);
		//pthread_detach(internal->thread);
	}
	running = false;
}

void Thread::join()
{
	if (running)
		pthread_join(internal->thread, nullptr);
	running = false;
}

void Thread::exit()
{
	pthread_exit(nullptr);
}

Thread *Thread::get_self()
{
	pthread_t s = pthread_self();
	for (Thread *t : _Thread_List_)
		if (t->internal)
			if (t->internal->thread == s)
				return t;
	return nullptr;
}

void Thread::cancelation_point()
{
	pthread_testcancel();
}


#endif


bool Thread::is_done()
{
	//std::lock_guard<std::mutex> lock(control_mutex);
	return done;
}

