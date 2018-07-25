/*
 * pointer.h
 *
 *  Created on: 25.07.2018
 *      Author: michi
 */

#ifndef SRC_LIB_BASE_POINTER_H_
#define SRC_LIB_BASE_POINTER_H_

#include "base.h"
#include <atomic>

class ReferenceCounted : public VirtualBase
{
public:
	ReferenceCounted(){ _reference_counter = 0; }
protected:
	std::atomic<int> _reference_counter;
};

template<class T>
class Pointer
{
public:
	Pointer(){ instance = NULL; }
	Pointer(T *i){ instance = NULL; set_instance(i); }
	~Pointer(){ set_instance(NULL); }
	bool operator == (const Pointer<T> &o) const
	{	return instance == o.instance;	}
	bool operator != (const Pointer<T> &o) const
	{	return instance != o.instance;	}
	T &operator *()
	{	return *instance;	}
	T *operator ->()
	{	return instance;	}
private:
	void set_instance(T *i)
	{
		if (instance)
			instance->_reference_counter --;
		instance = i;
		if (instance)
			instance->_reference_counter --;
	}
	T* instance;
};


#endif /* SRC_LIB_BASE_POINTER_H_ */
