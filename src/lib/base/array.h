#if !defined(ARRAY_H__INCLUDED_)
#define ARRAY_H__INCLUDED_

#include <new>
#include <string.h>

//--------------------------------------------------------------
// michi-array



class DynamicArray
{
	public:
	void init(int _element_size_);
	void reserve(int size);
	void resize(int size);
	void ensure_size(int size);
	void insert_blank(int pos);
	void append(const DynamicArray *a);
	void assign(const DynamicArray *a);
	void exchange(DynamicArray &a);
	void append_8_single(int x, int y);
	void append_4_single(int x);
	void append_1_single(char x);
	void append_single(const void *d);
	void insert_4_single(int x, int index);
	void insert_1_single(char x, int index);
	void insert_single(const void *d, int index);
	void delete_single(int index);
	void delete_single_by_pointer(const void *p);
	void swap(int i1, int i2);
	void reverse();
	DynamicArray ref_subarray(int start, int num_elements);
	bool iterate(void *&p);
	bool iterate_back(void *&p);
	int index(const void *p);
	void clear();
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
		void __init__()
		{
			init(sizeof(T));
		}
		~Array()
		{	clear();	}
		void clear()
		{
			if (allocated > 0){
				for (int i=0;i<num;i++)
					(*this)[i].~T();
			}
			((DynamicArray*)this)->clear();
		}
		void add(const T item)
		{
			resize(num + 1);
			(*this)[num - 1] = item;
		}
		T pop()
		{
			T r;
			if (num > 0){
				memcpy(&r, &back(), element_size);
				((DynamicArray*)this)->resize(num - 1);
			}
			return r;
		}
		void append(const Array<T> &a)
		{
			int num0 = num;
			resize(num + a.num);
			for (int i=0;i<a.num;i++)
				(*this)[num0 + i] = a[i];
		}
		void erase(int index)
		{
			(*this)[index].~T();
			delete_single(index);
		}
		void insert(const T item, int index)
		{
			insert_blank(index);
			new(&(*this)[index]) T;
			(*this)[index] = item;
		}
		void resize(int size)
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
				for (int i=num;i<size;i++)
					new(&(*this)[i]) T;
			}
			num = size;
		}
		Array<T> sub(int start, int num_elements) const
		{
			Array<T> s;
			if ((num_elements < 0) || (num_elements > num - start))
				num_elements = num - start;
			s.num = num_elements;
			s.data = ((T*)this->data) + start;
			return s;
		}
		void operator = (const Array &a)
		{
			if (this != &a){
				//clear();
				//init(sizeof(T));
				resize(a.num);
				for (int i=0;i<num;i++)
					(*this)[i] = a[i];
			}
		}
		void operator = (const T &item)
		{
			clear();
			add(item);
		}
		void set_ref(const Array<T> &a)
		{
			if (this != &a){
				clear();
				num = a.num;
				data = a.data;
				element_size = a.element_size;
				allocated = 0;
			}
		}
		void forget()
		{
			data = NULL;
			allocated = 0;
			num = 0;
		}
		void make_own()
		{
			if ((num == 0) || (allocated > 0))
				return;
			T *dd = (T*)data;
			int n = num;
			forget();
			resize(n);
			for (int i=0;i<num;i++)
				(*this)[i] = dd[i];
		}
		void operator += (const Array<T> &a)
		{	append(a);	}
		void operator += (const T &item)
		{	add(item);	}
		Array<T> operator + (const Array<T> &a) const
		{
			Array<T> r = *this;
			r.append(a);
			return r;
		}
		/*Array<T> &operator + (const T &item)
		{
			Array<T> a = *this;
			a += item;
			return a;
		}*/
		T operator[] (int index) const
		{	return ((T*)data)[index];	}
		T &operator[] (int index)
		{	return ((T*)data)[index];	}
		T &back()
		{	return ((T*)data)[num - 1];	}
		const T &back() const
		{	return ((T*)data)[num - 1];	}

		// iterators
		class Iterator
		{
		public:
			void operator ++()
			{	index ++;	p ++;	}
			void operator ++(int)
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
		//private:
			Iterator(Array<T> &a, int n) : array(a), num(a.num)
			{	p = &a[n];	index = n;	}
		private:
			Array<T> &array;
			T *p;
			int index;
			int &num;
		};
		Iterator begin()
		{	return Iterator(*this, 0);	}
		Iterator begin_down()
		{	return Iterator(*this, num - 1);	}
		/*void erase(Iterator &it)
		{	erase(it.get_index());	}*/
};


#define foreach(_array_, _it_) \
	for(typeof((_array_).begin()) _it_ = (_array_).begin(); _it_.valid(); _it_ ++)

#define foreachi(_array_, _it_, _i_) \
	for(typeof((_array_).begin()) _it_ = (_array_).begin(); _it_.valid(); _it_ ++) \
		for (int _i_ = _it_.get_index(); _i_ >= 0; _i_ = -1)

#define foreachb(_array_, _it_) \
	for(typeof((_array_).begin()) _it_ = (_array_).begin_down(); _it_.valid_down(); _it_ --)

#define foreachbi(_array_, _it_, _i_) \
	for(typeof((_array_).begin()) _it_ = (_array_).begin_down(); _it_.valid_down(); _it_ --) \
		for (int _i_ = _it_.get_index(); _i_ >= 0; _i_ = -1)


/*#define foreach(_array_, _v_)           for (int _vi_ = 0; _vi_ < (_array_).num; _vi_++) \
                                        	for (typeof((_array_)[0]) &_v_ = (_array_)[_vi_]; ;__extension__({break;}))
#define foreachi(_array_, _v_, _vi_)    for (int _vi_ = 0; _vi_ < (_array_).num; _vi_++) \
                                        	for (typeof((_array_)[0]) &_v_ = (_array_)[_vi_]; ;__extension__({break;}))
#define foreachb(_array_, _v_)          for (int _vi_ = (_array_).num - 1; _vi_ >= 0; _vi_--) \
                                        	for (typeof((_array_)[0]) &_v_ = (_array_)[_vi_]; ;__extension__({break;}))
#define foreachbi(_array_, _v_, _vi_)   for (int _vi_ = (_array_).num - 1; _vi_ >= 0; _vi_--) \
                                        	for (typeof((_array_)[0]) &_v_ = (_array_)[_vi_]; ;__extension__({break;}))
#define foreachc(_array_, _v_)          for (int _vi_ = 0; _vi_ < (_array_).num; _vi_++) \
                                        	for (const typeof((_array_)[0]) &_v_ = (_array_)[_vi_]; ;__extension__({break;}))
#define foreachci(_array_, _v_, _vi_)   for (int _vi_ = 0; _vi_ < (_array_).num; _vi_++) \
                                        	for (const typeof((_array_)[0]) &_v_ = (_array_)[_vi_]; ;__extension__({break;}))*/


#endif
