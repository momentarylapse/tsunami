/*
 * HuiConfig.h
 *
 *  Created on: 01.03.2014
 *      Author: michi
 */

#ifndef HUICONFIG_H_
#define HUICONFIG_H_

#include "../base/base.h"
#include "../base/map.h"

namespace hui
{

class Configuration {
public:
	Configuration();
	explicit Configuration(const string &filename);
	~Configuration();
	void _cdecl __init__();
	void _cdecl __init_ext__(const string &filename);
	void _cdecl __del__();

	void _cdecl set_int(const string &name, int val);
	void _cdecl set_float(const string &name, float val);
	void _cdecl set_bool(const string &name, bool val);
	void _cdecl set_str(const string &name, const string &str);
	int _cdecl get_int(const string &name, int default_val = 0);
	float _cdecl get_float(const string &name, float default_val = 0);
	bool _cdecl get_bool(const string &name, bool default_val = false);
	string _cdecl get_str(const string &name, const string &default_str);
	void _cdecl load();
	void _cdecl save();

	bool loaded, changed;
	string filename;
	Map<string, string> map;
};

extern Configuration Config;



};

#endif /* HUICONFIG_H_ */
