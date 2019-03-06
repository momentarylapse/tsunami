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
	ModuleConfiguration(){ _class = nullptr; }
	virtual ~ModuleConfiguration(){}
	void _cdecl __init__();
	virtual void _cdecl __delete__();
	virtual void _cdecl reset(){}
	const Kaba::Class *_class;
};

class ConfigPanel;
class Session;
class Plugin;
class Port;
class InPortDescription;

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
	PORT_IN,
	PORT_OUT,
	// plug-in (not really Modules)
	SONG_PLUGIN,
	TSUNAMI_PLUGIN,
};

enum class ModuleCommand
{
	START,
	STOP,
	PAUSE,
	UNPAUSE,
	RESET_BUFFER
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
	static const string MESSAGE_STATE_CHANGE;
	static const string MESSAGE_READ_END_OF_STREAM;
	static const string MESSAGE_PLAY_END_OF_STREAM;
	static const string MESSAGE_TICK;

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
	virtual void command(ModuleCommand cmd){}

	bool allow_config_in_chain;


	Plugin *plugin;
	bool usable;
	bool enabled;
	string get_error();

	Array<InPortDescription> port_in;
	Array<Port*> port_out;
	void plug(int in_port, Module *source, int out_port);
	void unplug(int in_port);

	static string type_to_name(ModuleType type);
	static ModuleType type_from_name(const string &name);

	Array<Module*> children;
};


#endif /* SRC_MODULE_MODULE_H_ */
