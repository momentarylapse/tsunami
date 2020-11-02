#include "any.h"
#include "../base/map.h"
#include "../config.h"

#ifdef _X_USE_KABA_
#include "../kaba/kaba.h"
namespace kaba {
	extern const Class *TypeAnyList;
	extern const Class *TypeAnyDict;
}
const void *_get_class(int t) {
	if (t == Any::TYPE_INT)
		return kaba::TypeInt;
	if (t == Any::TYPE_FLOAT)
		return kaba::TypeFloat32;
	if (t == Any::TYPE_BOOL)
		return kaba::TypeBool;
	if (t == Any::TYPE_STRING)
		return kaba::TypeString;
	if (t == Any::TYPE_ARRAY)
		return kaba::TypeAnyList;
	if (t == Any::TYPE_MAP)
		return kaba::TypeAnyDict;
	return kaba::TypeVoid;
}
#else
	const void *_get_class(int t) {
		return nullptr;
	}
#endif


string f2s_clean(float f, int dez);

class AnyMap : public Map<string, Any> {};

AnyMap _empty_dummy_map_;
static DynamicArray _empty_dummy_array_ = {NULL, 0, 0, sizeof(Any)};

Any EmptyVar;
//Any EmptyMap = _empty_dummy_map_;
Any Any::EmptyArray = *(Array<Any>*)&_empty_dummy_array_;
Any Any::EmptyMap = Any(_empty_dummy_map_);
bool Any::allow_simple_output = true;

//#define any_db(m)	msg_write(m)
#define any_db(m)


static string type_name(int t) {
	if (t == Any::TYPE_NONE)
		return "-none-";
	if (t == Any::TYPE_INT)
		return "int";
	if (t == Any::TYPE_FLOAT)
		return "float";
	if (t == Any::TYPE_BOOL)
		return "bool";
	if (t == Any::TYPE_STRING)
		return "string";
	if (t == Any::TYPE_ARRAY)
		return "array";
	if (t == Any::TYPE_MAP)
		return "map";
	if (t == Any::TYPE_POINTER)
		return "pointer";
	return "-unknown type-";
}

Any::Any() {
	type = TYPE_NONE;
	data = NULL;
	parent = nullptr;
#ifdef _X_USE_KABA_
	_class = kaba::TypeVoid;
#else
	_class = nullptr;
#endif
}

void Any::__init__() {
	new(this) Any;
}

void Any::__delete__() {
	this->~Any();
}

Any::Any(const Any &a) : Any() {
	*this = a;
}

Any::Any(int i) : Any() {
	create_type(TYPE_INT);
	as_int() = i;
}

Any::Any(float f) : Any() {
	create_type(TYPE_FLOAT);
	as_float() = f;
}

Any::Any(bool b) : Any() {
	create_type(TYPE_BOOL);
	as_bool() = b;
}

Any::Any(const string &s) : Any() {
	create_type(TYPE_STRING);
	as_string() = s;
}

Any::Any(const void *p) : Any() {
	create_type(TYPE_POINTER);
	as_pointer() = p;
}

Any::Any(const Array<Any> &a) : Any() {
	create_type(TYPE_ARRAY);
	as_array() = a;
}

Any::Any(const Array<int> &a) : Any() {
	create_type(TYPE_ARRAY);
	for (int i: a)
		as_array().add(i);
}

Any::Any(const AnyMap &m) : Any() {
	create_type(TYPE_MAP);
	as_map() = m;
}

Any Any::ref() {
	Any r;
	r.parent = this;
	r.type = type;
	r.data = data;
	r._class = _class;
	any_db(format("ref  %s -> %s:   %s", p2s(this), p2s(parent), str()));
	return r;
}

void Any::create_type(int _type) {
	if (parent) {
		any_db(format("parent create  %s -> %s:   %s", p2s(this), p2s(parent), str()));
		parent->create_type(_type);
		sync_from_parent();
		return;
	}
	clear();
	type = _type;
	_class = _get_class(type);
	if (is_int()) {
		data = new int;
	} else if (is_float()) {
		data = new float;
	} else if (is_bool()) {
		data = new bool;
	} else if (is_pointer()) {
		data = new (void*);
	} else if (is_string()) {
		data = new string;
	} else if (is_array()) {
		data = new Array<Any>;
	} else if (is_map()) {
		data = new AnyMap;
	} else if (is_empty()) {
	} else {
		msg_error("can not create " + type_name(type));
	}
}

