#if !defined(ARRAY_H__INCLUDED_)
#define ARRAY_H__INCLUDED_

#include <new>
#include <string.h>

// dynamic arrays



class DynamicArray
{
	public:
	void _cdecl init(int _element_size_);
	void _cdecl reserve(int size);
	void _cdecl resize(int size);
	void _cdecl ensure_size(int size);
	void _cdecl insert_blank(int pos);
	void _cdecl append(const DynamicArray *a);
	void _cdecl assign(const DynamicArray *a);
	void _cdecl exchange(DynamicArray &a);
	void _cdecl append_p_single(void *p);
	void _cdecl append_4_single(int x);
	void _cdecl append_f_single(float x);
	void _cdecl append_d_single(double x);
	void _cdecl append_1_single(char x);
	void _cdecl append_single(const void *d);
	void _cdecl insert_p_single(void *p, int index);
	void _cdecl insert_4_single(int x, int index);
	void _cdecl insert_f_single(float x, int index);
	void _cdecl insert_d_single(double x, int index);
	void _cdecl insert_1_single(char x, int index);
	void _cdecl insert_single(const void *d, int index);
	void _cdecl delete_single(int index);
	void _cdecl swap(int i1, int i2);
	void _cdecl move(int source, int target);
	void _cdecl reverse();
	DynamicArray _cdecl ref_subarray(int start, int num_elements);
	int _cdecl index(const void *p);
	void _cdecl clear();
	void *data;
	int num, allocated, element_size;
};

#include <stdio.h>

template <class T>
class Array : public DynamicArray
{
	public:
		Array()
		{	init(sizeof(T));	}
		Array(const Array &a)
		{
			init(sizeof(T));
			(*this) = a;
		}
		Array(const T &item)
		{
			init(sizeof(T));
			add(item);
		}
		void _cdecl __init__()
		{
			init(sizeof(T));
		}
		~Array()
		{	clear();	}
		void _cdecl clear()
		{
			if (allocated > 0){
				for (int i=0; i<num; i++)
					(*this)[i].~T();
			}
			DynamicArray::clear();
		}
		void _cdecl add(const T item)
		{
			resize(num + 1);
			(*this)[num - 1] = item;
		}
		T _cdecl pop()
		{
			T r;
			if (num > 0){
				//memcpy(&r, &back(), element_size);
				//DynamicArray::resize(num - 1);
				r = back();
				resize(num - 1);
			}
			return r;
		}
		void _cdecl append(const Array<T> &a)
		{
			int num0 = num;
			resize(num + a.num);
			for (int i=0;i<a.num;i++)
				(*this)[num0 + i] = a[i];
		}
		void _cdecl erase(int index)
		{
			(*this)[index].~T();
			delete_single(index);
		}
		void _cdecl insert(const T item, int index)
		{
			insert_blank(index);
			new(&(*this)[index]) T;
			(*this)[index] = item;
		}
		void _cdecl resize(int size)
		{
			if (size < num){
				// shrink -> destruct
				for (int i=size;i<num;i++)
					(*this)[i].~T();
			}
			if (size > num){
				reserve(size);
				// grow -> construct
				memset((char*)data + num * element_size, 0, (size - num) * element_size);
				for (int i=num; i<size; i++)
					new(&(*this)[i]) T;
			}
			num = size;
		}
		int find(const T item) const
		{
			for (int i=0; i<num; i++)
				if ((*this)[i] == item)
					return i;
			return -1;
		}
		Array<T> _cdecl sub(int start, int num_elements) const
		{
			Array<T> s;
			if ((num_elements < 0) or (num_elements > num - start))
				num_elements = num - start;
			s.num = num_elements;
			s.data = ((T*)this->data) + start;
			return s;
		}
		void operator = (const Array<T> &a)
		{
			if (this != &a){
				resize(a.num);
				for (int i=0; i<num; i++)
					(*this)[i] = a[i];
			}
		}
		void operator += (const Array<T> &a)
		{	append(a);	}
		Array<T> operator + (const Array<T> &a) const
		{
			Array<T> r = *this;
			r.append(a);
			return r;
		}
		T &operator[] (int index) const
		{	return ((T*)data)[index];	}
		T &back()
		{	return ((T*)data)[num - 1];	}
		const T &_cdecl back() const
		{	return ((T*)data)[num - 1];	}

