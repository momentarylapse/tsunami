/*
 * future.h
 *
 *  Created on: 4 Sept 2023
 *      Author: michi
 */

#ifndef SRC_LIB_BASE_FUTURE_H_
#define SRC_LIB_BASE_FUTURE_H_

#include <functional>
#include "pointer.h"

namespace base {


typedef std::function<void()> Callback;


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
template<>
struct xparam<void> {
	using t = void;
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

// internal/shared data structure
template<class T>
struct _promise_core_ : public Sharable<base::Empty> {
	typename xcallback<T>::t cb_success;
	Callback cb_fail;
	PromiseState state = PromiseState::UNFINISHED;
	mutable T result;

	_promise_core_() {};

	void success(typename xparam<T>::t t) {
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
	void reset() {
		state = PromiseState::UNFINISHED;
		cb_success = nullptr;
		cb_fail = nullptr;
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

	future(shared<typename P::CoreType> c) : core(c) {
	}
	future(const future<T>& f) : core(f.core) {
	}

	future<T>& then(typename xcallback<T>::t cb) {
		core->cb_success = cb;
		if (core->state == PromiseState::SUCCEEDED) {
			if constexpr (std::is_same<T, void>::value)
				cb();
			else
				cb(core->result);
		}
		return *this;
	}

	future<T>& on_fail(Callback cb) {
		core->cb_fail = cb;
		if (core->state == PromiseState::FAILED)
			cb();
		return *this;
	}
};


template<>
struct _promise_core_<void> : public Sharable<base::Empty> {
	Callback cb_success;
	Callback cb_fail;
	PromiseState state = PromiseState::UNFINISHED;

	void success() {
		state = PromiseState::SUCCEEDED;
		if (cb_success)
			cb_success();
	}
	void fail() {
		state = PromiseState::FAILED;
		if (cb_fail)
			cb_fail();
	}
	void reset() {
		state = PromiseState::UNFINISHED;
		cb_success = nullptr;
		cb_fail = nullptr;
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
inline future<T> success(typename xparam<T>::t t) {
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
inline future<T> failed() {
	base::promise<T> promise;
	promise.fail();
	return promise.get_future();
}

template<class T>
inline void await(const future<T>& f) {
	while (f.core->state == PromiseState::UNFINISHED) {}
}

}


#endif /* SRC_LIB_BASE_FUTURE_H_ */