void Any::sync_to_parent() {
	if (parent) {
		any_db(format("sync  %s -> %s:   %s", p2s(this), p2s(parent), str()));
		parent->data = data;
		parent->type = type;
		parent->_class = _class;
		parent->sync_to_parent();
	}
}

void Any::sync_from_parent() {
	if (parent) {
		any_db(format("sync  %s << %s:   %s", p2s(this), p2s(parent), parent->str()));
		parent->sync_from_parent();
		data = parent->data;
		type = parent->type;
		_class = parent->_class;
	}
}

/*Any::Any(const AnyHashMap &a)
{
	type = TYPE_DICT;
	data = new AnyHashMap;
	*((AnyHashMap*)data) = a;
}*/

Any::~Any() {
	if (!parent)
		clear();
}

void Any::clear() {
	//any_db(format("clear  %s", p2s(this)));
	if (parent) {
		any_db(format("parent clear  %s -> %s:   %s", p2s(this), p2s(parent), str()));
		parent->clear();
		sync_from_parent();
	} else {
		if (is_int())
			delete &as_int();
		else if (is_float())
			delete &as_float();
		else if (is_bool())
			delete &as_bool();
		else if (is_string())
			delete &as_string();
		else if (is_array())
			delete &as_array();
		else if (is_map())
			delete &as_map();
		else if (is_pointer())
			delete &as_pointer();
		else if (!is_empty())
			msg_error("any.clear(): " + type_name(type));
		type = TYPE_NONE;
		_class = _get_class(type);
		data = NULL;
	}
}

bool Any::is_empty() const {
	return type == TYPE_NONE;
}

bool Any::is_int() const {
	return type == TYPE_INT;
}

bool Any::is_float() const {
	return type == TYPE_FLOAT;
}

bool Any::is_bool() const {
	return type == TYPE_BOOL;
}

bool Any::is_string() const {
	return type == TYPE_STRING;
}

bool Any::is_pointer() const {
	return type == TYPE_POINTER;
}

bool Any::is_array() const {
	return type == TYPE_ARRAY;
}

bool Any::is_map() const {
	return type == TYPE_MAP;
}

bool key_needs_quotes(const string &k) {
	if (k == "")
		return true;
	for (char c: k) {
		if (c >= 'a' and c <= 'z')
			continue;
		if (c >= 'A' and c <= 'Z')
			continue;
		if (c >= '0' and c <= '9')
			continue;
		if (c == '_' or c == '-') // ya, we allow '-'...
			continue;
		return true;
	}
	return false;
}

string minimal_key_repr(const string &k) {
	if (key_needs_quotes(k) or !Any::allow_simple_output)
		return k.repr();
	return k;
}

string Any::repr() const {
	if (is_int()) {
		return i2s(as_int());
	} else if (is_float()) {
		return f2s_clean(as_float(), 6);
	} else if (is_bool()) {
		return b2s(as_bool());
	} else if (is_string()) {
		return as_string().repr();
	} else if (is_pointer()) {
		return p2s(as_pointer());
	} else if (is_array()) {
		string s = "[";
		for (Any &p: as_array()) {
			if (s.num > 1)
				s += ", ";
			s += p.repr();
		}
		return s + "]";
	} else if (is_map()) {
		string s = "{";
		for (AnyMap::Entry &p: as_map()) {
			if (s.num > 1)
				s += ", ";
			s += minimal_key_repr(p.key) + ": " + p.value.repr();
		}
		return s + "}";
	} else if (is_empty()) {
		return "nil";
	} else {
		return "unhandled Any.str(): " + type_name(type);
	}
}

string Any::str() const {
	if (is_string())
		return as_string();
	return repr();
}

bool str_is_number(const string &s) {
	for (int i=0; i<s.num; i++) {
		if (s[i] >= '0' and s[i] <= '9')
			continue;
		if (s[i] == '-')
			continue;
		if (s[i] == '.')
			continue;
		return false;
	}
	return true;
}


