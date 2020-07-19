#include "base.h"
#include <stdlib.h>
#ifdef OS_WINDOWS
	#include <malloc.h>
#endif
//#include <stdio.h>


void DynamicArray::init(int _element_size_)
{
	num = 0;
	element_size = _element_size_;
	allocated = 0;
	data = nullptr;
//	printf("init %d\n", element_size);
}

void DynamicArray::simple_reserve(int size)
{
//	printf("        reserve  %d\n", size);
	if (allocated == 0){
		if (size > 0){
			allocated = size;
			data = malloc((size_t)allocated * (size_t)element_size);
//			printf("          new  %p  ", data);
		}
	}else if (size > allocated){
		if (size > 1000)
			allocated = size + size / 2;
		else
			allocated = size * 2;
//		void *data0 = data;
		data = realloc(data, (size_t)allocated * (size_t)element_size);
//		printf("          %p  ->  %p ", data0, data);
	}else if (size == 0)
		simple_clear();
//	printf("        /r  %d\n", allocated);
}

void DynamicArray::simple_resize(int size)
{
//	printf("        resize %d\n", size);
//	printf("        .\n");
	if (size > num){
		simple_reserve(size);
		memset((char*)data + num * element_size, 0, (size - num) * element_size);
	}
	num = size;
//	printf("        /resize\n");
}

void DynamicArray::insert_blank(int pos)
{
	simple_resize(num + 1);
	if (num > pos + 1)
		memmove((char*)data + (pos + 1) * element_size,
		        (char*)data +  pos      * element_size,
		        (num - 1 - pos) * element_size);
	memset((char*)data + pos * element_size, 0, element_size);
}

void DynamicArray::simple_append(const DynamicArray *a)
{
	if (a->num > 0){
//		printf("        append %d %d %d   - %d %d %d\n", num, element_size, allocated, a->num, a->element_size, a->allocated);
		int num_old = num;
		int num_a_old = a->num; // in case (this = a)
//		printf("        a\n");
		simple_resize(num + a->num);
//		printf("        b\n");
		memcpy(&((char*)data)[num_old * element_size], a->data, num_a_old * element_size);
//		printf("        /append\n");
	}
}

void DynamicArray::append_p_single(void *p)
{
	simple_reserve(num + 1);
	((void**)data)[num ++] = p;
}

void DynamicArray::append_4_single(int x)
{
	simple_reserve(num + 1);
	((int*)data)[num ++] = x;
}

void DynamicArray::append_f_single(float x)
{
	simple_reserve(num + 1);
	((float*)data)[num ++] = x;
}

void DynamicArray::append_d_single(double x)
{
	simple_reserve(num + 1);
	((double*)data)[num ++] = x;
}

void DynamicArray::append_1_single(char x)
{
	simple_reserve(num + 1);
	((char*)data)[num ++] = x;
}

void DynamicArray::append_single(const void *d)
{
	simple_reserve(num + 1);
	memcpy(&((char*)data)[num * element_size], d, element_size);
	num ++;
}

void DynamicArray::insert_p_single(void *p, int index)
{
	insert_blank(index);
	((void**)data)[index] = p;
}

void DynamicArray::insert_4_single(int x, int index)
{
	insert_blank(index);
	((int*)data)[index] = x;
}

void DynamicArray::insert_f_single(float x, int index)
{
	insert_blank(index);
	((float*)data)[index] = x;
}

void DynamicArray::insert_d_single(double x, int index)
{
	insert_blank(index);
	((double*)data)[index] = x;
}

void DynamicArray::insert_1_single(char x, int index)
{
	insert_blank(index);
	((char*)data)[index] = x;
}

void DynamicArray::insert_single(const void *d, int index)
{
	insert_blank(index);
	memcpy(&((char*)data)[index * element_size], d, element_size);
}

void DynamicArray::simple_swap(int i1, int i2)
{
	if ((i1 < 0) or (i1 >= num))
		return;
	if ((i2 < 0) or (i2 >= num))
		return;
	if (i1 == i2)
		return;
	char *p1 = &((char*)data)[i1 * element_size];
	char *p2 = &((char*)data)[i2 * element_size];
	if (element_size == 1){
		char t = *p1;
		*p1 = *p2;
		*p2 = t;
	}else if (element_size == 4){
		int t = *(int*)p1;
		*(int*)p1 = *(int*)p2;
		*(int*)p2 = t;
	}else if (element_size <= 64){
		char t[64];
		memcpy(t, p1, element_size);
		memcpy(p1, p2, element_size);
		memcpy(p2, t, element_size);
	}else{
		char *t = new char[element_size];
		memcpy(t, p1, element_size);
		memcpy(p1, p2, element_size);
		memcpy(p2, t, element_size);
		delete[](t);
	}
}

