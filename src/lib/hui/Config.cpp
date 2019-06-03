/*
 * HuiConfig.cpp
 *
 *  Created on: 01.03.2014
 *      Author: michi
 */

#include "Config.h"
#include "hui.h"

#ifdef OS_WINDOWS
#include <tchar.h>
#endif

namespace hui
{


#ifdef OS_WINDOWS
	int _tchar_str_size_(TCHAR *str);
#endif

Configuration Config;

Configuration::Configuration()
{
	filename = "";
	loaded = false;
	changed = false;
}

Configuration::Configuration(const string &_filename)
{
	filename = _filename;
	loaded = false;
	changed = false;
}

Configuration::~Configuration()
{
}

void Configuration::__init__()
{
	new(this) Configuration;
}

void Configuration::__init_ext__(const string &_filename)
{
	new(this) Configuration(_filename);
}

void Configuration::__del__()
{
	this->~Configuration();
}

void Configuration::set_int(const string& name, int val)
{
	set_str(name, i2s(val));
}

void Configuration::set_float(const string& name, float val)
{
	set_str(name, f2s(val, 6));
}

void Configuration::set_bool(const string& name, bool val)
{
	set_str(name, b2s(val));
}

void Configuration::set_str(const string& name, const string& str)
{
	map.set(name, str);
	changed = true;
}

int Configuration::get_int(const string& name, int default_val)
{
	return s2i(get_str(name, i2s(default_val)));
}

float Configuration::get_float(const string& name, float default_val)
{
	return s2f(get_str(name, f2s(default_val, 6)));
}

bool Configuration::get_bool(const string& name, bool default_val)
{
	return (get_str(name, b2s(default_val)) == "true");
}

string Configuration::get_str(const string& name, const string& default_str)
{
	if (!loaded)
		load();
	try{
		return map[name];
	}catch(...){
		return default_str;
	}
}

void Configuration::load()
{
	try{
		File *f = FileOpenText(filename);
		map.clear();
		f->read_comment();
		int num = f->read_int();
		for (int i=0;i<num;i++){
			string temp = f->read_str();
			string key = temp.substr(3, temp.num - 3);
			string value = f->read_str();
			map.set(key, value);
		}
		FileClose(f);
		loaded = true;
		changed = false;
	}catch(...){
	}
}

void Configuration::save()
{
	dir_create(filename.dirname());
	try{
		File *f = FileCreateText(filename);
		f->write_str("// NumConfigs");
		f->write_int(map.num);
		for (auto &e: map){
			f->write_str("// " + e.key);
			f->write_str(e.value);
		}
		f->write_str("#");
		FileClose(f);
		loaded = true;
		changed = false;
	}catch(...){}
}





};

