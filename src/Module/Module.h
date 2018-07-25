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
	ModuleConfiguration(){ _class = NULL; }
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

enum class ModuleType
{
	// plug-ins
	AUDIO_SOURCE,
	AUDIO_EFFECT,
	MIDI_SOURCE,
	MIDI_EFFECT,
	SYNTHESIZER,
	BEAT_SOURCE,
	OUTPUT_STREAM_AUDIO,
	INPUT_STREAM_AUDIO,
	INPUT_STREAM_MIDI,
	AUDIO_VISUALIZER,
	// other
	PITCH_DETECTOR,
	AUDIO_JOINER,
	BEAT_MIDIFIER,
	AUDIO_SUCKER,
	// recursion!
	SIGNAL_CHAIN,
	// plug-in (not really Modules)
	SONG_PLUGIN,
	TSUNAMI_PLUGIN,
};

class Module : public Observable<VirtualBase>
{
public:

	Module(ModuleType type);
	virtual ~Module();
	void _cdecl __init__(ModuleType type);
	virtual void _cdecl __delete__();

	void set_session_etc(Session *session, const string &sub_type, Plugin *plugin);

	static const string MESSAGE_CHANGE_BY_ACTION;

	void _cdecl reset_config();
	virtual void _cdecl reset_state(){}
	bool configure(hui::Window *win);
	virtual ConfigPanel *_cdecl create_panel();
	void _cdecl changed();
	virtual void _cdecl on_config(){}

	virtual ModuleConfiguration *get_config() const;

	virtual string config_to_string() const;
	virtual void config_from_string(const string &options);

	Module *copy() const;

	ModuleType module_type;
	string module_subtype;
	Session *session;

	float module_x, module_y;
	virtual void module_start(){}
	virtual void module_stop(){}
	virtual void module_pause(bool paused);

	bool allow_config_in_chain;


	Plugin *plugin;
	bool usable;
	bool enabled;
	string get_error();

	Array<PortDescription> port_in, port_out;

	static string type_to_name(ModuleType type);
	static ModuleType type_from_name(const string &name);
};


#endif /* SRC_MODULE_MODULE_H_ */