void DynamicArray::simple_move(int source, int target)
{
	if (source > target){
		for (int i=source; i>target; i--)
			simple_swap(i, i-1);
	}else{
		for (int i=source; i<target; i++)
			simple_swap(i, i+1);
	}
}

void DynamicArray::reverse()
{
	int n = num / 2;
	for (int i=0;i<n;i++)
		simple_swap(i, num - i - 1);
}

void DynamicArray::simple_assign(const DynamicArray *a)
{
	if (a != this){
//		printf("assign\n");
		element_size = a->element_size;
		simple_resize(a->num);
		if (num > 0)
			memcpy(data, a->data, num * element_size);
//		printf("/assign\n");
	}
}

void DynamicArray::exchange(DynamicArray &a)
{
	int tnum = num;
	num = a.num;
	a.num = tnum;
	void *tdata = data;
	data = a.data;
	a.data = tdata;
	int tall = allocated;
	allocated = a.allocated;
	a.allocated = tall;
}

void DynamicArray::simple_clear()
{
	if (allocated > 0){
//		printf("        ~   %p %d  %d\n", data, element_size, num);
		free(data);
	}
	data = nullptr;
	allocated = 0;
	num = 0;
}

void DynamicArray::delete_single(int index)
{
	int n = (num - 1) * element_size;
#if 0
	for (int i=index*element_size;i<n;i++)
	    ((char*)data)[i] = ((char*)data)[i + element_size];
#else
	memmove(&((char*)data)[index * element_size], &((char*)data)[(index + 1) * element_size], (num - index - 1) * element_size);
#endif
	num --;
}


void DynamicArray::forget() {
	data = nullptr;
	allocated = 0;
	num = 0;
}

bool DynamicArray::is_ref() const {
	return (num > 0) and (allocated == 0);
}

int DynamicArray::index(const void *p)
{	return ((int_p)p - (int_p)data) / element_size;	}


DynamicArray DynamicArray::ref_subarray(int start, int end)
{
	DynamicArray s;
	s.init(element_size);

	// magic value (-_-)'
	if ((unsigned)end == 0x81234567)
		end = num;

	if (start < 0)
		start += num;
	if (start < 0)
		start = 0;
	if (end < 0)
		end += num;
	if (end > num)
		end = num;
	s.num = max(end - start, 0);
	s.data = &((char*)data)[element_size * start];
	return s;
}



// Array<char>
template <> void Array<char>::add(const char &item)
{	DynamicArray::append_1_single(item);	}
template <> void Array<char>::erase(int index)
{	DynamicArray::delete_single(index);	}
template <> void Array<char>::operator = (const Array<char> &a)
{	DynamicArray::simple_assign(&a);	}
template <> void Array<char>::operator += (const Array<char> &a)
{	DynamicArray::simple_append(&a);	}

// Array<bool>
template <> void Array<bool>::add(const bool &item)
{	DynamicArray::append_1_single(item);	}
template <> void Array<bool>::erase(int index)
{	DynamicArray::delete_single(index);	}
template <> void Array<bool>::operator = (const Array<bool> &a)
{	DynamicArray::simple_assign(&a);	}
template <> void Array<bool>::operator += (const Array<bool> &a)
{	DynamicArray::simple_append(&a);	}

// Array<int>
template <> void Array<int>::add(const int &item)
{	DynamicArray::append_4_single(item);	}
template <> void Array<int>::erase(int index)
{	DynamicArray::delete_single(index);	}
template <> void Array<int>::operator = (const Array<int> &a)
{	DynamicArray::simple_assign(&a);	}
template <> void Array<int>::operator += (const Array<int> &a)
{	DynamicArray::simple_append(&a);	}

// Array<float>
template <> void Array<float>::add(const float &item)
{	DynamicArray::append_4_single(*(int*)&item);	}
template <> void Array<float>::erase(int index)
{	DynamicArray::delete_single(index);	}
template <> void Array<float>::operator = (const Array<float> &a)
{	DynamicArray::simple_assign(&a);	}
template <> void Array<float>::operator += (const Array<float> &a)
{	DynamicArray::simple_append(&a);	}



/*template <> void Array<char>::insert(char c, int pos)
{
	DynamicArray::resize(((DynamicArray*)this)->num + 1);
	for (int i=((DynamicArray*)this)->num-2;i>=pos;i--)
		(*this)[i+1] = (*this)[i];
	(*this)[pos] = c;
}*/

