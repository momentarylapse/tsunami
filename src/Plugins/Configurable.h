/*
 * Configurable.h
 *
 *  Created on: 05.01.2014
 *      Author: michi
 */

#ifndef CONFIGURABLE_H_
#define CONFIGURABLE_H_


#include "../lib/base/base.h"

namespace Script{
class Script;
class Type;
};


class PluginData : public VirtualBase
{
public:
	virtual ~PluginData(){}
	void __init__();
	virtual void __delete__();
	virtual void reset(){}
	Script::Type *type;
};

class Configurable : public VirtualBase
{
public:
	virtual ~Configurable(){}
	void __init__();
	virtual void __delete__();

	virtual void ResetConfig();
	virtual void ResetState();
	virtual void Configure(){};
	virtual void UpdateDialog(){};

	PluginData *get_config();
	PluginData *get_state();

	string ConfigToString();
	void ConfigFromString(const string &options);
};


#endif /* CONFIGURABLE_H_ */
