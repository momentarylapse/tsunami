/*
 * pointer.h
 *
 *  Created on: Oct 4, 2020
 *      Author: michi
 */

#ifndef SRC_LIB_BASE_POINTER_H_
#define SRC_LIB_BASE_POINTER_H_


#include "base.h"

#define POINTER_DEBUG 0

#if POINTER_DEBUG
	void pdb(const string &s);
	void pcrash(const char *msg);
#else
	#define pdb(x)
	#define pcrash(m)
#endif


template<class T>
using xfer = T*;


template <class T>
class owned {
	T *_p = nullptr;
public:
	owned() {
		_p = nullptr;
	}
	explicit owned(T *p) {
		_p = p;
	}
	owned(owned<T> &&o) {
		set(o._p);
		o._p = nullptr;
	}
	~owned() {
		clear();
	}

	T *get() const {
		return _p;
	}
	void set(xfer<T> p) {
		if (p == _p)
			return;
		clear();
		_p = p;
	}
	void clear() {
		if (_p)
			delete _p;
		forget();
	}
	xfer<T> give() {
		T *r = _p;
		forget();
		return r;
	}



	T &operator *() {
		return *_p;
	}
	const T &operator *() const {
		return *_p;
	}
	T *operator ->() {
		return _p;
	}
	const T *operator ->() const {
		return _p;
	}
	void operator=(xfer<T> o) {
		set(o);
	}
	void operator=(owned<T> &&o) {
		set(o);
		o.forget();
	}
	void operator=(std::nullptr_t o) {
		clear();
	}
	bool operator==(const T *o) const {
		return _p == o;
	}
	bool operator!=(const T *o) const {
		return _p != o;
	}
	friend bool operator==(const T *o, const owned<T> &m) {
		return m._p == o;
	}
	friend bool operator!=(const T *o, const owned<T> &m) {
		return m._p != o;
	}
	explicit operator bool() const {
		return _p;
	}
private:
	void forget() {
		_p = nullptr;
	}
};



template<class T>
auto ownify(T *p) {
	return owned<T>(p);
}

template <class T>
class owned_array : public Array<T*> {
public:
	~owned_array() {
		clear();
	}
	void clear() {
		pdb("owned[] ");
		for (T *p: *this)
			delete p;
		Array<T*>::clear();
	}
	void resize(int size) {
		for (int i=size; i<this->num; i++)
			delete (*this)[i];
		Array<T*>::resize(size);
	}
	void erase(int index) {
		pdb("owned[] erase");
		delete (*this)[index];
		Array<T*>::erase(index);
	}
	T *extract(int index) {
		pdb("owned[] extract");
		auto p = (*this)[index];
		Array<T*>::erase(index);
		return p;
	}
	void operator=(const Array<T*> &o) {
		clear();
		Array<T*>::operator=(o);
	}
};

namespace base {
	class Empty {};
}

template <class T>
class Sharable : public T {
public:
	mutable int _pointer_ref_counter = 0;

	template<typename... Args>
	Sharable(Args... args) : T(args...) {}

	// prevent copying
	// WHY?!?!?
//	Sharable(const Sharable<T> &o) = delete;
//	void operator=(const Sharable<T> &o) = delete;

	auto _pointer_ref() const {
		_pointer_ref_counter ++;
		pdb(format("ref %s -> %d", p2s(this), _pointer_ref_counter));
		return this;
	}
	void _pointer_unref() const {
		_pointer_ref_counter --;
		pdb(format("unref %s -> %d", p2s(this), _pointer_ref_counter));
		if (_pointer_ref_counter < 0) {
			pcrash("---- ref count < O");
		}
	}
	bool _has_pointer_refs() const {
		return _pointer_ref_counter > 0;
	}
};


template<class T>
class shared {
	T *_p = nullptr;
	friend class owned<T>;
public:
	shared() {
		_p = nullptr;
	}
	shared(T *p) {
		pdb(format("+shared %s", p2s(p)));
		if (p)
			_p = (T*)p->_pointer_ref();
	}
	shared(const shared<T> &o) {
		pdb(format("+shared/s %s", p2s(o._p)));
		if (o)
			_p = (T*)o._p->_pointer_ref();
	}
	~shared() {
		pdb(format("-shared %s", p2s(_p)));
		clear();
	}

	T *get() const {
		return _p;
	}
	void set(T *p) {
		pdb(format("shared  set %s", p2s(p)));
		if (p == _p)
			return;
		if (p) {
			// keep p, even if we are a parent to p!
			auto p_next = (T*)p->_pointer_ref();
			clear();
			_p = p_next;
		} else {
			clear();
		}
	}
	void clear() {
		if (_p) {
			_p->_pointer_unref();
			if (!_p->_has_pointer_refs()) {
				pdb("  del");
				delete _p;
			}
		}
		_p = nullptr;
	}
	template<class X>
	shared<X> to() const {
		return (X*)_p;
	}


	T &operator *() {
		return *_p;
	}
	const T &operator *() const {
		return *_p;
	}
	T *operator ->() {
		return _p;
	}
	const T *operator ->() const {
		return _p;
	}
	void operator=(const shared<T> o) {
		pdb(format("shared/s = %s", p2s(o._p)));
		set(o._p);
	}
	void operator=(T *o) {
		pdb(format("shared/p = %s", p2s(o)));
		set(o);
	}
	/*bool operator==(const shared<T> o) const {
		return _p == o._p;
	}
	bool operator!=(const shared<T> o) const {
		return _p != o._p;
	}*/
	bool operator==(const T *o) const {
		return _p == o;
	}
	bool operator!=(const T *o) const {
		return _p != o;
	}
	friend bool operator==(const T *o, const shared<T> &m) {
		return m._p == o;
	}
	friend bool operator!=(const T *o, const shared<T> &m) {
		return m._p != o;
	}
	explicit operator bool() const {
		return _p;
	}
};


template <class T>
using shared_array = Array<shared<T>>;


template <class T>
const Array<T*> &weak(const shared_array<T> &a) {
	return *(const Array<T*>*)&a;
}

template <class T>
Array<T*> &weak(shared_array<T> &a) {
	return *(Array<T*>*)&a;
}

template <class T>
const Array<T*> &weak(const owned_array<T> &a) {
	return *(const Array<T*>*)&a;
}

template <class T>
Array<T*> &weak(owned_array<T> &a) {
	return *(Array<T*>*)&a;
}



#endif /* SRC_LIB_BASE_POINTER_H_ */
