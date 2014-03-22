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

class HuiPanel;


class PluginData : public VirtualBase
{
public:
	virtual ~PluginData(){}
	void _cdecl __init__();
	virtual void _cdecl __delete__();
	virtual void _cdecl reset(){}
	Script::Type *type;
};

class Configurable : public VirtualBase
{
public:
	virtual ~Configurable(){}
	void __init__();
	virtual void __delete__();

	virtual void _cdecl ResetConfig();
	virtual void _cdecl ResetState();
	void Configure();
	virtual HuiPanel *_cdecl CreatePanel();
	virtual void _cdecl UpdateDialog(){};
	void _cdecl notify();

	PluginData *get_config();
	PluginData *get_state();

	string ConfigToString();
	void ConfigFromString(const string &options);

	string name;
};


#endif /* CONFIGURABLE_H_ */
