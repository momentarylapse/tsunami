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

typedef MapEntry<string, string> HuiConfigEntry;

class HuiConfiguration
{
public:
	HuiConfiguration();
	HuiConfiguration(const string &filename);
	~HuiConfiguration();
	void _cdecl __init__();
	void _cdecl __init_ext__(const string &filename);
	void _cdecl __del__();

	void _cdecl setInt(const string &name, int val);
	void _cdecl setFloat(const string &name, float val);
	void _cdecl setBool(const string &name, bool val);
	void _cdecl setStr(const string &name, const string &str);
	int _cdecl getInt(const string &name, int default_val = 0);
	float _cdecl getFloat(const string &name, float default_val = 0);
	bool _cdecl getBool(const string &name, bool default_val = false);
	string _cdecl getStr(const string &name, const string &default_str);
	void _cdecl load();
	void _cdecl save();

	bool loaded, changed;
	string filename;
	Map<string, string> map;
};

extern HuiConfiguration HuiConfig;



void _cdecl HuiRegisterFileType(const string &ending, const string &description, const string &icon_path, const string &open_with, const string &command_name, bool set_default);

#endif /* HUICONFIG_H_ */
