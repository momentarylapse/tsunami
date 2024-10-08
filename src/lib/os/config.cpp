/*
 * HuiConfig.cpp
 *
 *  Created on: 01.03.2014
 *      Author: michi
 */

#include "config.h"
#include "../os/file.h"
#include "../os/filesystem.h"
#include "../os/formatter.h"
#include "../os/msg.h"
#include "../any/any.h"



Configuration::Configuration() {
	loaded = false;
	changed = false;
}

Configuration::~Configuration() {
}

void Configuration::__init__() {
	new(this) Configuration;
}

void Configuration::__del__() {
	this->~Configuration();
}

void Configuration::set(const string& name, const Any& val) {
	map.set(name, val);
	changed = true;
}

void Configuration::set_int(const string& name, int val) {
	set(name, Any(val));
}

void Configuration::set_float(const string& name, float val) {
	set(name, Any(val));
}

void Configuration::set_bool(const string& name, bool val) {
	set(name, Any(val));
}

void Configuration::set_str(const string& name, const string& val) {
	set(name, Any(val));
}

void Configuration::set_str_array(const string &name, const Array<string> &val) {
	Any a = Any::EmptyList;
	for (auto &s: val)
		a.add(Any(s));
	set(name, a);
}

Any Configuration::get(const string& name, const Any& default_val) const {
	if (has(name))
		return map[name];
	else
		return default_val;
}

int Configuration::get_int(const string& name, int default_val) const {
	return get(name, Any(default_val))._int();
}

float Configuration::get_float(const string& name, float default_val) const {
	return get(name, Any(default_val))._float();
}

bool Configuration::get_bool(const string& name, bool default_val) const {
	return get(name, Any(default_val))._bool();
}

string Configuration::get_str(const string& name, const string& default_val) const {
	auto a = get(name, Any(default_val));
	if (a.is_empty())
		return "";
	return a.str();
}

Array<string> Configuration::get_str_array(const string &name, const Array<string> &default_val) const {
	auto a = get(name, Any::EmptyList);
	if (a.is_empty())
		return {};
	if (a.is_string())
		return {a.str()};
	Array<string> r;
	for (auto &s: a.as_list()) {
		if (s.is_empty())
			r.add("");
		else
			r.add(s.str());
	}
	return r;
}

bool Configuration::has(const string& name) const {
	return map.find(name) >= 0;
}

/*static string _parse_value(const string &s) {
	string r = s.trim();
	if (r.head(1) == "\"" and r.tail(1) == "\"")
		return r.sub(1, -1).unescape();
	if (r.head(1) == "\'" and r.tail(1) == "\'")
		return r.sub(1, -1).replace("\\\'", "\'");
	return r;
}*/

bool Configuration::load(const Path &filename) {
	try {
		auto f = os::fs::open(filename, "rt");
		map.clear();

		string t = f->read_str();
		if (t == "// NumConfigs") {
			// old format
			int num = f->read_int();
			for (int i=0;i<num;i++) {
				string temp = f->read_str();
				string key = temp.sub(3);
				string value = f->read_str();
				map.set(key, value);
			}
		} else if (t.head(3) == "// ") {
			// semi old format
			f->set_pos(0);
			while (!f->is_end()) {
				string temp = f->read_str();
				if (temp == "#")
					break;
				string key = temp.sub(3).lower().replace(" ", "-");
				string value = f->read_str();
				map.set(key, value);
			}
		} else {
			// new format
			f->set_pos(0);
			string _namespace;
			while (!f->is_end()) {
				string s = f->read_str();
				if (s.num == 0)
					continue;

				if (s[0] == '#') {
					comments.add(s);
				} else if (s[0] == '[') {
					_namespace = s.replace("[", "").replace("]", "").trim();
				} else {
					int p = s.find("=");
					if (p >= 0) {
						string key = s.head(p).trim();
						if (_namespace.num > 0)
							key = _namespace + "." + key;
						auto value = Any::parse(s.sub(p+1)); //_parse_value(s.sub(p+1));
						map.set(key, value);
					}
				}
			}
		}
//		for (auto &k: map.keys())
//			msg_write("config:  " + k + " := " + map[k]);
		delete f;
		loaded = true;
		changed = false;
		return true;
	} catch(Exception &e) {
		return false;
	}
}

/*bool string_needs_quotes(const string &v) {
	if (v.num == 0)
		return false;

	if (v.head(1) == " " or v.tail(1) == " ")
		return true;
	if (v.has_char('\"') or v.has_char('\t') or v.has_char('\n'))
		return true;
	return false;
}

string str_simple_repr(const string &v) {
	if (!string_needs_quotes(v))
		return v;
	int mode = 2;
	if (v.has_char('\t') or v.has_char('\n'))
		mode = 1;
	if (v.has_char('\''))
		mode = 1;

	if (mode == 1)
		return v.repr();
	return "'" + v.replace("'", "\\'") + "'";
}*/

string config_get_namespace(const string &key) {
	int p = key.rfind(".");
	if (p >= 0)
		return key.sub_ref(0, p);
	return "";
}

string config_get_base(const string &key) {
	int p = key.rfind(".");
	if (p >= 0)
		return key.sub_ref(p + 1);
	return key;
}

void Configuration::save(const Path &filename) {
	os::fs::create_directory(filename.parent());
	try {
		auto f = os::fs::open(filename, "wt");
		base::set<string> namespaces;
		for (auto&& [k,v]: map)
			namespaces.add(config_get_namespace(k));
		for (auto&& [key,value]: map)
			if (config_get_namespace(key) == "")
				f->write_str(format("%s = %s", key, value.repr()));
		for (auto &n: namespaces)
			if (n.num > 0) {
				f->write_str(format("\n[%s]", n));
				for (auto&& [key,value]: map)
					if (config_get_namespace(key) == n)
						f->write_str(format("\t%s = %s", config_get_base(key), value.repr()));
			}

		if (comments.num > 0)
			f->write_str("");
		for (auto &s: comments)
			f->write_str(s);
		delete f;
		loaded = true;
		changed = false;
	} catch(Exception &e) {
		msg_error(e.message());
	}
}

Array<string> Configuration::keys() const {
	return map.keys();
}

void Configuration::migrate(const string& from_key, const string& to_key) {
	if (has(from_key)) {
		if (has(to_key))
			map[to_key] = map[from_key];
		remove(from_key);
	}
}

void Configuration::remove(const string& key) {
	map.drop(key);
}


