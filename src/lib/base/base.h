#if !defined(BASE_H__INCLUDED_)
#define BASE_H__INCLUDED_

#include "macros.h"
#include "array.h"
#include "strings.h"

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

template<class T>
inline T max(T a, T b) {
	if (a > b)
		return a;
	return b;
}

template<class T>
inline T min(T a, T b) {
	if (a > b)
		return b;
	return a;
}


// base class for classes with virtual functions
//  -> compatibility with kaba
class VirtualBase {
public:
	virtual ~VirtualBase(){}
	virtual void _cdecl __delete__(){}
	
#ifdef OS_WINDOWS
	void __thiscall __delete_external__(){ __delete__(); }
#else
	void __delete_external__(){ __delete__(); }
#endif
};

// implement Derived.~Derived()  AND  Derived.__delete__()
//    Derived.__delete__() should call this->Derived::~Derived()

// instances in the main program can be delete()ed

// classes derived by kaba override __delete__()
//    they also override ~() with __delete_external__()




// strings.cpp
class Exception : public VirtualBase {
public:
	Exception(){}
	explicit Exception(const string &msg){ text = msg; }
	virtual ~Exception(){}
	virtual string message() const { return text; }
	string text;
};

#endif
