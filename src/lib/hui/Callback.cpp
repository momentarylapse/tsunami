/*
 * Callback.cpp
 *
 *  Created on: 16.05.2017
 *      Author: michi
 */

#include "hui.h"

namespace hui
{



Callback _idle_function_;
Callback _error_function_;

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
		if (c->func)
			c->func();
		_hui_runner_delete_(c->id);
		return FALSE;
	}

	gboolean GtkRunRepeatedFunction(gpointer data)
	{
		HuiGtkRunner *c = reinterpret_cast<HuiGtkRunner*>(data);
		if (c->func)
			c->func();
		return TRUE;
	}
#endif



void SetIdleFunction(const Callback &c)
{
#ifdef HUI_API_GTK
	bool old_idle = (bool)_idle_function_;
	bool new_idle = (bool)c;
	if (new_idle and !old_idle)
		idle_id = g_idle_add_full(300, GtkIdleFunction, NULL, NULL);
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
	#ifdef HUI_API_WIN
		msg_todo("HuiRunLater");
		return 0;
	#endif
	#ifdef HUI_API_GTK
		HuiGtkRunner *r = new HuiGtkRunner(c);
		r->id = g_timeout_add_full(300, max((int)(time * 1000), 0), &GtkRunLaterFunction, (void*)r, NULL);
		_hui_runners_.add(r);
		return r->id;
	#endif
}

int RunRepeated(float time, const Callback &c)
{
	#ifdef HUI_API_WIN
		msg_todo("HuiRunRepeated");
		return 0;
	#endif
	#ifdef HUI_API_GTK
		HuiGtkRunner *r = new HuiGtkRunner(c);
		r->id = g_timeout_add_full(300, max((int)(time * 1000), 0), &GtkRunRepeatedFunction, (void*)r, NULL);
		_hui_runners_.add(r);
		return r->id;
	#endif
}

void CancelRunner(int id)
{
#ifdef HUI_API_GTK
	g_source_remove(id);
	_hui_runner_delete_(id);
#endif
}

}
