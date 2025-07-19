#pragma once

#include <lib/base/base.h>
#if __has_include(<lib/kaba/lib/extern.h>)
#include <lib/kaba/lib/extern.h>

#else

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


