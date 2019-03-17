#if !defined(SET_H__INCLUDED_)
#define SET_H__INCLUDED_


#include "array.h"

template <class T>
class Set : public Array<T>
{
	public:
		using DynamicArray::num;

		Set() : Array<T>(){}
		Set(const T &item) : Array<T>({item}){}

		int _cdecl add(const T &item)
		{
			int i0 = 0;
			int i1 = num;
			while(i1 > i0){
				int i = (i1 + i0) >> 1;
				if ((*this)[i] == item)
					return -1;
				else if ((*this)[i] > item){
					i1 = i;
				}else{
					i0 = i + 1;
				}
			}
			if (i0 < num)
				this->insert(item, i0);
			else
				((Array<T>*)this)->add(item);
			return i0;
		}
		void _cdecl join(const Set &a)
		{
			for (int i=0;i<a.num;i++)
				add(a[i]);
		}
		int _cdecl find(const T &item) const
		{
			int i0 = 0;
			int i1 = num;
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
		void _cdecl erase(const T &item)
		{
			int index = find(item);
			if (index >= 0)
				((Array<T>*)this)->erase(index);
		}
		bool _cdecl contains(const T &item) const
		{
			return (find(item) >= 0);
		}
};


#endif
