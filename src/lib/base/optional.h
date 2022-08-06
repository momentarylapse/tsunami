/*
 * optional.h
 *
 *  Created on: Oct 16, 2020
 *      Author: michi
 */

#ifndef SRC_LIB_BASE_OPTIONAL_H_
#define SRC_LIB_BASE_OPTIONAL_H_

#include "base.h"

//class E {};
class Empty;
extern Empty None;

// still very experimental!
template<class T>
class optional {
public:
	optional() {
		_is_set = false;
	}
	~optional() {
		clear();
	}

	bool has_value() const {
		return _is_set;
	}

	void operator=(const T &o) {
		_init();
		value() = o;
	}
	void operator=(const optional<T> &o) {
		clear();
		if (!o.has_value())
			return;
		_init();
		value() = o.value();
	}
	void operator=(const Empty &o) {
		clear();
	}

	T &value() const {
		if (!_is_set)
			throw Exception("no value");
		return *(T*)&_value;
	}
	T &value() {
		if (!_is_set)
			throw Exception("no value");
		return *(T*)&_value;
	}
	void clear() {
		if (!_is_set)
			return;
		((T*)_value)->~T();
		_is_set = false;
	}

private:
	char _value[sizeof(T)];
	bool _is_set;

	void _init() {
		if (_is_set)
			return;
		new(_value) T();
		_is_set = true;
	}
};



#endif /* SRC_LIB_BASE_OPTIONAL_H_ */
