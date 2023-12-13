
#ifndef __MAP_INCLUDED__
#define __MAP_INCLUDED__

#include "set.h"

namespace base {

template<class K, class V>
struct MapEntry {
	K key;
	V value;
	bool operator == (const MapEntry<K, V> &e) const {
		return key == e.key;
	}
	bool operator > (const MapEntry<K, V> &e) const {
		return key > e.key;
	}
};

class MapKeyError : public Exception {
public:
	MapKeyError(): Exception("key not found"){}
};

template<class K, class V>
class map : public set<MapEntry<K, V>> {
public:
	using KeyType = K;
	using ValueType = V;
	using Entry = MapEntry<K, V>;
	using DynamicArray::num;
	using DynamicArray::data;
	void set(const K &key, const V &value) {
		int n = find(key);
		if (n >= 0)
			((Entry*)data)[n].value = value;
		else
			::base::set<MapEntry<K, V>>::add({key, value});
	}
	int find(const K &key) const {
		return ::base::set<Entry>::find({key, V()});
	}
	bool contains(const K &key) const {
		return find(key) >= 0;
	}
	V &by_index(int index) {
		return ((Entry*)data)[index].value;
	}
	const V &by_index(int index) const {
		return ((Entry*)data)[index].value;
	}
	const V &operator[] (const K &key) const {
		//msg_write("const[]");
		int n = find(key);
		if (n < 0)
			throw MapKeyError();
		return by_index(n);
	}
	V &operator[] (const K &key) {
		int n = find(key);
		if (n < 0)
			throw MapKeyError();
		return by_index(n);
	}
	void drop(const K &key) {
		int n = find(key);
		if (n < 0)
			throw MapKeyError();
		Array<MapEntry<K, V> >::erase(n);
	}
	Array<K> keys() const {
		Array<K> keys;
		for (auto && [k,v]: *this)
			keys.add(k);
		return keys;
	}
	string str() const {
		string r;
		for (auto && [k,v]: *this) {
			if (r.num > 0)
				r += ", ";
			r += repr(k) + ": " + repr(v);
		}
		return "{" + r + "}";
	}

	struct ConstIterator {
		void operator ++()
		{	index ++;	p ++;	}
		void operator ++(int) // postfix
		{	index ++;	p ++;	}
		bool operator == (const ConstIterator &i) const
		{	return p == i.p;	}
		bool operator != (const ConstIterator &i) const
		{	return p != i.p;	}
		const std::pair<const K&,const V&> operator *()
		{	return {p->key, p->value};	}
		ConstIterator(const map<K, V> &m, int n) {
			p = &((const Entry*)m.data)[n];
			//p = &m.by_index(n);
			index = n;
		}
		int index;
		const Entry* p;
	};

	struct Iterator {
		void operator ++()
		{	index ++;	p ++;	}
		void operator ++(int) // postfix
		{	index ++;	p ++;	}
		bool operator == (const Iterator &i) const
		{	return p == i.p;	}
		bool operator != (const Iterator &i) const
		{	return p != i.p;	}
		std::pair<K&,V&> operator *()
		{	return {p->key, p->value};	}
		Iterator(map<K, V> &m, int n) {
			p = &((Entry*)m.data)[n];
			index = n;
		}
		int index;
		Entry* p;
	};
	ConstIterator begin() const {
		return ConstIterator(*this, 0);
	}
	ConstIterator end() const {
		return ConstIterator(*this, this->num);
	}
	Iterator begin() {
		return Iterator(*this, 0);
	}
	Iterator end() {
		return Iterator(*this, this->num);
	}
};

template<class K, class V>
struct HashMapEntry {
	K key;
	int hash;
	V value;
	bool operator == (const HashMapEntry<K, V> &e) const {
		return hash == e.hash;
	}
	bool operator > (const HashMapEntry<K, V> &e) const {
		return hash > e.hash;
	}
};

template<class K, class V>
class hash_map : public set<HashMapEntry<K, V>> {
public:
	typedef HashMapEntry<K, V> Entry;
	using DynamicArray::num;
	using DynamicArray::data;
	int _cdecl add(const K &key, const V &value) {
		HashMapEntry<K, V> e = {key, key.hash(), value};
		return ::base::set<HashMapEntry<K, V>>::add(e);
	}
	const V &operator[] (const K &key) const {
		//msg_write("const[]");
		int hash = key.hash();
		for (int i=0;i<num;i++)
			if (((Entry*)data)[i].hash == hash)
				return ((Entry*)data)[i].value;
		throw MapKeyError();
		//return V();
	}
	V &operator[] (const K &key) {
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
		//return V();
	}
};

}

#endif
