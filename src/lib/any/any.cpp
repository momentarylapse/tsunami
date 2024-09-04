#include "any.h"
#include "../base/map.h"
#include "../os/msg.h"


string f2s_clean(float f, int dez);

class AnyDict : public base::map<string, Any> {};

AnyDict _empty_dummy_map_;
static DynamicArray _empty_dummy_array_ = {NULL, 0, 0, sizeof(Any)};

Any EmptyVar;
//Any EmptyMap = _empty_dummy_map_;
Any Any::EmptyList = *(Array<Any>*)&_empty_dummy_array_;
Any Any::EmptyDict = Any(_empty_dummy_map_);
bool Any::allow_simple_output = true;

//#define any_db(m)	msg_write(m)
#define any_db(m)


static string type_name(Any::Type t) {
	if (t == Any::Type::None)
		return "-none-";
	if (t == Any::Type::Int)
		return "int";
	if (t == Any::Type::Float)
		return "float";
	if (t == Any::Type::Bool)
		return "bool";
	if (t == Any::Type::String)
		return "string";
	if (t == Any::Type::List)
		return "list";
	if (t == Any::Type::Dict)
		return "dict";
	if (t == Any::Type::Pointer)
		return "pointer";
	return "-unknown type-";
}

Any::Any() {
	type = Type::None;
	data = nullptr;
	parent = nullptr;
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
	create_type(Type::Int);
	as_int() = i;
}

Any::Any(float f) : Any() {
	create_type(Type::Float);
	as_float() = f;
}

Any::Any(bool b) : Any() {
	create_type(Type::Bool);
	as_bool() = b;
}

Any::Any(const string &s) : Any() {
	create_type(Type::String);
	as_string() = s;
}

Any::Any(const char *s) : Any(string(s)) {}

Any::Any(const void *p) : Any() {
	create_type(Type::Pointer);
	as_pointer() = p;
}

Any::Any(const Array<Any> &a) : Any() {
	create_type(Type::List);
	as_list() = a;
}

Any::Any(const Array<int> &a) : Any() {
	create_type(Type::List);
	for (int i: a)
		as_list().add(i);
}

Any::Any(const AnyDict &m) : Any() {
	create_type(Type::Dict);
	as_dict() = m;
}

Any Any::ref() {
	Any r;
	r.parent = this;
	r.type = type;
	r.data = data;
	any_db(format("ref  %s -> %s:   %s", p2s(this), p2s(parent), str()));
	return r;
}

void Any::create_type(Type _type) {
	if (parent) {
		any_db(format("parent create  %s -> %s:   %s", p2s(this), p2s(parent), str()));
		parent->create_type(_type);
		sync_from_parent();
		return;
	}
	clear();
	type = _type;
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
	} else if (is_list()) {
		data = new Array<Any>;
	} else if (is_dict()) {
		data = new AnyDict;
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
		parent->sync_to_parent();
	}
}

void Any::sync_from_parent() {
	if (parent) {
		any_db(format("sync  %s << %s:   %s", p2s(this), p2s(parent), parent->str()));
		parent->sync_from_parent();
		data = parent->data;
		type = parent->type;
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
		else if (is_list())
			delete &as_list();
		else if (is_dict())
			delete &as_dict();
		else if (is_pointer())
			delete &as_pointer();
		else if (!is_empty())
			msg_error("any.clear(): " + type_name(type));
		type = Type::None;
		data = nullptr;
	}
}

bool Any::is_empty() const {
	return type == Type::None;
}

bool Any::is_int() const {
	return type == Type::Int;
}

bool Any::is_float() const {
	return type == Type::Float;
}

bool Any::is_bool() const {
	return type == Type::Bool;
}

bool Any::is_string() const {
	return type == Type::String;
}

bool Any::is_pointer() const {
	return type == Type::Pointer;
}

bool Any::is_list() const {
	return type == Type::List;
}

