#pragma once

#include "../base/base.h"

#if __has_include("../kaba/lib/extern.h")
#define KABA_EXCEPTION_WRAPPER(CODE) \
try{ \
	CODE; \
}catch(::Exception &e){ \
	kaba::kaba_raise_exception(kaba::create_kaba_exception(e.message())); \
}
#else
#define KABA_EXCEPTION_WRAPPER(CODE) CODE
#endif



#if defined(COMPILER_GCC)
#define KABA_LINK_GROUP_BEGIN _Pragma("GCC push_options") \
_Pragma("GCC optimize(\"no-omit-frame-pointer\")") \
_Pragma("GCC optimize(\"no-inline\")") \
_Pragma("GCC optimize(\"0\")")
#elif defined(COMPILER_CLANG)
#define KABA_LINK_GROUP_BEGIN _Pragma("clang attribute push (__attribute((noinline)), apply_to = function)")
#else
#define KABA_LINK_GROUP_BEGIN
#endif


#if defined(COMPILER_GCC)
#define KABA_LINK_GROUP_END _Pragma("GCC pop_options")
#elif defined(COMPILER_CLANG)
#define KABA_LINK_GROUP_END _Pragma("clang attribute pop")
#else
#define KABA_LINK_GROUP_END
#endif


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

class KabaException;

KabaException* create_kaba_exception(const string& message);
void kaba_raise_exception(KabaException* e);

class IExporter {
public:
	virtual ~IExporter() = default;
	virtual void package_info(const string& name, const string& version) = 0;
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

template<class T>
class generic_virtual : public T {
public:
	void __delete__() {
		this->~T();
	}
};

template<class T, class... Args>
void generic_init_ext(T* me, Args... args) {
	new(me) T(args...);
}
}

//#endif


