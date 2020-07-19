
#ifndef __MAP_INCLUDED__
#define __MAP_INCLUDED__

#include "set.h"


template<class T1, class T2>
struct MapEntry {
	T1 key;
	T2 value;
	bool operator == (const MapEntry<T1, T2> &e) const
	{	return key == e.key;	}
	bool operator > (const MapEntry<T1, T2> &e) const
	{	return key > e.key;	}
};

class MapKeyError : public Exception {
public:
	MapKeyError(): Exception("key not found"){}
};

template<class T1, class T2>
class Map : public Set<MapEntry<T1, T2> > {
public:
	typedef MapEntry<T1, T2> Entry;
	using DynamicArray::num;
	using DynamicArray::data;
	void _cdecl set(const T1 &key, const T2 &value) {
		int n = find(key);
		if (n >= 0) {
			((Entry*)data)[n].value = value;
		} else {
			MapEntry<T1, T2> e = {key, value};
			Set<MapEntry<T1, T2> >::add(e);
		}
	}
	int _cdecl find(const T1 &key) const {
		Entry e = {key, T2()};
		return Set<Entry>::find(e);
	}
	bool _cdecl contains(const T1 &key) const {
		return find(key) >= 0;
	}
	const T2 &operator[] (const T1 &key) const {
		//msg_write("const[]");
		int n = find(key);
		if (n < 0)
			throw MapKeyError();
		return ((Entry*)data)[n].value;
	}
	T2 &operator[] (const T1 &key) {
		int n = find(key);
		if (n < 0)
			throw MapKeyError();
		return ((Entry*)data)[n].value;
	}
	void drop(const T1 &key) {
		int n = find(key);
		if (n < 0)
			throw MapKeyError();
		Array<MapEntry<T1, T2> >::erase(n);
	}
	Array<T1> keys() const {
		Array<T1> keys;
		for (int i=0; i<this->num; i++)
			keys.add(((Entry*)data)[i].key);
		return keys;
	}
};

template<class T1, class T2>
struct HashMapEntry
{
	T1 key;
	int hash;
	T2 value;
	bool operator == (const HashMapEntry<T1, T2> &e) const
	{	return hash == e.hash;	}
	bool operator > (const HashMapEntry<T1, T2> &e) const
	{	return hash > e.hash;	}
};

template<class T1, class T2>
class HashMap : public Set<HashMapEntry<T1, T2> >
{
public:
	typedef HashMapEntry<T1, T2> Entry;
	using DynamicArray::num;
	using DynamicArray::data;
	int _cdecl add(const T1 &key, const T2 &value)
	{
		HashMapEntry<T1, T2> e = {key, key.hash(), value};
		return Set<HashMapEntry<T1, T2> >::add(e);
	}
	const T2 &operator[] (const T1 &key) const
	{
		//msg_write("const[]");
		int hash = key.hash();
		for (int i=0;i<num;i++)
			if (((Entry*)data)[i].hash == hash)
				return ((Entry*)data)[i].value;
		throw MapKeyError();
		//return T2();
	}
	T2 &operator[] (const T1 &key)
	{
		int hash = key.hash();
		/*HashEntry e = {"", hash, EmptyVar};
		int n = find(e);
		if (n >= 0)
			return ((HashEntry*)data)[n].value;
		n = add(key, EmptyVar);
		return ((HashEntry*)data)[n].value;*/

		for (int i=0;i<num;i++)
			if (((Entry*)data)[i].hash == hash)
				return ((Entry*)data)[i].value;
		throw MapKeyError();
		//return T2();
	}
};

#endif
