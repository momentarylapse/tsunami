#if !defined(ARRAY_H__INCLUDED_)
#define ARRAY_H__INCLUDED_

#include <new>
#include <string.h>
#include <initializer_list>
#include <stdlib.h>
#include <functional>
#include <ciso646>

// dynamic arrays



class DynamicArray {
public:
	static const int MAGIC_END_INDEX;
	void _cdecl init(int _element_size_);
	void _cdecl simple_reserve(int size);
	void _cdecl simple_resize(int size);
	void _cdecl insert_blank(int pos);
	void _cdecl simple_append(const DynamicArray *a);
	void _cdecl simple_assign(const DynamicArray *a);
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
	void _cdecl simple_swap(int i1, int i2);
	void _cdecl simple_move(int source, int target);
	void _cdecl reverse();
	DynamicArray _cdecl ref_subarray(int start, int end = MAGIC_END_INDEX) const;
	int _cdecl simple_index(const void *p) const;
	void* simple_element(int index);
	void _cdecl simple_clear();

	// reference arrays
	void _cdecl forget();
	bool _cdecl is_ref() const;

	void *data;
	int num, allocated, element_size;
};

template <class T>
class Array : public DynamicArray {
public:
	Array() {
		init(sizeof(T));
	}

	// copy constructor
	Array(const Array &a) {
		init(sizeof(T));
		(*this) = a;
	}

	// move constructor
	Array(Array &&a) {
		init(sizeof(T));
		exchange(a);
	}

	Array(std::initializer_list<T> il) {
		init(sizeof(T));
		resize(il.size());
		auto it = il.begin();
		for (int i=0; i<num; i++)
			(*this)[i] = *(it++);

	}

	// kaba
	void _cdecl __init__() {
		init(sizeof(T));
	}

	~Array() {
		clear();
	}
	void _cdecl clear() {
		if (allocated > 0) {
			for (int i=0; i<num; i++)
				(*this)[i].~T();
		}
		simple_clear();
	}
	void _cdecl add(const T &item) {
		resize(num + 1);
		(*this)[num - 1] = item;
	}
	T _cdecl pop() {
		T r;
		if (num > 0) {
			//memcpy(&r, &back(), element_size);
			//DynamicArray::resize(num - 1);
			r = back();
			resize(num - 1);
		}
		return r;
	}
	void _cdecl append(const Array<T> &a) {
		int num0 = num;
		resize(num + a.num);
		for (int i=0; i<a.num; i++)
			(*this)[num0 + i] = a[i];
	}
	void _cdecl erase(int index) {
		if ((index >= 0) and (index < num)) {
			for (int i=index; i<num-1; i++)
				(*this)[i] = (*this)[i+1];
			resize(num - 1);
		}
	}
	void _cdecl insert(const T &item, int index) {
		resize(num + 1);
		if (index < num-1)
			for (int i=num-1; i>index; i--)
				(*this)[i] = (*this)[i-1];
		(*this)[index] = item;
	}


	void __reserve(int size) {
		if (size > allocated) {
			int new_allocated = size * 2;
			if (size > 1000)
				new_allocated = size + size / 2;
			T *new_data = (T*)malloc((size_t)new_allocated * (size_t)element_size);
			for (int i = 0; i < num; i++) {
				new(&new_data[i]) T;
				new_data[i] = ((T*)data)[i];
				((T*)data)[i].~T();
			}
			if (allocated > 0)
				free(data);
			data = new_data;
			allocated = new_allocated;
		} else if (size == 0) {
			clear();
		}
	}
	void _cdecl resize(int size) {
		if (size < num) {
			// shrink -> destruct
			for (int i=size; i<num; i++)
				(*this)[i].~T();
		} else if (size > num) {
			__reserve(size);
			memset((char*)data + element_size * num, 0, element_size * (size - num));
			// grow -> construct
			for (int i=num; i<size; i++)
				new(&(*this)[i]) T;
		}
		num = size;
	}

	void swap(int i1, int i2) {
		if ((i1 < 0) or (i1 >= num))
			return;
		if ((i2 < 0) or (i2 >= num))
			return;
		if (i1 == i2)
			return;
		T t = (*this)[i1];
		(*this)[i1] = (*this)[i2];
		(*this)[i2] = t;
	}

	void move(int source, int target) {
		if (source > target) {
			for (int i = source; i > target; i--)
				swap(i, i - 1);
		} else {
			for (int i = source; i < target; i++)
				swap(i, i + 1);
		}
	}
	int find(const T &item) const {
		for (int i=0; i<num; i++)
			if ((*this)[i] == item)
				return i;
		return -1;
	}
	template<class O>
	O _cdecl sub_ref_as(int start, int end = MAGIC_END_INDEX) const {
		//return reinterpret_cast<Array<T>>(DynamicArray::ref_subarray(start, end));
		O s;
		if (start < 0)
			start += num;
		if (end == MAGIC_END_INDEX)
			end = num;
		else if (end > num)
			end = num;
		else if (end < 0)
			end += num;
		if (end >= start)
			s.num = end - start;
		s.data = ((T*)this->data) + start;
		return s;
	}
	Array<T> _cdecl sub_ref(int start, int end = MAGIC_END_INDEX) const {
		return sub_ref_as<Array<T>>(start, end);
	}

	bool operator == (const Array<T> &o) const {
		if (num != o.num)
			return false;
		for (int i=0; i<num; i++)
			if ((*this)[i] != o[i])
				return false;
		return true;
	}
	bool operator != (const Array<T> &o) const {
		return !(*this == o);
	}

	// copy assignment
	void operator = (const Array<T> &a) {
		if (this != &a) {
			resize(a.num);
			for (int i=0; i<num; i++)
				(*this)[i] = a[i];
		}
	}

	// move assignment
	void operator = (Array<T> &&a) {
		if (this != &a) {
			clear();
			exchange(a);
		}
	}
	void operator += (const Array<T> &a) {
		append(a);
	}
	Array<T> operator + (const Array<T> &a) const {
		Array<T> r = *this;
		r.append(a);
		return r;
	}
	T &operator[] (int index) const {
		return ((T*)data)[index];
	}
	T &back() {
		return ((T*)data)[num - 1];
	}
	const T &_cdecl back() const {
		return ((T*)data)[num - 1];
	}

	Array<T> filter(std::function<bool(const T&)> f) const {
		Array<T> r;
		for (T &e: *this)
			if (f(e))
				r.add(e);
		return r;
	}

	// reference arrays
	void _cdecl set_ref(const Array<T> &a) {
		if (this != &a) {
			clear();
			num = a.num;
			data = a.data;
			element_size = a.element_size;
			allocated = 0;
		}
	}
	void _cdecl make_own() {
		if (!is_ref())
			return;
		T *dd = (T*)data;
		int n = num;
		forget();
		resize(n);
		for (int i=0; i<num; i++)
			(*this)[i] = dd[i];
	}

	// iterators
	class Iterator {
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