void any_parse_part(Any &a, const Array<string> &tokens, int &pos) {
	auto expect_no_end = [&tokens, &pos] {
		if (pos >= tokens.num)
			throw Exception("string ended unexpectedly");
	};
	auto expect_token = [&tokens, &pos, expect_no_end] (const string &t) {
		expect_no_end();
		if (tokens[pos] != t)
			throw Exception(format("'%s' expected, got '%s'", t, tokens[pos]));
		pos ++;
	};
	expect_no_end();
	auto &cur = tokens[pos];
	if (cur == "[") {
		a.create_type(Any::TYPE_ARRAY);
		pos ++;
		while (tokens[pos] != "]") {
			a.as_array().resize(a.as_array().num + 1);
			any_parse_part(a.as_array().back(), tokens, pos);
			if (tokens[pos] == "]")
				break;
			expect_token(",");
		}
		pos ++;
	} else if (cur == "{") {
		a.create_type(Any::TYPE_MAP);
		pos ++;
		while (tokens[pos] != "}") {
			Any key;
			any_parse_part(key, tokens, pos);
			if (key.type != Any::TYPE_STRING)
				throw Exception("only strings allowed as dict keys");
			expect_token(":");
			a.as_map().set(key.as_string(), Any());
			any_parse_part(a.as_map()[key.as_string()], tokens, pos);
			if (tokens[pos] == "}")
				break;
			expect_token(",");
		}
		pos ++;
	} else if (cur == "nil") {
		a.clear();
		pos ++;
	} else if (str_is_number(cur)) {
		if (cur.has_char('.')) {
			a.create_type(Any::TYPE_FLOAT);
			a.as_float() = cur._float();
		} else {
			a.create_type(Any::TYPE_INT);
			a.as_int() = cur._int();
		}
		pos ++;
	} else if (cur == "true" or cur == "false") {
		a.create_type(Any::TYPE_BOOL);
		a.as_bool() = cur._bool();
		pos ++;
	} else if (cur[0] == '\"' or cur[0] == '\'') {
		a.create_type(Any::TYPE_STRING);
		a.as_string() = cur.substr(1, -2).unescape();
		pos ++;
	} else {
		// token (string without quotes)
		a.create_type(Any::TYPE_STRING);
		a.as_string() = cur;
		pos ++;
		//throw Exception(format("what is '%s'?", cur));
	}
};

Any Any::parse(const string &s) {
	auto tokens = s.parse_tokens(",:[](){}\"");
	int pos = 0;

	Any r;
	any_parse_part(r, tokens, pos);
	return r;
}

bool Any::_bool() const {
	if (is_bool())
		return as_bool();
	if (is_int())
		return as_int() != 0;
	throw Exception("can not interpret as bool: " + type_name(type));
}

int Any::_int() const {
	if (is_int())
		return as_int();
	if (is_bool())
		return (int)as_bool();
	if (is_float())
		return (int)as_float();
	if (is_string())
		return as_string()._int();
	throw Exception("can not interpret as int: " + type_name(type));
}

float Any::_float() const {
	if (is_int())
		return (float)as_int();
	if (is_float())
		return as_float();
	if (is_string())
		return as_string()._float();
	throw Exception("can not interpret as float: " + type_name(type));
}

void Any::operator = (const Any &a) {
	if (&a != this) {
		bool b = parent;
		if (parent)
			any_db("=   IS REF " + str());
		create_type(a.type);
		if (a.is_int()) {
			as_int() = a.as_int();
		} else if (a.is_float()) {
			as_float() = a.as_float();
		} else if (a.is_bool()) {
			as_bool() = a.as_bool();
		} else if (a.is_pointer()) {
			as_pointer() = a.as_pointer();
		} else if (a.is_string()) {
			as_string() = a.as_string();
		} else if (a.is_array()) {
			as_array() = a.as_array();
		} else if (a.is_map()) {
			as_map() = a.as_map();
		} else if (a.is_empty()) {
			clear();
		} else {
			clear();
			msg_error("any = any: " + type_name(a.type));
		}
		if (b)
			any_db(str());
	}
}

Any Any::operator + (const Any &a) const {
	if (is_int() and a.is_int())
		return _int() + a._int();
	if ((is_float() or is_int()) and (a.is_float() or a.is_int()))
		return _float() + a._float();
	if (is_string() and a.is_string())
		return str() + a.str();
	throw Exception(format("%s + %s not allowed", type_name(type), type_name(a.type)));
	return Any();
}

Any Any::operator - (const Any &a) const {
	if (is_int() and a.is_int())
		return _int() - a._int();
	if ((is_float() or is_int()) and (a.is_float() or a.is_int()))
		return _float() - a._float();
	throw Exception(format("%s - %s not allowed", type_name(type), type_name(a.type)));
	return Any();
}