bool Any::is_dict() const {
	return type == Type::Dict;
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
	} else if (is_list()) {
		string s = "[";
		for (Any &p: as_list()) {
			if (s.num > 1)
				s += ", ";
			s += p.repr();
		}
		return s + "]";
	} else if (is_dict()) {
		string s = "{";
		for (auto&& [k,v]: as_dict()) {
			if (s.num > 1)
				s += ", ";
			s += minimal_key_repr(k) + ": " + v.repr();
		}
		return s + "}";
	} else if (is_empty()) {
		return "nil";
	} else {
		return "unhandled Any.str(): " + type_name(type);
	}
}

string Any::str() const {
	if (is_empty())
		return "";
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
		a.create_type(Any::Type::List);
		pos ++;
		while (tokens[pos] != "]") {
			if (tokens[pos] == ",") {
				a.add(Any());
				pos ++;
				continue;
			}
			a.as_list().resize(a.as_list().num + 1);
			any_parse_part(a.as_list().back(), tokens, pos);
			if (tokens[pos] == "]")
				break;
			expect_token(",");
		}
		pos ++;
	} else if (cur == "{") {
		a.create_type(Any::Type::Dict);
		pos ++;
		while (tokens[pos] != "}") {
			Any key;
			any_parse_part(key, tokens, pos);
			if (key.type != Any::Type::String)
				throw Exception("only strings allowed as dict keys");
			expect_token(":");
			a.as_dict().set(key.as_string(), Any());
			any_parse_part(a.as_dict()[key.as_string()], tokens, pos);
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
			a.create_type(Any::Type::Float);
			a.as_float() = cur._float();
		} else {
			a.create_type(Any::Type::Int);
			a.as_int() = cur._int();
		}
		pos ++;
	} else if (cur == "true" or cur == "false") {
		a.create_type(Any::Type::Bool);
		a.as_bool() = cur._bool();
		pos ++;
	} else if (cur[0] == '\"' or cur[0] == '\'') {
		a.create_type(Any::Type::String);
		a.as_string() = cur.sub(1, -1).unescape();
		pos ++;
	} else {
		// token (string without quotes)
		a.create_type(Any::Type::String);
		a.as_string() = cur;
		pos ++;
		//throw Exception(format("what is '%s'?", cur));
	}
};

Any Any::parse(const string &s) {
	auto tokens = s.parse_tokens(",:[](){}\"");
	if (tokens.num == 0)
		return Any();
	int pos = 0;

	Any r;
	any_parse_part(r, tokens, pos);
	if ((r.is_string() or r.is_int() or r.is_bool() or r.is_float()) and tokens.num > 1 and s[0] != '\"')
		return Any(s.trim());
	return r;
}

bool Any::_bool() const {
	if (is_bool())
		return as_bool();
	if (is_int())
		return as_int() != 0;
	return false;
	//throw Exception("can not interpret as bool: " + type_name(type));
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
	return 0;
	//throw Exception("can not interpret as int: " + type_name(type));
}

float Any::_float() const {
	if (is_int())
		return (float)as_int();
	if (is_float())
		return as_float();
	if (is_string())
		return as_string()._float();
	return 0;
	//throw Exception("can not interpret as float: " + type_name(type));
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
		} else if (a.is_list()) {
			as_list() = a.as_list();
		} else if (a.is_dict()) {
			as_dict() = a.as_dict();
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
	else if (is_list() and a.is_list())
		append(a);
	else if (is_list())
		add(a);
	else
		throw Exception(format("%s += %s not allowed", type_name(type), type_name(a.type)));
}

bool Any::operator == (const Any& a) const {
	if (type != a.type)
		return false;
	if (is_int())
		return as_int() == a.as_int();
	if (is_float())
		return as_float() == a.as_float();
	if (is_bool())
		return as_bool() == a.as_bool();
	if (is_string())
		return as_string() == a.as_string();
	if (is_list())
		return as_list() == a.as_list();
	if (is_dict())
		return false;//as_dict() == a.as_dict();
	return false;
}

