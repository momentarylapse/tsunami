#include "any.h"
#include "../base/map.h"
#include "../config.h"

#ifdef _X_USE_KABA_
#include "../kaba/kaba.h"
namespace Kaba {
	extern const Class *TypeAnyList;
	extern const Class *TypeAnyDict;
}
const void *_get_class(int t) {
	if (t == Any::TYPE_INT)
		return Kaba::TypeInt;
	if (t == Any::TYPE_FLOAT)
		return Kaba::TypeFloat32;
	if (t == Any::TYPE_BOOL)
		return Kaba::TypeBool;
	if (t == Any::TYPE_STRING)
		return Kaba::TypeString;
	if (t == Any::TYPE_ARRAY)
		return Kaba::TypeAnyList;
	if (t == Any::TYPE_MAP)
		return Kaba::TypeAnyDict;
	return Kaba::TypeVoid;
}
#else
	const void *_get_class(int t) {
		return nullptr;
	}
#endif

class AnyMap : public Map<string, Any> {};

AnyMap _empty_dummy_map_;
static DynamicArray _empty_dummy_array_ = {NULL, 0, 0, sizeof(Any)};

Any EmptyVar;
//Any EmptyMap = _empty_dummy_map_;
Any Any::EmptyArray = *(Array<Any>*)&_empty_dummy_array_;
Any Any::EmptyMap = Any(_empty_dummy_map_);

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
	_class = Kaba::TypeVoid;
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
	*as_int() = i;
}

Any::Any(float f) : Any() {
	create_type(TYPE_FLOAT);
	*as_float() = f;
}

Any::Any(bool b) : Any() {
	create_type(TYPE_BOOL);
	*as_bool() = b;
}

Any::Any(const string &s) : Any() {
	create_type(TYPE_STRING);
	*as_string() = s;
}

Any::Any(const void *p) : Any() {
	create_type(TYPE_POINTER);
	*as_pointer() = p;
}

Any::Any(const Array<Any> &a) : Any() {
	create_type(TYPE_ARRAY);
	*as_array() = a;
}

Any::Any(const AnyMap &m) : Any() {
	create_type(TYPE_MAP);
	*as_map() = m;
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
	if (type == TYPE_INT) {
		data = new int;
	} else if (type == TYPE_FLOAT) {
		data = new float;
	} else if (type == TYPE_BOOL) {
		data = new bool;
	} else if (type == TYPE_POINTER) {
		data = new (void*);
	} else if (type == TYPE_STRING) {
		data = new string;
	} else if (type == TYPE_ARRAY) {
		data = new Array<Any>;
	} else if (type == TYPE_MAP) {
		data = new AnyMap;
	} else if (type == TYPE_NONE) {
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
		if (type == TYPE_INT)
			delete as_int();
		else if (type == TYPE_FLOAT)
			delete as_float();
		else if (type == TYPE_BOOL)
			delete as_bool();
		else if (type == TYPE_STRING)
			delete as_string();
		else if (type == TYPE_ARRAY)
			delete as_array();
		else if (type == TYPE_MAP)
			delete as_map();
		else if (type == TYPE_POINTER)
			delete as_pointer();
		else if (type != TYPE_NONE)
			msg_error("any.clear(): " + type_name(type));
		type = TYPE_NONE;
		_class = _get_class(type);
		data = NULL;
	}
}

string Any::repr() const {
	if (type == TYPE_INT) {
		return i2s(*as_int());
	} else if (type == TYPE_FLOAT) {
		return f2s(*as_float(), 6);
	} else if (type == TYPE_BOOL) {
		return b2s(*as_bool());
	} else if (type == TYPE_STRING) {
		return as_string()->repr();
	} else if (type == TYPE_POINTER) {
		return p2s(*as_pointer());
	} else if (type == TYPE_ARRAY) {
		string s = "[";
		for (Any &p: *as_array()) {
			if (s.num > 1)
				s += ", ";
			s += p.repr();
		}
		return s + "]";
	} else if (type == TYPE_MAP) {
		string s = "{";
		for (AnyMap::Entry &p: *as_map()) {
			if (s.num > 1)
				s += ", ";
			s += p.key.repr() + ": " + p.value.repr();
		}
		return s + "}";
	} else if (type == TYPE_NONE) {
		return "<empty>";
	} else {
		return "unhandled Any.str(): " + type_name(type);
	}
}

