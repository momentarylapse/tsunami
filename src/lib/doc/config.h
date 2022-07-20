/*
 * config.h
 *
 *  Created on: 01.03.2014
 *      Author: michi
 */

#ifndef HUICONFIG_H_
#define HUICONFIG_H_

#include "../base/base.h"
#include "../base/map.h"


class Path;
class Any;


class Configuration {
public:
	Configuration();
	~Configuration();
	void _cdecl __init__();
	void _cdecl __del__();

	void _cdecl set_int(const string &name, int val);
	void _cdecl set_float(const string &name, float val);
	void _cdecl set_bool(const string &name, bool val);
	void _cdecl set_str(const string &name, const string &val);
	void _cdecl set(const string &name, const Any &val);
	int _cdecl get_int(const string &name, int default_val = 0) const;
	float _cdecl get_float(const string &name, float default_val = 0) const;
	bool _cdecl get_bool(const string &name, bool default_val = false) const;
	string _cdecl get_str(const string &name, const string &default_str) const;
	Any get(const string &name, const Any &default_val) const;
	bool _cdecl has(const string &name) const;
	bool _cdecl load(const Path &filename);
	void _cdecl save(const Path &filename);

	Array<string> keys() const;

	bool loaded, changed;
	Array<string> comments;
	Map<string, Any> map;
};


#endif /* HUICONFIG_H_ */