		// reference arrays
		void _cdecl set_ref(const Array<T> &a)
		{
			if (this != &a){
				clear();
				num = a.num;
				data = a.data;
				element_size = a.element_size;
				allocated = 0;
			}
		}
		void _cdecl forget()
		{
			data = NULL;
			allocated = 0;
			num = 0;
		}
		void _cdecl make_own()
		{
			if ((num == 0) or (allocated > 0))
				return;
			T *dd = (T*)data;
			int n = num;
			forget();
			resize(n);
			for (int i=0; i<num; i++)
				(*this)[i] = dd[i];
		}

		// iterators
		class Iterator
		{
		public:
			void operator ++()
			{	index ++;	p ++;	}
			void operator ++(int) // postfix
			{	index ++;	p ++;	}
			void operator --()
			{	index --;	p --;	}
			void operator --(int)
			{	index --;	p --;	}
			bool operator == (const Iterator &i) const
			{	return p == i.p;	}
			bool operator != (const Iterator &i) const
			{	return p != i.p;	}
			T &operator *()
			{	return *p;	}
			T *operator ->()
			{	return p;	}
			bool valid() const
			{	return index < num;	}
			bool valid_down() const
			{	return index >= 0;	}
			int get_index() const
			{	return index;	}
			void update()
			{	p = &array[index];	}
			operator bool() const
		    {	return false;	}
		//private:
			Iterator(const Array<T> &a, int n) : array(a), num(a.num)
			{
				p = &array[n];
				index = n;
			}
		private:
			const Array<T> &array;
			T *p;
			int index;
			const int &num;
		};
		Iterator begin() const
		{	return Iterator(*this, 0);	}
		Iterator end() const
		{	return Iterator(*this, num);	}
		Iterator begin_down() const
		{	return Iterator(*this, num - 1);	}
		/*void erase(Iterator &it)
		{	erase(it.get_index());	}*/
};

// foreach loop
// stolen from boost...

inline bool _foreach_set_false_(bool &b)
{	b = false;	return false;	}


#define ___foreach(_var_, _array_) \
	if (auto _foreach_it_ = (_array_).begin()) {} else \
	for (bool _foreach_continue = true; \
		_foreach_continue && _foreach_it_.valid(); \
		_foreach_continue ? (_foreach_it_ ++) : (void)0) \
	if  (_foreach_set_false_(_foreach_continue)) {} else \
	for (_var_ = *_foreach_it_; !_foreach_continue; _foreach_continue = true)

#define foreachi(_var_, _array_, _i_) \
	if (auto _foreach_it_ = (_array_).begin()) {} else \
	for (bool _foreach_continue = true; \
		_foreach_continue && _foreach_it_.valid(); \
		_foreach_continue ? (_foreach_it_ ++) : (void)0) \
	if  (_foreach_set_false_(_foreach_continue)) {} else \
	for (int _i_ = _foreach_it_.get_index(); _i_ >= 0; _i_ = -1) \
	for (_var_ = *_foreach_it_; !_foreach_continue; _foreach_continue = true)



#define foreachb(_var_, _array_) \
	if (auto _foreach_it_ = (_array_).begin_down()) {} else \
	for (bool _foreach_continue = true; \
		_foreach_continue && _foreach_it_.valid_down(); \
		_foreach_continue ? (_foreach_it_ --) : (void)0) \
	if  (_foreach_set_false_(_foreach_continue)) {} else \
	for (_var_ = *_foreach_it_; !_foreach_continue; _foreach_continue = true)

#define foreachib(_var_, _array_, _i_) \
	if (auto _foreach_it_ = (_array_).begin_down()) {} else \
	for (bool _foreach_continue = true; \
		_foreach_continue && _foreach_it_.valid_down(); \
		_foreach_continue ? (_foreach_it_ --) : (void)0) \
	if  (_foreach_set_false_(_foreach_continue)) {} else \
	for (int _i_ = _foreach_it_.get_index(); _i_ >= 0; _i_ = -1) \
	for (_var_ = *_foreach_it_; !_foreach_continue; _foreach_continue = true)

#endif
