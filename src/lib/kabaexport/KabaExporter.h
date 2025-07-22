#pragma once

#include <lib/base/base.h>
#if __has_include(<lib/kaba/lib/extern.h>)
#include <lib/kaba/lib/extern.h>
#include <lib/kaba/lib/lib.h>

#else

#define KABA_LINK_GROUP_BEGIN
#define KABA_LINK_GROUP_END
#define KABA_EXCEPTION_WRAPPER(x) x

template<typename T>
void* mf(T tmf) {
	union {
		T f;
		struct {
			int_p a;
			int_p b;
		};
	} pp;
	pp.a = 0;
	pp.b = 0;
	pp.f = tmf;

	// on ARM the "virtual bit" is in <b>, on x86 it is in <a>
	return (void*)(pp.a | (pp.b & 1));
}

namespace kaba {
class Exporter {
public:
	virtual ~Exporter() = default;
	virtual void declare_class_size(const string& name, int size) = 0;
	virtual void _declare_class_element(const string& name, int offset) = 0;
	virtual void link(const string& name, void* p) = 0;
	virtual void _link_virtual(const string& name, void* p, void* instance) = 0;

	template<typename T>
	void link_class_func(const string& name, T pointer) {
		link(name, mf(pointer));
	}
	template<class T>
	void declare_class_element(const string& name, T pointer) {
		_declare_class_element(name, *(int*)(void*)&pointer);
	}
	template <typename R, typename ...Args>
	void link_func(const string& name, R (*func)(Args...)) {
		link(name, (void*)func);
	}
	template<class T>
	void declare_enum(const string& name, T value) {
		_declare_class_element(name, (int)value);
	}
	template<class T>
	void link_virtual(const string& name, T pointer, void* instance) {
		_link_virtual(name, mf(pointer), instance);
	}
};

template<class T>
void generic_init(T* t) {
	new(t) T;
}

template<class T>
void generic_delete(T* t) {
	t->~T();
}

template<class T>
void generic_assign(T& a, const T& b) {
	a = b;
}
}

#endif


