/*
 * future.h
 *
 *  Created on: 4 Sept 2023
 *      Author: michi
 */

#ifndef SRC_LIB_BASE_FUTURE_H_
#define SRC_LIB_BASE_FUTURE_H_

#include <functional>
#include "base.h"
#include "pointer.h"
#include "xparam.h"

namespace base {

// future<X>  - an X that might get its value later (asynchronously)
// promise<X> - "remote control" to update the value
//
// Rules:
// * a promise can have multiple futures
// * don't trigger a promise multiple times!
//   use promise.reset() in between
// * setting a callback AFTER the value has been defined will directly trigger the callback
//
// Warning: callbacks will be triggered even after futures die!
//   future<X> get() { ... }
//   get().then([] { ... });


typedef std::function<void()> Callback;


template<class T>
struct future;


template<class T>
struct xcallback {
	using t = std::function<void(typename xparam<T>::t)>;
};
template<>
struct xcallback<void> {
	using t = std::function<void()>;
};

enum class PromiseState {
	Unfinished,
	Succeeded,
	Failed
};

// internal/shared data structure
template<class T>
struct _promise_core_ : public Sharable<base::Empty> {
	Array<typename xcallback<T>::t> cb_success;
	Array<Callback> cb_fail;
	PromiseState state = PromiseState::Unfinished;
	mutable T result;

	_promise_core_() = default;

	void success(typename xparam<T>::t t) {
		state = PromiseState::Succeeded;
		result = t;
		for (auto& f: cb_success)
			f(t);
	}
	void fail() {
		state = PromiseState::Failed;
		for (auto& f: cb_fail)
			f();
	}
	// clears the "value" AND (kind of) detaches all previous futures
	void reset() {
		state = PromiseState::Unfinished;
		cb_success.clear();
		cb_fail.clear();
	}
};

// used inside async function
// capture in lambdas as copy!
template<class T>
struct promise {
	using CoreType = _promise_core_<T>;
	shared<CoreType> core;

	promise() {
		core = new CoreType;
	};

	void operator() (typename xparam<T>::t t) {
		core->success(t);
	}
	void fail() {
		core->fail();
	}
	void reset() {
		core->reset();
	}
	future<T> get_future() {
		return future<T>(core);
	}
};

// returned by async function
// this is currently only a thin interface to set callbacks
template<class T>
struct future {
	using P = promise<T>;

	shared<typename P::CoreType> core;

	explicit future(shared<typename P::CoreType> c) : core(c) {
	}
	future(const future<T>& f) : core(f.core) {
	}

	future<T>& then(typename xcallback<T>::t cb) {
		core->cb_success.add(cb);
		if (core->state == PromiseState::Succeeded) {
			if constexpr (std::is_same_v<T, void>)
				cb();
			else
				cb(core->result);
		}
		return *this;
	}

	future<T>& on_fail(Callback cb) {
		core->cb_fail.add(cb);
		if (core->state == PromiseState::Failed)
			cb();
		return *this;
	}
};


template<>
struct _promise_core_<void> : public Sharable<base::Empty> {
	Array<Callback> cb_success;
	Array<Callback> cb_fail;
	PromiseState state = PromiseState::Unfinished;

	void success() {
		state = PromiseState::Succeeded;
		for (auto& f: cb_success)
			f();
	}
	void fail() {
		state = PromiseState::Failed;
		for (auto& f: cb_fail)
			f();
	}
	void reset() {
		state = PromiseState::Unfinished;
		cb_success.clear();
		cb_fail.clear();
	}
};


template<>
struct promise<void> {
	using CoreType = _promise_core_<void>;
	shared<CoreType> core;

	promise() {
		core = new CoreType;
	};

	void operator() () {
		core->success();
	}
	void fail() {
		core->fail();
	}
	void reset() {
		core->reset();
	}
	future<void> get_future() {
		return future<void>(core);
	}
};

template<class T>
future<T> success(typename xparam<T>::t t) {
	base::promise<T> promise;
	promise(t);
	return promise.get_future();
}

inline future<void> success() {
	base::promise<void> promise;
	promise();
	return promise.get_future();
}

template<class T>
future<T> failed() {
	base::promise<T> promise;
	promise.fail();
	return promise.get_future();
}

template<class T>
void await(const future<T>& f) {
	while (f.core->state == PromiseState::Unfinished) {}
}

}


#endif /* SRC_LIB_BASE_FUTURE_H_ */