void Any::operator += (const Any &a) {
	if (is_int() and (a.is_int() or a.is_float()))
		as_int() += a._int();
	else if (is_float() and (a.is_float() or a.is_int()))
		as_float() += a._float();
	else if (is_string() and a.is_string())
		as_string() += a.str();
	else if (is_array() and a.is_array())
		append(a);
	else if (is_array())
		add(a);
	else
		throw Exception(format("%s += %s not allowed", type_name(type), type_name(a.type)));
}

void Any::add(const Any &a) {
	if (parent) {
		parent->add(a);
		sync_from_parent();
		any_db(format("parent add  %s -> %s:   %s", p2s(this), p2s(parent), str()));
		return;
	}
	if (is_empty())
		create_type(TYPE_ARRAY);
	if (is_array()) {
		if (&a == this) {
			Any b = a;
			as_array().add(b);
		} else {
			as_array().add(a);
		}
	} else {
		throw Exception("only allowed for arrays: " + type_name(type));
	}
}

// TODO: map.append(map)
void Any::append(const Any &a) {
	if (parent) {
		parent->append(a);
		sync_from_parent();
		return;
	}
	if (!a.is_array())
		throw Exception("parameter not an array: " + type_name(a.type));
	if (is_empty())
		create_type(TYPE_ARRAY);
	if (is_array() and a.is_array()) {
		if (&a == this) {
			Any b = a;
			as_array().append(b.as_array());
		} else {
			as_array().append(a.as_array());
		}
	} else {
		throw Exception("not an array: " + type_name(type));
	}
}

int Any::length() {
	if (is_array())
		return as_array().num;
	if (is_map())
		return as_map().num;
	if (is_string())
		return as_string().num;
	return 0;
}

Any &Any::operator[] (int index) {
	if (!is_array())
		msg_error("only allowed for arrays: " + type_name(type));
	return as_array()[index];
}

const Any &Any::operator[] (int index) const {
	if (!is_array())
		msg_error("only allowed for arrays: " + type_name(type));
	return as_array()[index];
}

Any &Any::back() {
	if (!is_array())
		msg_error("only allowed for arrays: " + type_name(type));
	return as_array().back();
}

const Any &Any::operator[] (const string &key) const {
	if (!is_map())
		msg_error("only allowed for maps: " + type_name(type));
	return as_map()[key];
}

Any &Any::operator[] (const string &key) {
	if (is_empty())
		create_type(TYPE_MAP);
	if (!is_map())
		msg_error("only allowed for maps: " + type_name(type));
	return as_map()[key];
}

Array<string> Any::keys() const {
	if (!is_map())
		return {};
	return as_map().keys();
}

bool Any::has(const string &key) const {
	if (!is_map())
		return false;
	return sa_contains(as_map().keys(), key);
}

int& Any::as_int() const {
	return *(int*)data;
}

float& Any::as_float() const {
	return *(float*)data;
}

bool& Any::as_bool() const {
	return *(bool*)data;
}

const void*& Any::as_pointer() const {
	return *(const void**)data;
}

string& Any::as_string() const {
	return *(string*)data;
}

AnyMap& Any::as_map() const {
	return *(AnyMap*)data;
}

Array<Any>& Any::as_array() const {
	return *(Array<Any>*)data;
}

Any Any::array_get(int i) {
	if (!is_array())
		throw Exception("not an array: " + type_name(type));
	return (*this)[i].ref();
}

void Any::array_set(int i, const Any &value) {
	if (parent) {
		parent->array_set(i, value);
		sync_from_parent();
		return;
	}
	if (is_empty())
		create_type(TYPE_ARRAY);
	if (!is_array())
		throw Exception("not an array: " + type_name(type));
	(*this)[i] = value;
}

Any Any::map_get(const string &key) {
	if (!is_map())
		throw Exception("not a map: " + type_name(type));
	if (!as_map().contains(key))
		throw Exception("key not found: " + key);
	return as_map()[key].ref();
}

void Any::map_set(const string &key, const Any &value) {
	if (parent) {
		parent->map_set(key, value);
		sync_from_parent();
		return;
	}
	if (is_empty())
		create_type(TYPE_MAP);
	if (!is_map())
		throw Exception("not a map: " + type_name(type));
	as_map().set(key, value);
}

void Any::map_drop(const string &key) {
	if (parent) {
		parent->map_drop(key);
		sync_from_parent();
		return;
	}
	if (!is_map())
		throw Exception("not a map: " + type_name(type));
	as_map().drop(key);
}
