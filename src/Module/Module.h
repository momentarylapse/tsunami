/*
 * Module.h
 *
 *  Created on: 05.01.2014
 *      Author: michi
 */

#ifndef SRC_MODULE_MODULE_H_
#define SRC_MODULE_MODULE_H_


#include "../lib/base/base.h"
#include "../Stuff/Observable.h"

namespace Kaba{
	class Script;
	class Class;
};

namespace hui{
	class Panel;
}


class ModuleConfiguration : public VirtualBase
{
public:
	virtual ~ModuleConfiguration(){}
	void _cdecl __init__();
	virtual void _cdecl __delete__();
	virtual void _cdecl reset(){}
	Kaba::Class *_class;
};

class ConfigPanel;
class Session;
class Plugin;
class PortDescription;

class Module : public Observable<VirtualBase>
{
public:
	Module(Session *session, int type);
	virtual ~Module();
	void _cdecl __init__();
	virtual void _cdecl __delete__();

	static const string MESSAGE_CHANGE_BY_ACTION;

	void _cdecl reset_config();
	virtual void _cdecl reset_state(){}
	bool configure(hui::Window *win);
	virtual ConfigPanel *_cdecl create_panel();
	void _cdecl notify();
	virtual void _cdecl on_config(){}

	ModuleConfiguration *get_config() const;

	string config_to_string() const;
	void config_from_string(const string &options);

	Module *copy() const;

	int module_type;
	string name;
	Session *session;


	Plugin *plugin;
	bool usable;
	bool enabled;
	string get_error();

	Array<PortDescription> port_in, port_out;

	enum Type
	{
		AUDIO_SOURCE,
		AUDIO_EFFECT,
		MIDI_SOURCE,
		MIDI_EFFECT,
		SYNTHESIZER,
		BEAT_SOURCE,
		OUTPUT_STREAM_AUDIO,
		INPUT_STREAM_AUDIO,
		INPUT_STREAM_MIDI,
		PITCH_DETECTOR,
		AUDIO_JOINER,
		BEAT_MIDIFIER,
		PEAK_METER,
		AUDIO_SUCKER,
	};

	enum SignalType
	{
		AUDIO,
		MIDI,
		BEATS
	};

	static string type_to_name(int type);
	static Type type_from_name(const string &name);
};


#endif /* SRC_MODULE_MODULE_H_ */
