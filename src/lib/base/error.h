/*
* error.h
 *
 *  Created on: Aug 18, 2025
 *      Author: michi
 */

#ifndef SRC_LIB_BASE_ERROR_H_
#define SRC_LIB_BASE_ERROR_H_

#include "base.h"

namespace base {
	struct Error {
		string msg;
		Error() = default;
		Error(const string& s) { msg = s; }
		string str() const { return msg; }
	};

	inline constexpr size_t _size_max(size_t a, size_t b) {
		return (a > b) ? a : b;
	}

	// still very experimental!
	template<class T, class E = Error>
	class result {
	public:
		result() {
			type = 0;
		}
		result(const E& e) : result() {
			_switch_type(2);
			_error() = e;
		}
		result(const T& v) : result() {
			_switch_type(1);
			value() = v;
		}
		result(const result &o) : result() {
			*this = o;
		}
		result(result &&o) : result() {
			*this = std::move(o);
		}
		~result() {
			_switch_type(0);
		}

		bool has_value() const {
			return type == 1;
		}
		bool has_error() const {
			return type == 2;
		}

		explicit operator bool() const {
			return has_value();
		}
		T &operator*() const {
			return value();
		}
		T &operator*() {
			return value();
		}
		T *operator ->() {
			return &value();
		}
		const T *operator ->() const {
			return &value();
		}

		void operator=(const T& o) {
			_switch_type(1);
			value() = o;
		}
		void operator=(const result &o) {
			_switch_type(o.type);
			if (type == 1)
				value() = o.value();
			else if (type == 2)
				_error() = o.error();
		}
		/*void operator=(result<T, E> &&o) {  TODO
		}*/

		bool operator==(const result &o) const {
			if (type != o.type)
				return false;
			if (type == 1)
				return *(T*)&_value == *(T*)&o._value;
			return true; // compare errors? ...nope
		}

		T& value() const {
			//if (type != 1)
			//	throw Exception("no value");
			return *(T*)&_value;
		}
		T& value() {
			//if (type != 1)
			//	throw Exception("no value");
			return *(T*)&_value;
		}
		T value_or(const T& alt) const {
			if (type != 1)
				return alt;
			return value();
		}
		template<class F>
		T value_or_do(F f) const {
			if (type != 1)
				f(error().msg);
			return value();
		}
		template<class R, class F>
		result<R> transform(F f) const {
			if (has_value())
				return f(value());
			return error();
		}
		const E& error() const {
			//if (type != 2)
			//	throw Exception("no error");
			return *(E*)&_value;
		}
		E& _error() {
			//if (type != 2)
			//	throw Exception("no error");
			return *(E*)&_value;
		}

	private:
		alignas(int64) char _value[_size_max(sizeof(T), sizeof(E))];
		alignas(T) uint8 type;

		void _switch_type(uint8 t) {
			if (type == t)
				return;
			if (type == 1)
				((T*)_value)->~T();
			else if (type == 2)
				((E*)_value)->~E();
			type = t;
			if (type == 1)
				new(_value) T();
			if (type == 2)
				new(_value) E();
		}
	};

	using result_void = result<int>; // TODO result<void>
	inline result_void result_success() {
		return 0;
	}
}

template<class T, class E>
string str(const base::result<T, E>& e) {
	if (e.has_value())
		return str(e.value());
	if (e.has_error())
		return "ERROR: " + str(e.error());
	return "nil";
}

#define RESULT_PROPAGATE_ERROR(VAR, EXPR, X) \
	auto X = (EXPR); \
	if (X.has_error()) \
		return X.error(); \
	auto& VAR = X.value();

#endif