bool Any::operator != (const Any& a) const {
	return !(*this == a);
}

void Any::add(const Any &a) {
	if (parent) {
		parent->add(a);
		sync_from_parent();
		any_db(format("parent add  %s -> %s:   %s", p2s(this), p2s(parent), str()));
		return;
	}
	if (!is_list())
		create_type(Type::List);
	if (&a == this) {
		Any b = a;
		as_list().add(b);
	} else {
		as_list().add(a);
	}
}

// TODO: map.append(map)
void Any::append(const Any &a) {
	if (parent) {
		parent->append(a);
		sync_from_parent();
		return;
	}
	if (!a.is_list())
		throw Exception("parameter not an array: " + type_name(a.type));
	if (is_empty())
		create_type(Type::List);
	if (is_list() and a.is_list()) {
		if (&a == this) {
			Any b = a;
			as_list().append(b.as_list());
		} else {
			as_list().append(a.as_list());
		}
	} else {
		throw Exception("not an array: " + type_name(type));
	}
}

int Any::length() const {
	if (is_list())
		return as_list().num;
	if (is_dict())
		return as_dict().num;
	if (is_string())
		return as_string().num;
	return 0;
}

Any &Any::operator[] (int index) {
	if (!is_list())
		msg_error("only allowed for arrays: " + type_name(type));
	return as_list()[index];
}

const Any &Any::operator[] (int index) const {
	if (!is_list())
		msg_error("only allowed for arrays: " + type_name(type));
	return as_list()[index];
}

Any &Any::back() {
	if (!is_list())
		msg_error("only allowed for arrays: " + type_name(type));
	return as_list().back();
}

const Any &Any::operator[] (const string &key) const {
	if (!is_dict())
		msg_error("only allowed for maps: " + type_name(type));
	return as_dict()[key];
}

Any &Any::operator[] (const string &key) {
	if (is_empty())
		create_type(Type::Dict);
	if (!is_dict())
		msg_error("only allowed for maps: " + type_name(type));
	if (!as_dict().contains(key))
		as_dict().set(key, Any());
	return as_dict()[key];
}

Array<string> Any::keys() const {
	if (!is_dict())
		return {};
	return as_dict().keys();
}

bool Any::has(const string &key) const {
	if (!is_dict())
		return false;
	return sa_contains(as_dict().keys(), key);
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

AnyDict& Any::as_dict() const {
	return *(AnyDict*)data;
}

Array<Any>& Any::as_list() const {
	return *(Array<Any>*)data;
}

base::optional<Any*> Any::list_get(int i) {
	if (!is_list())
		return base::None;
	if (i < 0 or i >= as_list().num)
		return base::None;
	return &(*this)[i];
}

void Any::list_set(int i, const Any &value) {
	if (i < 0)
		return;
	if (parent) {
		parent->list_set(i, value);
		sync_from_parent();
		return;
	}
	if (!is_list())
		create_type(Type::List);
	if (as_list().num <= i)
		as_list().resize(i + 1);
	(*this)[i] = value;
}

base::optional<Any*> Any::dict_get(const string &key) {
	if (!is_dict())
		return base::None;
	if (!as_dict().contains(key))
		return base::None;
	return &as_dict()[key];
}

void Any::dict_set(const string &key, const Any &value) {
	if (parent) {
		parent->dict_set(key, value);
		sync_from_parent();
		return;
	}
	if (!is_dict())
		create_type(Type::Dict);
	as_dict().set(key, value);
}

void Any::dict_drop(const string &key) {
	if (parent) {
		parent->dict_drop(key);
		sync_from_parent();
		return;
	}
	if (is_dict())
		as_dict().drop(key);
}

template<> string repr(const Any& a) {
	return a.repr();
}
