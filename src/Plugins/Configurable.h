/*
 * Configurable.h
 *
 *  Created on: 05.01.2014
 *      Author: michi
 */

#ifndef CONFIGURABLE_H_
#define CONFIGURABLE_H_


#include "../lib/base/base.h"
#include "../Stuff/Observable.h"

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

//class AutoConfigPanel;
class ConfigPanel;

class Configurable : public Observable
{
public:
	Configurable(const string &observable_name, int type);
	virtual ~Configurable();
	void _cdecl __init__();
	virtual void _cdecl __delete__();

	static const string MESSAGE_CHANGE_BY_ACTION;

	void _cdecl resetConfig();
	void _cdecl resetState();
	bool configure();
	virtual ConfigPanel *_cdecl createPanel();
	void _cdecl notify();
	virtual void _cdecl onConfig(){}

	PluginData *get_config();
	PluginData *get_state();

	string configToString();
	void configFromString(const string &options);

	int configurable_type;
	string name;


	enum
	{
		TYPE_EFFECT,
		TYPE_SYNTHESIZER,
		TYPE_MIDI_EFFECT,
	};
};


#endif /* CONFIGURABLE_H_ */
