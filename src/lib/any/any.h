
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
	Any(const Any& a);
	Any(int i);
	Any(int64 i);
	Any(float f);
	Any(double f);
	Any(bool b);
	Any(const string& s);
	Any(const char* s);
	explicit Any(const void* p);
	Any(const Array<Any>& a);
	Any(const Array<int>& a);
	Any(const Dict& m);
	~Any();

	void clear();
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

	static Any parse(const string& s);


	int to_i32() const;
	int64 to_i64() const;
	float to_f32() const;
	double to_f64() const;
	bool to_bool() const;
	void operator=(const Any& a);
	Any operator+(const Any& a) const;
	Any operator-(const Any& a) const;
	void operator+=(const Any& a);
	bool operator==(const Any& other) const;
	bool operator!=(const Any& other) const;

	// list
	void add(const Any& a);
	void append(const Any& a);
	const Any &operator[] (int index) const;
	Any& operator[] (int index);
	Any& back() const;
	int length() const;
	
	int64& as_int() const;
	double& as_float() const;
	bool& as_bool() const;
	string& as_string() const;
	Array<Any>& as_list() const;
	Dict& as_dict() const;
	const void*& as_pointer() const;

	// map/dict
	const Any& operator[] (const string& key) const;
	Any& operator[] (const string& key);
	Array<string> keys() const;
	bool has(const string& key) const;

	// data
	Type type;
	void *data;

	// kaba
	Any* list_get(int i);
	void list_set(int i, const Any& value);
	Any* dict_get(const string& key);
	void dict_set(const string& key, const Any& value);
	void dict_drop(const string& key);

	static Any EmptyDict;
	static Any EmptyList;
	static bool allow_simple_output;
};

template<> string repr(const Any& a);




#endif

