#include "base.h"

template<class T>
struct EnumeratedWrapper {
	Array<T> &array;
	
	struct Iterator {
		void operator ++()
		{	index ++;	p ++;	}
		void operator ++(int) // postfix
		{	index ++;	p ++;	}
		bool operator == (const Iterator &i) const
		{	return p == i.p;	}
		bool operator != (const Iterator &i) const
		{	return p != i.p;	}
		std::pair<int,T&> operator *()
		{	return {index, *p};	}
		//T *operator ->()
		//{	return p;	}
		Iterator(const Array<T> &a, int n) {
			p = &a[n];
			index = n;
		}
		int index;
		T *p;
	};
	Iterator begin() const {
		return Iterator(array, 0);
	}
	Iterator end() const {
		return Iterator(array, array.num);
	}
};

template<class T>
struct ConstEnumeratedWrapper {
	const Array<T> &array;
	
	struct Iterator {
		void operator ++()
		{	index ++;	p ++;	}
		void operator ++(int) // postfix
		{	index ++;	p ++;	}
		bool operator == (const Iterator &i) const
		{	return p == i.p;	}
		bool operator != (const Iterator &i) const
		{	return p != i.p;	}
		std::pair<int,const T&> operator *()
		{	return {index, *p};	}
		//T *operator ->()
		//{	return p;	}
		Iterator(const Array<T> &a, int n) {
			p = &a[n];
			index = n;
		}
		int index;
		const T *p;
	};
	Iterator begin() const {
		return Iterator(array, 0);
	}
	Iterator end() const {
		return Iterator(array, array.num);
	}
};

template<class T>
auto enumerate(Array<T> &array) -> EnumeratedWrapper<T> {
	return {array};
}

template<class T>
auto enumerate(const Array<T> &array) -> ConstEnumeratedWrapper<T> {
	return {array};
}
