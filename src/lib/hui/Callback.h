/*
 * Callback.h
 *
 *  Created on: 15.05.2017
 *      Author: michi
 */

#ifndef SRC_LIB_HUI_CALLBACK_H_
#define SRC_LIB_HUI_CALLBACK_H_

#include <functional>
#include "../base/pointer.h"

class Painter;

namespace hui
{

class EventHandler;

typedef void kaba_callback();
typedef void kaba_member_callback(EventHandler *h);
typedef void kaba_member_callback_p(EventHandler *h, void *p);

typedef std::function<void()> Callback;
typedef std::function<void(Painter*)> CallbackP;

template<class T>
struct future;

// internal
template<class T>
struct promise : public Sharable<base::Empty> {
	using Callback = std::function<void(T)>;
	using CallbackFail = std::function<void()>;

	Callback cb_success;
	CallbackFail cb_fail;

	void set_value(T t) {
		if (cb_success)
			cb_success(t);
	}
	void fail() {
		if (cb_fail)
			cb_fail();
	}
	future<T> get_future() {
		return future<T>(*this);
	}
};

// returned by async function
// this is currently only a thin interface to set callbacks
template<class T>
struct future {
	using P = promise<T>;
	using Callback = std::function<void(T)>;
	using CallbackFail = std::function<void()>;

	P& _promise;

	future(P& p) : _promise(p) {
	}
	future(future<T>& f) : _promise(f._promise) {
	}

	future<T>& on(Callback cb) {
		_promise.cb_success = cb;
		return *this;
	}

	future<T>& on_fail(CallbackFail cb) {
		_promise.cb_fail = cb;
		return *this;
	}
};


void set_idle_function(const Callback &idle_function);
int run_later(float time, const Callback &function);
int run_repeated(float time, const Callback &function);
int run_in_gui_thread(const Callback &function);
void cancel_runner(int i);


}



#endif /* SRC_LIB_HUI_CALLBACK_H_ */
