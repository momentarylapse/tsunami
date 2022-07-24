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
#include "../any/any.h"


class Path;
class Any;


class Configuration {
public:
	Configuration();
	~Configuration();
	void __init__();
	void __del__();

	void set_int(const string &name, int val);
	void set_float(const string &name, float val);
	void set_bool(const string &name, bool val);
	void set_str(const string &name, const string &val);
	void set_str_array(const string &name, const Array<string> &val);
	void set(const string &name, const Any &val);
	int get_int(const string &name, int default_val = 0) const;
	float get_float(const string &name, float default_val = 0) const;
	bool get_bool(const string &name, bool default_val = false) const;
	string get_str(const string &name, const string &default_val = "") const;
	Array<string> get_str_array(const string &name, const Array<string> &default_val = {}) const;
	Any get(const string &name, const Any &default_val = Any()) const;
	bool has(const string &name) const;
	bool load(const Path &filename);
	void save(const Path &filename);

	Array<string> keys() const;

	bool loaded, changed;
	Array<string> comments;
	Map<string, Any> map;
};


#endif /* HUICONFIG_H_ */
