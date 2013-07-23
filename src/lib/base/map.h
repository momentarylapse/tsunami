
#ifndef __MAP_INCLUDED__
#define __MAP_INCLUDED__

#include "set.h"


template<class T1, class T2>
struct MapEntry
{
	T1 key;
	T2 value;
	bool operator == (const MapEntry<T1, T2> &e) const
	{	return key == e.key;	}
	bool operator > (const MapEntry<T1, T2> &e) const
	{	return key > e.key;	}
};

template<class T1, class T2>
class Map : public Set<MapEntry<T1, T2> >
{
	T2 dummy;
public:
	typedef MapEntry<T1, T2> Entry;
	using DynamicArray::num;
	using DynamicArray::data;
	int _cdecl add(const T1 &key, const T2 &value)
	{
		MapEntry<T1, T2> e = {key, value};
		return Set<MapEntry<T1, T2> >::add(e);
	}
	int _cdecl find(const T1 &key) const
	{
		Entry e = {key, dummy};
		return Set<Entry>::find(e);
	}
	bool _cdecl contains(const T1 &key) const
	{
		return find(key) >= 0;
	}
	const T2 &operator[] (const T1 &key) const
	{
		//msg_write("const[]");
		int n = find(key);
		if (n >= 0)
			return ((Entry*)data)[n].value;
		return dummy;
	}
	T2 &operator[] (const T1 &key)
	{
		int n = find(key);
		if (n >= 0)
			return ((Entry*)data)[n].value;

		n = add(key, dummy);
		return ((Entry*)data)[n].value;
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
	T2 dummy;
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
		return dummy;
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
		int n = add(key, dummy);
		return ((Entry*)data)[n].value;
	}
};

#endif
