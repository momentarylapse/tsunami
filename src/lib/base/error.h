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
	};

	inline constexpr size_t _size_max(size_t a, size_t b) {
		return (a > b) ? a : b;
	}

	// still very experimental!
	template<class T, class E = Error>
	class expected {
	public:
		expected() {
			type = 0;
		}
		expected(const E& e) : expected() {
			_switch_type(2);
			error() = e;
		}
		expected(const T& v) : expected() {
			_switch_type(1);
			value() = v;
		}
		expected(const expected &o) : expected() {
			*this = o;
		}
		expected(expected &&o) : expected() {
			*this = std::move(o);
		}
		~expected() {
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
		void operator=(const expected &o) {
			_switch_type(o.type);
			if (type == 1)
				value() = o.value();
			else if (type == 2)
				error() = o.error();
		}
		/*void operator=(expected<T, E> &&o) {  TODO
		}*/

		bool operator==(const expected &o) const {
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
			return *(T*)&_value;
		}
		E& error() const {
			//if (type != 2)
			//	throw Exception("no error");
			return *(E*)&_value;
		}
		E& error() {
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

}

template<class T, class E>
string str(const base::expected<T, E>& e) {
	if (e.has_value())
		return str(e.value());
	return "nil";
}

#endif
