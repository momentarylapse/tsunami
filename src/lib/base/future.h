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

namespace base {


typedef std::function<void()> Callback;


template<class T>
struct future;


// automatic call-by-value/reference type
// default: call-by-reference
template<class T>
struct xparam {
	using t = const T&;
};
// explicit call-by-value
template<>
struct xparam<int> {
	using t = int;
};
template<>
struct xparam<int64> {
	using t = int64;
};
template<>
struct xparam<int8> {
	using t = int8;
};
template<>
struct xparam<uint8> {
	using t = uint8;
};
template<>
struct xparam<float> {
	using t = float;
};
template<>
struct xparam<double> {
	using t = double;
};
template<>
struct xparam<bool> {
	using t = bool;
};
template<>
struct xparam<char> {
	using t = char;
};
template<class T>
struct xparam<T*> {
	using t = T*;
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
	Unfinished,
	Succeeded,
	Failed
};

// internal/shared data structure
template<class T>
struct _promise_core_ : public Sharable<base::Empty> {
	typename xcallback<T>::t cb_success;
	Callback cb_fail;
	PromiseState state = PromiseState::Unfinished;
	mutable T result;

	_promise_core_() = default;

	void success(typename xparam<T>::t t) {
		state = PromiseState::Succeeded;
		result = t;
		if (cb_success)
			cb_success(t);
	}
	void fail() {
		state = PromiseState::Failed;
		if (cb_fail)
			cb_fail();
	}
	void reset() {
		state = PromiseState::Unfinished;
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

	explicit future(shared<typename P::CoreType> c) : core(c) {
	}
	future(const future<T>& f) : core(f.core) {
	}

	future<T>& then(typename xcallback<T>::t cb) {
		core->cb_success = cb;
		if (core->state == PromiseState::Succeeded) {
			if constexpr (std::is_same_v<T, void>)
				cb();
			else
				cb(core->result);
		}
		return *this;
	}

	future<T>& on_fail(Callback cb) {
		core->cb_fail = cb;
		if (core->state == PromiseState::Failed)
			cb();
		return *this;
	}
};


template<>
struct _promise_core_<void> : public Sharable<base::Empty> {
	Callback cb_success;
	Callback cb_fail;
	PromiseState state = PromiseState::Unfinished;

	void success() {
		state = PromiseState::Succeeded;
		if (cb_success)
			cb_success();
	}
	void fail() {
		state = PromiseState::Failed;
		if (cb_fail)
			cb_fail();
	}
	void reset() {
		state = PromiseState::Unfinished;
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
