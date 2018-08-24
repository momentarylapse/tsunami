/*
 * Callback.cpp
 *
 *  Created on: 16.05.2017
 *      Author: michi
 */

#include "hui.h"
#include <mutex>

namespace hui
{

void db_out(const char *str)
{
	//printf("%s\n", str);
}


Callback _idle_function_;
Callback _error_function_;

// the Runner[] list needs protection!
static std::mutex runner_mutex;

#ifdef HUI_API_GTK
	int idle_id = -1;

	class HuiGtkRunner
	{
	public:
		HuiGtkRunner(const Callback &_func)
		{
			func = _func;
			id = -1;
		}
		Callback func;
		int id;
	};
	Array<HuiGtkRunner*> _hui_runners_;
	void _hui_runner_delete_(int id)
	{
		for (int i=_hui_runners_.num-1; i>=0; i--)
			if (_hui_runners_[i]->id == id){
				delete _hui_runners_[i];
				_hui_runners_.erase(i);
			}
	}

	gboolean GtkIdleFunction(void*)
	{
		if (_idle_function_)
			_idle_function_();
		else
			Sleep(0.010f);
		return TRUE;
	}

	gboolean GtkRunLaterFunction(gpointer data)
	{
		HuiGtkRunner *c = reinterpret_cast<HuiGtkRunner*>(data);
		db_out("<<< rl");
		if (c->func)
			c->func();
		db_out("rl >>");

		std::lock_guard<std::mutex> lock (runner_mutex);
		_hui_runner_delete_(c->id);
		db_out("..");
		return FALSE;
	}

	gboolean GtkRunRepeatedFunction(gpointer data)
	{
		HuiGtkRunner *c = reinterpret_cast<HuiGtkRunner*>(data);
		db_out("<<< rr");
		if (c->func)
			c->func();
		db_out("rr >>");
		return TRUE;
	}
#endif



void SetIdleFunction(const Callback &c)
{
#ifdef HUI_API_GTK
	bool old_idle = (bool)_idle_function_;
	bool new_idle = (bool)c;
	if (new_idle and !old_idle)
		idle_id = g_idle_add_full(300, GtkIdleFunction, nullptr, nullptr);
	if (!new_idle and old_idle and (idle_id >= 0)){
		g_source_remove(idle_id);
		idle_id = -1;
	}
#endif
	_idle_function_ = c;
}

/*void HuiSetIdleFunction(hui_callback *idle_function)
{
	_HuiSetIdleFunction(idle_function);
}

void _HuiSetIdleFunctionM(HuiEventHandler *object, void (HuiEventHandler::*function)())
{
	_HuiSetIdleFunction(HuiCallback(object, function));
}*/

int RunLater(float time, const Callback &c)
{
	db_out("rl lock 0");
	//std::lock_guard<std::mutex> lock(runner_mutex);

	#ifdef HUI_API_WIN
		msg_todo("HuiRunLater");
		return 0;
	#endif
	#ifdef HUI_API_GTK
		HuiGtkRunner *r = new HuiGtkRunner(c);
		db_out("rl lock a");
		{
			std::lock_guard<std::mutex> lock(runner_mutex);
			_hui_runners_.add(r);
		}
		db_out("rl lock b");
		r->id = g_timeout_add_full(300, max((int)(time * 1000), 1), &GtkRunLaterFunction, (void*)r, nullptr);
		db_out("rl lock z");
		return r->id;
	#endif
}

int RunRepeated(float time, const Callback &c)
{
	db_out("rr lock 0");
	//std::lock_guard<std::mutex> lock(runner_mutex);

	#ifdef HUI_API_WIN
		msg_todo("HuiRunRepeated");
		return 0;
	#endif
	#ifdef HUI_API_GTK
		HuiGtkRunner *r = new HuiGtkRunner(c);
		db_out("rr lock a");
		{
			std::lock_guard<std::mutex> lock(runner_mutex);
			_hui_runners_.add(r);
		}
		db_out("rr lock b");
		r->id = g_timeout_add_full(300, max((int)(time * 1000), 1), &GtkRunRepeatedFunction, (void*)r, nullptr);
		db_out("rr lock z");
		return r->id;
	#endif
}

void CancelRunner(int id)
{
	db_out("rl cancel a0");
	//std::lock_guard<std::mutex> lock(runner_mutex);
#ifdef HUI_API_GTK
	//msg_write("rl cancel a");
	g_source_remove(id);
	db_out("rl cancel b");
	{
		std::lock_guard<std::mutex> lock(runner_mutex);
		_hui_runner_delete_(id);
	}
	db_out("rl cancel c");
#endif
}

}
