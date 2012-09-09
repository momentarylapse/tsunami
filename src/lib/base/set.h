#if !defined(SET_H__INCLUDED_)
#define SET_H__INCLUDED_


#include "array.h"

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
