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

namespace Kaba{
class Script;
class Class;
};

namespace hui{
	class Panel;
}


class PluginData : public VirtualBase
{
public:
	virtual ~PluginData(){}
	void _cdecl __init__();
	virtual void _cdecl __delete__();
	virtual void _cdecl reset(){}
	Kaba::Class *_class;
};

//class AutoConfigPanel;
class ConfigPanel;
class Song;

class Configurable : public Observable<VirtualBase>
{
public:
	Configurable(int type);
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

	PluginData *get_config() const;
	PluginData *get_state() const;

	string configToString() const;
	void configFromString(const string &options);

	Configurable *copy() const;

	int configurable_type;
	string name;
	Song *song;


	enum
	{
		TYPE_EFFECT,
		TYPE_SYNTHESIZER,
		TYPE_MIDI_EFFECT,
	};
};

template<class T>
class ConfigurableMixin
{
public:
	ConfigurableMixin(int type);
	virtual ~ConfigurableMixin();
	void _cdecl __init__();
	virtual void _cdecl __delete__();

	static const string MESSAGE_CHANGE_BY_ACTION;

	void _cdecl resetConfig();
	void _cdecl resetState();
	bool configure();
	virtual ConfigPanel *_cdecl createPanel();
	void _cdecl notify();
	virtual void _cdecl onConfig(){}

	PluginData *get_config() const;
	PluginData *get_state() const;

	string configToString() const;
	void configFromString(const string &options);

	Configurable *copy() const;

	int configurable_type;
	string name;
	Song *song;


	enum
	{
		TYPE_EFFECT,
		TYPE_SYNTHESIZER,
		TYPE_MIDI_EFFECT,
	};
};


#endif /* CONFIGURABLE_H_ */
