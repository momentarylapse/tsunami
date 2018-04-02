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
class Session;
class Song;
class Plugin;

class Configurable : public Observable<VirtualBase>
{
public:
	Configurable(Session *session, int type);
	virtual ~Configurable();
	void _cdecl __init__();
	virtual void _cdecl __delete__();

	static const string MESSAGE_CHANGE_BY_ACTION;

	void _cdecl reset_config();
	void _cdecl reset_state();
	bool configure(hui::Window *win);
	virtual ConfigPanel *_cdecl create_panel();
	void _cdecl notify();
	virtual void _cdecl on_oonfig(){}

	PluginData *get_config() const;
	PluginData *get_state() const;

	string config_to_string() const;
	void config_from_string(const string &options);

	Configurable *copy() const;

	int configurable_type;
	string name;
	Session *session;
	Song *song;


	Plugin *plugin;
	bool usable;
	bool enabled;
	string getError();


	enum Type
	{
		AUDIO_SOURCE,
		AUDIO_EFFECT,
		MIDI_SOURCE,
		MIDI_EFFECT,
		SYNTHESIZER,
		BEAT_SOURCE,
	};

	static string type_to_name(int type);
	static Type type_from_name(const string &name);
};

template<class T>
class ConfigurableMixin
{
public:
	ConfigurableMixin(Session *session, int type);
	virtual ~ConfigurableMixin();
	void _cdecl __init__();
	virtual void _cdecl __delete__();

	static const string MESSAGE_CHANGE_BY_ACTION;

	void _cdecl reset_config();
	void _cdecl reset_state();
	bool configure();
	virtual ConfigPanel *_cdecl create_panel();
	void _cdecl notify();
	virtual void _cdecl on_config(){}

	PluginData *get_config() const;
	PluginData *get_state() const;

	string config_to_string() const;
	void config_from_string(const string &options);

	Configurable *copy() const;

	int configurable_type;
	string name;
	Session *session;
	Song *song;


	enum Type
	{
		AUDIO_SOURCE,
		AUDIO_EFFECT,
		MIDI_SOURCE,
		MIDI_EFFECT,
		SYNTHESIZER,
		BEAT_SOURCE,
	};
};


#endif /* CONFIGURABLE_H_ */
