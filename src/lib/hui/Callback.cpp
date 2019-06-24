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



Callback _idle_function_;
Callback _error_function_;

// the Runner[] list needs protection!
static std::mutex runner_mutex;

#ifdef HUI_API_GTK
	int idle_id = -1;
	static int _next_runner_id_ = 0;

	class HuiGtkRunner
	{
	public:
		HuiGtkRunner(const Callback &_func, float _dt)
		{
			func = _func;
			gtk_id = -1;
			hui_id = _next_runner_id_ ++;
			dt = _dt;
			delete_me = false;
		}
		Callback func;
		float dt;
		int gtk_id;
		int hui_id;
		bool delete_me;
	};
	Array<HuiGtkRunner*> _hui_runners_;
	void _hui_runners_cleanup_()
	{
		for (int i=_hui_runners_.num-1; i>=0; i--)
			if (_hui_runners_[i]->delete_me){
				auto *c = _hui_runners_[i];
				//printf("del %d\n", c->hui_id);
				_hui_runners_.erase(i);
				delete c;
			}

	}
	HuiGtkRunner *_hui_get_runner_(int id)
	{
		for (auto *r: _hui_runners_)
			if (r->hui_id == id)
				return r;
		return nullptr;
	}
	/*void _hui_runner_delete_(int id)
	{
		for (int i=_hui_runners_.num-1; i>=0; i--)
			if (_hui_runners_[i]->hui_id == id){
				auto *c = _hui_runners_[i];
				_hui_runners_.erase(i);
				delete c;
			}
		Array<int> ids;
		for (auto *c: _hui_runners_)
			ids.add(c->hui_id);
		msg_write(ia2s(ids));
	}*/

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
		//printf("<<< rl  %d\n", c->hui_id);
		if (c->func)
			c->func();
		//printf("rl >>\n");

		c->gtk_id = -1;
		c->delete_me;
		return FALSE;
	}

	gboolean GtkRunRepeatedFunction(gpointer data)
	{
		HuiGtkRunner *c = reinterpret_cast<HuiGtkRunner*>(data);
		//printf("<<< rr %d\n", c->hui_id);
		if (c->func)
			c->func();
		//printf("rr >>\n");
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
	//msg_write("rl lock 0");
	//std::lock_guard<std::mutex> lock(runner_mutex);

	#ifdef HUI_API_WIN
		msg_todo("HuiRunLater");
		return 0;
	#endif
	#ifdef HUI_API_GTK
		HuiGtkRunner *r = new HuiGtkRunner(c, time);
		{
			std::lock_guard<std::mutex> lock(runner_mutex);
			//msg_write("rl lock 0");
			_hui_runners_.add(r);
			_hui_runners_cleanup_();
			//msg_write("rl lock z");
		}
		//msg_write("rl lock b");
		r->gtk_id = g_timeout_add_full(300, max((int)(time * 1000), 1), &GtkRunLaterFunction, (void*)r, nullptr);
		//msg_write("rl lock z");
		//msg_write(r->hui_id);
		return r->hui_id;
	#endif
}

int RunRepeated(float time, const Callback &c)
{
	//msg_write("rr lock 0");
	//std::lock_guard<std::mutex> lock(runner_mutex);

	#ifdef HUI_API_WIN
		msg_todo("HuiRunRepeated");
		return 0;
	#endif
	#ifdef HUI_API_GTK
		HuiGtkRunner *r = new HuiGtkRunner(c, time);
		{
			std::lock_guard<std::mutex> lock(runner_mutex);
			//msg_write("rr lock a");
			_hui_runners_.add(r);
			_hui_runners_cleanup_();
		}
		//msg_write("rr lock b");
		r->gtk_id = g_timeout_add_full(300, max((int)(time * 1000), 1), &GtkRunRepeatedFunction, (void*)r, nullptr);
		//msg_write("rr lock z");
		return r->hui_id;
	#endif
}

void CancelRunner(int id)
{
	//msg_write("cancel " + i2s(id));
	std::lock_guard<std::mutex> lock(runner_mutex);
#ifdef HUI_API_GTK
	//msg_write("rl cancel a");
	auto *r = _hui_get_runner_(id);
	if (r){
		if (r->gtk_id >= 0)
			g_source_remove(r->gtk_id);
	//msg_write("rl cancel b");
		r->delete_me = true;
		_hui_runners_cleanup_();
	}
	//msg_write("rl cancel c");
#endif
}

}