string Any::str() const {
	if (type == TYPE_STRING)
		return *as_string();
	return repr();
}

bool Any::_bool() const {
	if (type == TYPE_BOOL)
		return *as_bool();
	if (type == TYPE_INT)
		return *as_int() != 0;
	throw Exception("can not interpret as bool: " + type_name(type));
}

int Any::_int() const {
	if (type == TYPE_INT)
		return *as_int();
	if (type == TYPE_BOOL)
		return (int)*as_bool();
	if (type == TYPE_FLOAT)
		return (int)*as_float();
	if (type == TYPE_STRING)
		return as_string()->_int();
	throw Exception("can not interpret as int: " + type_name(type));
}

float Any::_float() const {
	if (type == TYPE_INT)
		return (float)*as_int();
	if (type == TYPE_FLOAT)
		return *as_float();
	if (type == TYPE_STRING)
		return as_string()->_float();
	throw Exception("can not interpret as float: " + type_name(type));
}

void Any::operator = (const Any &a) {
	if (&a != this) {
		bool b = parent;
		if (parent)
			any_db("=   IS REF " + str());
		create_type(a.type);
		if (a.type == TYPE_INT) {
			*as_int() = *a.as_int();
		} else if (a.type == TYPE_FLOAT) {
			*as_float() = *a.as_float();
		} else if (a.type == TYPE_BOOL) {
			*as_bool() = *a.as_bool();
		} else if (a.type == TYPE_POINTER) {
			*as_pointer() = *a.as_pointer();
		} else if (a.type == TYPE_STRING) {
			*as_string() = *a.as_string();
		} else if (a.type == TYPE_ARRAY) {
			*as_array() = *a.as_array();
		} else if (a.type == TYPE_MAP) {
			*as_map() = *a.as_map();
		} else if (a.type != TYPE_NONE) {
			clear();
			msg_error("any = any: " + type_name(a.type));
		}
		if (b)
			any_db(str());
	}
}

Any Any::operator + (const Any &a) const {
	if ((type == TYPE_INT) and (a.type == TYPE_INT))
		return _int() + a._int();
	if ((type == TYPE_FLOAT or type == TYPE_INT) and (a.type == TYPE_FLOAT or a.type == TYPE_INT))
		return _float() + a._float();
	if ((type == TYPE_STRING) and (a.type == TYPE_STRING))
		return str() + a.str();
	throw Exception(format("%s + %s not allowed", type_name(type), type_name(a.type)));
	return Any();
}

Any Any::operator - (const Any &a) const {
	if ((type == TYPE_INT) and (a.type == TYPE_INT))
		return _int() - a._int();
	if ((type == TYPE_FLOAT or type == TYPE_INT) and (a.type == TYPE_FLOAT or a.type == TYPE_INT))
		return _float() - a._float();
	throw Exception(format("%s - %s not allowed", type_name(type), type_name(a.type)));
	return Any();
}

