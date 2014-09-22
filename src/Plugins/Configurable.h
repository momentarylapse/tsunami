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

enum
{
	CONFIGURABLE_EFFECT,
	CONFIGURABLE_SYNTHESIZER,
	CONFIGURABLE_MIDI_EFFECT,
};

class AutoConfigPanel;

class Configurable : public Observable
{
public:
	Configurable(const string &observable_name, int type);
	virtual ~Configurable();
	void _cdecl __init__();
	virtual void _cdecl __delete__();

	static const string MESSAGE_CHANGE_BY_ACTION;

	virtual void _cdecl ResetConfig();
	virtual void _cdecl ResetState();
	bool Configure();
	virtual HuiPanel *_cdecl CreatePanel();
	virtual void _cdecl UpdateDialog();
	void _cdecl notify();

	PluginData *get_config();
	PluginData *get_state();

	string ConfigToString();
	void ConfigFromString(const string &options);

	string name;
	int configurable_type;
	AutoConfigPanel *_auto_panel_;
};


#endif /* CONFIGURABLE_H_ */
