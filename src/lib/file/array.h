#if !defined(ARRAY_H__INCLUDED_)
#define ARRAY_H__INCLUDED_

#include <new>

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
};



template <class T>
class DumbArray : public DynamicArray
{
	public:
		DumbArray()
		{	init(sizeof(T));	}
		~DumbArray()
		{	clear();	}
		void add(const T &item)
		{
//			printf("add\n");
			resize(num + 1);
			(*this)[num - 1] = item;
		}
		void erase(int index)
		{
			(*this)[index].~T();
			delete_single(index);
		}
		void resize(int size)
		{
			reserve(size);
			if (size > num)
				memset((char*)data + num * element_size, 0, (size - num) * element_size);
			for (int i=num;i<size;i++)
				(*this)[i] = T();
			num = size;
		}
		void operator = (const DumbArray &a)
		{
			num = a.num;
			data = a.data;
			element_size = a.element_size;
			allocated = a.allocated; // ???? TODO
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
		T operator[] (int index) const
		{	return ((T*)data)[index];	}
		T &operator[] (int index)
		{	return ((T*)data)[index];	}
		T &back()
		{	return ((T*)data)[num - 1];	}
};



template <class T>
class Set : public Array<T>
{
	public:
		void add(const T &item)
		{
			int i0 = 0;
			int i1 = ((DynamicArray*)this)->num;
			while(i1 > i0){
				int i = (i1 + i0) >> 1;
				if ((*this)[i] == item)
					return;
				else if ((*this)[i] > item){
					i1 = i;
				}else{
					i0 = i + 1;
				}
			}
			if (i0 < ((DynamicArray*)this)->num)
				insert(item, i0);
			else
				((Array<T>*)this)->add(item);
		}
		void join(const Set &a)
		{
			for (int i=0;i<a.num;i++)
				add(a[i]);
		}
		int find(const T &item) const
		{
			int i0 = 0;
			int i1 = ((DynamicArray*)this)->num;
			while(i1 > i0){
				int i = (i1 + i0) >> 1;
				if ((*this)[i] == item)
					return i;
				else if ((*this)[i] > item){
					i1 = i;
				}else{
					i0 = i + 1;
				}
			}
			return -1;
		}
		void erase(const T &item)
		{
			int index = find(item);
			if (index >= 0)
				((Array<T>*)this)->erase(index);
		}
		bool contains(const T &item) const
		{
			return (find(item) >= 0);
		}
};

#define foreach(_array_, _v_)           for (int _vi_ = 0; _vi_ < (_array_).num; _vi_++) \
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
                                        	for (const typeof((_array_)[0]) &_v_ = (_array_)[_vi_]; ;__extension__({break;}))


/*#define foreach(_array_, _v_)			typeof((_array_)[0]) *_v_ = (typeof((_array_)[0]) *) (_array_).data; \
										for (int _vi_ = 0; _vi_ < (_array_).num;_vi_++, _v_ = &(_array_)[_vi_])
#define foreachi(_array_, _v_, _vi_)	typeof((_array_)[0]) *_v_ = (typeof((_array_)[0]) *) (_array_).data; \
										for (int _vi_ = 0; _vi_ < (_array_).num;_vi_++, _v_ = &(_array_)[_vi_])

#define foreach2(_array_, _v_)			_v_ = (typeof((_array_)[0]) *) (_array_).data; \
										for (int _vi_ = 0; _vi_ < (_array_).num;_vi_++, _v_ = &(_array_)[_vi_])
#define foreachi2(_array_, _v_, _vi_)	_v_ = (typeof((_array_)[0]) *) (_array_).data; \
										for (int _vi_ = 0; _vi_ < (_array_).num;_vi_++, _v_ = &(_array_)[_vi_])

#define foreachb(_array_, _v_)			typeof((_array_)[0]) *_v_ = ((_array_).num > 0) ? &(_array_).back() : NULL; \
										for (int _vi_ = (_array_).num - 1;_vi_ >= 0; _vi_ --, _v_ = &(_array_)[_vi_])
#define foreachbi(_array_, _v_, _vi_)	typeof((_array_)[0]) *_v_ = ((_array_).num > 0) ? &(_array_).back() : NULL; \
										for (int _vi_ = (_array_).num - 1; _vi_ >= 0; _vi_ --, _v_ = &(_array_)[_vi_])*/


#endif