void Any::operator += (const Any &a) {
	if ((type == TYPE_INT) and (a.type == TYPE_INT or a.type == TYPE_FLOAT))
		*as_int() += a._int();
	else if ((type == TYPE_FLOAT) and (a.type == TYPE_FLOAT or a.type == TYPE_INT))
		*as_float() += a._float();
	else if ((type == TYPE_STRING) and (a.type == TYPE_STRING))
		*as_string() += a.str();
	else if ((type == TYPE_ARRAY) and (a.type == TYPE_ARRAY))
		append(a);
	else if (type == TYPE_ARRAY)
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
	if (type == TYPE_NONE)
		create_type(TYPE_ARRAY);
	if (type == TYPE_ARRAY) {
		if (&a == this) {
			Any b = a;
			as_array()->add(b);
		} else {
			as_array()->add(a);
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
	if (a.type != TYPE_ARRAY)
		throw Exception("parameter not an array: " + type_name(a.type));
	if (type == TYPE_NONE)
		create_type(TYPE_ARRAY);
	if ((type == TYPE_ARRAY) and (a.type == TYPE_ARRAY)) {
		if (&a == this) {
			Any b = a;
			as_array()->append(*b.as_array());
		} else {
			as_array()->append(*a.as_array());
		}
	} else {
		throw Exception("not an array: " + type_name(type));
	}
}

int Any::length() {
	if (type == TYPE_ARRAY)
		return as_array()->num;
	if (type == TYPE_MAP)
		return as_map()->num;
	if (type == TYPE_STRING)
		return as_string()->num;
	return 0;
}

Any &Any::operator[] (int index) {
	if (type != TYPE_ARRAY)
		msg_error("only allowed for arrays: " + type_name(type));
	return (*as_array())[index];
}

const Any &Any::operator[] (int index) const {
	if (type != TYPE_ARRAY)
		msg_error("only allowed for arrays: " + type_name(type));
	return (*as_array())[index];
}

Any &Any::back() {
	if (type != TYPE_ARRAY)
		msg_error("only allowed for arrays: " + type_name(type));
	return as_array()->back();
}

const Any &Any::operator[] (const string &key) const {
	if (type != TYPE_MAP)
		msg_error("only allowed for maps: " + type_name(type));
	return (*as_map())[key];
}

Any &Any::operator[] (const string &key) {
	if (type == TYPE_NONE)
		create_type(TYPE_MAP);
	if (type != TYPE_MAP)
		msg_error("only allowed for maps: " + type_name(type));
	return (*as_map())[key];
}

Array<string> Any::keys() const {
	if (type != TYPE_MAP)
		return {};
	return as_map()->keys();
}

int* Any::as_int() const {
	return (int*)data;
}

float* Any::as_float() const {
	return (float*)data;
}

bool* Any::as_bool() const {
	return (bool*)data;
}

const void** Any::as_pointer() const {
	return (const void**)data;
}

string* Any::as_string() const {
	return (string*)data;
}

AnyMap* Any::as_map() const {
	return (AnyMap*)data;
}

Array<Any>* Any::as_array() const {
	return (Array<Any>*)data;
}

Any Any::array_get(int i) {
	if (type != TYPE_ARRAY)
		throw Exception("not an array: " + type_name(type));
	return (*this)[i].ref();
}

void Any::array_set(int i, const Any &value) {
	if (parent) {
		parent->array_set(i, value);
		sync_from_parent();
		return;
	}
	if (type == TYPE_NONE)
		create_type(TYPE_ARRAY);
	if (type != TYPE_ARRAY)
		throw Exception("not an array: " + type_name(type));
	(*this)[i] = value;
}

Any Any::map_get(const string &key) {
	if (type != TYPE_MAP)
		throw Exception("not a map: " + type_name(type));
	if (!as_map()->contains(key))
		throw Exception("key not found: " + key);
	return (*as_map())[key].ref();
}

void Any::map_set(const string &key, const Any &value) {
	if (parent) {
		parent->map_set(key, value);
		sync_from_parent();
		return;
	}
	if (type == TYPE_NONE)
		create_type(TYPE_MAP);
	if (type != TYPE_MAP)
		throw Exception("not a map: " + type_name(type));
	as_map()->set(key, value);
}

void Any::map_drop(const string &key) {
	if (parent) {
		parent->map_drop(key);
		sync_from_parent();
		return;
	}
	if (type != TYPE_MAP)
		throw Exception("not a map: " + type_name(type));
	as_map()->drop(key);
}
