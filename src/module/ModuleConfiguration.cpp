/*
 * ModuleConfiguration.cpp
 *
 *  Created on: 06.03.2019
 *      Author: michi
 */


//#include "../lib/kaba/kaba.h"
#include "ModuleConfiguration.h"
#include "Module.h"
#include "../lib/kaba/syntax/Class.h"
#include "../lib/kaba/syntax/SyntaxTree.h"
#include "../lib/os/msg.h"
#include "../data/SampleRef.h"
#include "../data/Sample.h"
#include "../data/Song.h"
#include "../Session.h"

namespace kaba {
	string find_enum_label(const Class *type, int value);
	int enum_parse(const string &label, const Class *type);
}

namespace tsunami {

bool module_config_debug = false;


void ModuleConfiguration::__init__() {
	new(this) ModuleConfiguration;
}

void ModuleConfiguration::__delete__() {
	this->ModuleConfiguration::~ModuleConfiguration();
}


Array<kaba::ClassElement> get_unique_elements(const kaba::Class *c) {
	Array<kaba::ClassElement> r;
	for (auto &e: c->elements)
		if (!e.hidden())
			r.add(e);
	return r;
}

Any var_to_any(const kaba::Class *c, const char *v) {
	if (c == kaba::TypeInt32) {
		return Any(*(const int*)v);
	} else if (c == kaba::TypeInt8) {
		return Any((int)*(const char*)v);
	} else if (c == kaba::TypeFloat32) {
		return Any(*(const float*)v);
	} else if (c == kaba::TypeBool) {
		return Any(*(const bool*)v);
	} else if (c == kaba::TypeString) {
		return Any(*(const string*)v);
	} else if (c->is_array()) {
		Any r = Any::EmptyList;
		auto tel = c->get_array_element();
		for (int i=0; i<c->array_length; i++)
			r.add(var_to_any(tel, &v[i * tel->size]));
		return r;
	} else if (c->is_list()) {
		Any r = Any::EmptyList;
		auto a = (DynamicArray*)v;
		auto tel = c->get_array_element();
		for (int i=0; i<a->num; i++)
			r.add(var_to_any(tel, (char*)a->simple_element(i)));
		return r;
	} else if (c->is_enum()) {
		/*auto l = kaba::find_enum_label(c, *(const int*)v);
		if (!str_is_integer(l))
			return Any(l);*/
		return Any(*(const int*)v);
	} else if (c->name == "shared[SampleRef]") {
		if (auto sr = *(SampleRef**)v)
			return Any("sample:" + i2h(sr->origin->uid, 4));
		return Any();
	} else if (c->is_product() or c == kaba::TypeComplex or c == kaba::TypeVec2 or c == kaba::TypeVec3 or c == kaba::TypeQuaternion or c == kaba::TypeColor) {
		// rect ...nope
		Any r = Any::EmptyList;
		for (auto &e: c->elements)
			if (!e.hidden())
				r.add(var_to_any(e.type, &v[e.offset]));
		return r;
	} else {
		Any r = Any::EmptyDict;
		for (auto &e: c->elements)
			if (!e.hidden())
				r.dict_set(e.name, var_to_any(e.type, &v[e.offset]));
		return r;
	}
	return Any();
}

string get_next(const string &var_temp, int &pos) {
	int start = pos;
	bool in_string = false;
	for (int i=start;i<var_temp.num;i++) {
		if ((i == start) and (var_temp[i] == '"')) {
			in_string = true;
		} else if (in_string) {
			if (var_temp[i] == '\\') {
				i ++;
			} else if (var_temp[i] == '"') {
				pos = i + 1;
				return var_temp.sub(start + 1, i).unescape();
			}
		} else if ((var_temp[i] == ' ') or (var_temp[i] == ']') or (var_temp[i] == ')') or (var_temp[i] == '[') or (var_temp[i] == '(')) {
			pos = i;
			return var_temp.sub(start, i);
		}
	}
	return var_temp.sub(start);
}

void var_from_string_legacy(const kaba::Class *type, char *v, const string &s, int &pos, Session *session) {
	if (pos >= s.num)
		return;
	if (type == kaba::TypeInt32) {
		*(int*)v = get_next(s, pos)._int();
	} else if (type == kaba::TypeInt8) {
		*(char*)v = get_next(s, pos)._int();
	} else if (type == kaba::TypeFloat32) {
		*(float*)v = get_next(s, pos)._float();
	} else if (type == kaba::TypeBool) {
		*(bool*)v = get_next(s, pos)._bool();
	} else if (type == kaba::TypeString) {
		*(string*)v = get_next(s, pos);
	} else if (type->is_array()) {
		auto tel = type->get_array_element();
		pos ++; // '['
		for (int i=0;i<type->array_length;i++) {
			if (i > 0)
				pos ++; // ' '
			var_from_string_legacy(tel, &v[i * tel->size], s, pos, session);
		}
		pos ++; // ']'
	} else if (type->is_list()) {
		pos ++; // '['
		auto *a = (DynamicArray*)v;
		auto tel = type->get_array_element();
		a->simple_clear(); // todo...
		while (true) {
			if ((s[pos] == ']') or (pos >= s.num))
				break;
			if (a->num > 0)
				pos ++; // ' '
			a->simple_resize(a->num + 1);
			var_from_string_legacy(tel, &(((char*)a->data)[(a->num - 1) * tel->size]), s, pos, session);
		}
		pos ++; // ']'
	} else if (type->is_enum()) {
		*(int*)v = get_next(s, pos)._int();
	} else if (type->name == "shared[SampleRef]") {
		string ss = get_next(s, pos);
		*(shared<SampleRef>*)v = nullptr;
		if ((ss != "nil") and session->song) {
			int n = ss._int();
			if ((n >= 0) and (n < session->song->samples.num)) {
				*(shared<SampleRef>*)v = new SampleRef(session->song->samples[n]);
			}
		}
	} else {
		auto e = get_unique_elements(type);
		pos ++; // '('
		for (int i=0; i<e.num; i++) {
			if (i > 0)
				pos ++; // ' '
			var_from_string_legacy(e[i].type, &v[e[i].offset], s, pos, session);
		}
		pos ++; // ')'
	}
}

int h2i(const string &h) {
	string s = string("\0\0\0\0", 4) + h.unhex().reverse();
	return *(int*)&s[s.num - 4];
}

void var_from_any(const kaba::Class *type, char *v, const Any &a, Session *session) {
	if (type == kaba::TypeInt32) {
		*(int*)v = a._int();
	} else if (type == kaba::TypeInt8) {
		*(char*)v = a._int();
	} else if (type == kaba::TypeFloat32) {
		*(float*)v = a._float();
	} else if (type == kaba::TypeBool) {
		*(bool*)v = a._bool();
	} else if (type == kaba::TypeString) {
		*(string*)v = a.str();
	} else if (type->is_array()) {
		if (!a.is_list())
			throw Exception("array expected");
		auto &array = a.as_list();
		auto tel = type->get_array_element();
		for (int i=0; i<min(type->array_length, array.num); i++)
			var_from_any(tel, &v[i * tel->size], array[i], session);
	} else if (type->is_list()) {
		if (!a.is_list())
			throw Exception("array expected");
		auto &array = a.as_list();
		auto *aa = (DynamicArray*)v;
		auto tel = type->get_array_element();
		aa->simple_resize(array.num); // todo...
		for (int i=0; i<array.num; i++)
			var_from_any(tel, &(((char*)aa->data)[i * tel->size]), array[i], session);
	} else if (type->is_enum()) {
		*(int*)v = kaba::enum_parse(a.str(), type);
	} else if (type->name == "shared[SampleRef]") {
		*(shared<SampleRef>*)v = nullptr;
		if (a.is_string()) {
			string ss = a.str();
			if ((ss.head(7) == "sample:") and session->song) {
				int uid = h2i(ss.sub(7));
				if (auto s = session->song->get_sample_by_uid(uid))
					*(shared<SampleRef>*)v = s->create_ref();
				else
					session->e(format("sample invalid: %s  %d  %8x", ss, uid, uid));
			}
		}
	} else {
		auto e = get_unique_elements(type);
		if (a.is_list()) {
			auto &aa = a.as_list();
			for (int i=0; i<min(e.num, aa.num); i++)
				var_from_any(e[i].type, &v[e[i].offset], aa[i], session);
		} else if (a.is_dict()) {
			for (auto &el: e)
				if (a.has(el.name))
					var_from_any(el.type, &v[el.offset], a[el.name], session);
		} else {
			throw Exception("array or map expected for " + type->long_name());
		}
	}
}

string ModuleConfiguration::to_string() const {
	return to_any().str();
	//return var_to_string(_class, (char*)this);
}

Any ModuleConfiguration::to_any() const {
	auto a = var_to_any(kaba_class, (const char*)this);
	if (module_config_debug)
		msg_write("to_any: " + a.str());
	return a;
}

void ModuleConfiguration::from_string(const string &s, Session *session) {
	//msg_write("from_string: " + s);
	try {
		from_any(Any::parse(s), session);
	} catch (Exception &e) {
		session->e(format("invalid configuration for module %s: %s", safe_module_name(), e.message()));
	}
}

void ModuleConfiguration::from_string_legacy(const string &s, Session *session) {
	Session::GLOBAL->w(format("legacy configuration for module %s: ", safe_module_name(), s));
	reset();
	int pos = 0;
	try  {
		var_from_string_legacy(kaba_class, (char*)this, s, pos, session);
	} catch (Exception &e) {
		session->e(e.message());
	}
	Session::GLOBAL->i("-> " + to_string());
}

string ModuleConfiguration::safe_module_name() const {
	if (_module)
		return _module->module_name;
	return "(unknown)";
}

string string_limit(const string &s, int n) {
	if (s.num <= n)
		return s;
	return s.head(n) + "...";
}

void ModuleConfiguration::from_any(const Any &a, Session *session) {
	reset();
	try {
		var_from_any(kaba_class, (char*)this, a, session);
	} catch (Exception &e) {
		session->e(format("invalid configuration for module %s: %s:   %s", safe_module_name(), e.message(), string_limit(a.str(), 256)));
	}

	if (module_config_debug)
		msg_write("from_any: " + a.str());
}

bool ac_name_match(const string &const_name, const string &var_name) {
	return (("AUTO_CONFIG_" + var_name).upper().replace("_", "") == const_name.upper().replace("_", ""));
}

string ModuleConfiguration::auto_conf(const string &name) const {
	if (!kaba_class)
		return "";
	auto *ps = kaba_class->owner;
	if (!ps)
		return "";
	for (auto c: weak(ps->base_class->constants)) {
		if (c->type == kaba::TypeString)
			if (ac_name_match(c->name, name))
				return c->as_string();
	}
	return "";
}

void ModuleConfiguration::changed() {
	if (_module)
		_module->changed();
}

}
