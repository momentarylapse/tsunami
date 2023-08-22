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


template<class T>
struct xparam {
	using t = const T&;
};
template<>
struct xparam<int> {
	using t = int;
};
template<>
struct xparam<float> {
	using t = float;
};
template<>
struct xparam<bool> {
	using t = bool;
};

template<class T>
struct xcallback {
	using t = std::function<void(typename xparam<T>::t)>;
};
template<>
struct xcallback<void> {
	using t = std::function<void()>;
};

enum class PromiseState {
	UNFINISHED,
	SUCCEEDED,
	FAILED
};

// internal
template<class T>
struct promise : public Sharable<base::Empty> {
	typename xcallback<T>::t cb_success;
	Callback cb_fail;
	PromiseState state;
	mutable T result;

	promise() {};

	void operator() (typename xparam<T>::t t) {
		state = PromiseState::SUCCEEDED;
		result = t;
		if (cb_success)
			cb_success(t);
	}
	void fail() {
		state = PromiseState::FAILED;
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

	P& _promise;

	future(P& p) : _promise(p) {
	}
	future(future<T>& f) : _promise(f._promise) {
	}

	future<T>& on(typename xcallback<T>::t cb) {
		_promise.cb_success = cb;
		if (_promise.state == PromiseState::SUCCEEDED) {
			if constexpr (std::is_same<T, void>::value)
				cb();
			else
				cb(_promise.result);
		}
		return *this;
	}

	future<T>& on_fail(Callback cb) {
		_promise.cb_fail = cb;
		if (_promise.state == PromiseState::FAILED)
			cb();
		return *this;
	}
};


template<>
struct promise<void> : public Sharable<base::Empty> {
	Callback cb_success;
	Callback cb_fail;
	PromiseState state;

	void operator() () {
		state = PromiseState::SUCCEEDED;
		if (cb_success)
			cb_success();
	}
	void fail() {
		state = PromiseState::FAILED;
		if (cb_fail)
			cb_fail();
	}
	future<void> get_future() {
		return future<void>(*this);
	}
};


void set_idle_function(const Callback &idle_function);
int run_later(float time, const Callback &function);
int run_repeated(float time, const Callback &function);
int run_in_gui_thread(const Callback &function);
void cancel_runner(int i);


}



#endif /* SRC_LIB_HUI_CALLBACK_H_ */
