
#ifndef __ANY_INCLUDED__
#define __ANY_INCLUDED__

#include "../base/base.h"
#include "../base/map.h"


class Any {
public:
	using Dict = base::map<string, Any>;

	enum class Type {
		None,
		Int,
		Float,
		Bool,
		String,
		List,
		Dict,
		Pointer
	};

	Any();
	Any(const Any &a);
	Any(int i);
	Any(float f);
	Any(bool b);
	Any(const string &s);
	Any(const char *s);
	Any(const void *p);
	Any(const Array<Any> &a);
	Any(const Array<int> &a);
	Any(const Dict &m);
	~Any();

	void clear();
	Any ref();
	void sync_to_parent();
	void sync_from_parent();
	void create_type(Type type);

	bool is_empty() const;
	bool is_string() const;
	bool is_int() const;
	bool is_float() const;
	bool is_bool() const;
	bool is_pointer() const;
	bool is_list() const;
	bool is_dict() const;

	string str() const;
	string repr() const;

	static Any parse(const string &s);


	int _int() const;
	float _float() const;
	bool _bool() const;
	void operator= (const Any &a);
	Any operator+ (const Any &a) const;
	Any operator- (const Any &a) const;
	void operator+= (const Any &a);
	bool operator==(const Any& other) const;
	bool operator!=(const Any& other) const;

	// list
	void add(const Any &a);
	void append(const Any &a);
	const Any &operator[] (int index) const;
	Any &operator[] (int index);
	Any &_cdecl back();
	int length() const;
	
	int& as_int() const;
	float& as_float() const;
	bool& as_bool() const;
	string& as_string() const;
	Array<Any>& as_list() const;
	Dict& as_dict() const;
	const void*& as_pointer() const;

	// map/dict
	const Any &operator[] (const string &key) const;
	Any &operator[] (const string &key);
	Array<string> keys() const;
	bool has(const string &key) const;

	// data
	Type type;
	void *data;
	Any *parent;

	// kaba
	Any* list_get(int i);
	void list_set(int i, const Any &value);
	Any* dict_get(const string &key);
	void dict_set(const string &key, const Any &value);
	void dict_drop(const string &key);

	static Any EmptyDict;
	static Any EmptyList;
	static bool allow_simple_output;
};

template<> string repr(const Any& a);




#endif

