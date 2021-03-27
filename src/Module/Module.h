/*
 * Module.h
 *
 *  Created on: 05.01.2014
 *      Author: michi
 */

#ifndef SRC_MODULE_MODULE_H_
#define SRC_MODULE_MODULE_H_


#include "../lib/base/base.h"
#include "../lib/base/pointer.h"
#include "../Stuff/Observable.h"

namespace kaba {
	class Script;
	class Class;
};

namespace hui {
	class Panel;
}


class ModuleConfiguration;
class ConfigPanel;
class Session;
class Plugin;
class Port;
class InPortDescription;
class Any;

enum class ModuleCategory {
	// plug-ins
	AUDIO_SOURCE,
	AUDIO_EFFECT,
	MIDI_SOURCE,
	MIDI_EFFECT,
	SYNTHESIZER,
	BEAT_SOURCE,
	AUDIO_VISUALIZER,
	// other
	STREAM,
	PITCH_DETECTOR,
	PLUMBING,
	// recursion!
	SIGNAL_CHAIN,
	PORT_IN,
	PORT_OUT,
	// plug-in (not really Modules)
	SONG_PLUGIN,
	TSUNAMI_PLUGIN,
	// internal stuff
	OTHER,
};

enum class ModuleCommand {
	START,
	STOP,
	PREPARE_START,
	ACCUMULATION_START,
	ACCUMULATION_STOP,
	ACCUMULATION_CLEAR,
	ACCUMULATION_GET_SIZE,
	SUCK,
};

class Module : public Sharable<Observable<VirtualBase>> {
public:

	Module(ModuleCategory type, const string &sub_type);
	virtual ~Module();
	void _cdecl __init__(ModuleCategory type, const string &sub_type);
	void _cdecl __delete__() override;

	void set_session_etc(Session *session, const string &sub_type);

	static const string MESSAGE_STATE_CHANGE;
	static const string MESSAGE_READ_END_OF_STREAM;
	static const string MESSAGE_PLAY_END_OF_STREAM;
	static const string MESSAGE_TICK;


	// basic
	ModuleCategory module_category;
	string module_class;
	Session *session;
	float module_x, module_y;
	const kaba::Class *_class;

	Module *copy() const;

	bool enabled;
	bool belongs_to_system;
	
	int perf_channel;
	void perf_start();
	void perf_end();
	void perf_set_parent(Module *m);



	// config data
	bool allow_config_in_chain;
	void _cdecl reset_config();
	virtual ConfigPanel *_cdecl create_panel();
	void _cdecl changed();
	virtual void _cdecl on_config(){}

	virtual ModuleConfiguration *get_config() const;
	int version() const;
	static const int VERSION_LATEST = -1;
	static const int VERSION_LEGACY = -2;

	string config_to_string() const;
	Any config_to_any() const;
	void config_from_string(int version, const string &options);
	void config_from_any(int version, const Any &options);


	string _config_latest_history;
	void set_func_edit(std::function<void()> f);
	std::function<void()> func_edit;



	virtual void _cdecl reset_state() {}


	static const int COMMAND_NOT_HANDLED;
	virtual int command(ModuleCommand cmd, int param){ return COMMAND_NOT_HANDLED; }



	// ports
	Array<InPortDescription> port_in;
	Array<Port*> port_out;
	void _plug_in(int in_port, Module *source, int out_port);
	void _unplug_in(int in_port);




	static string category_to_name(ModuleCategory cat);
	static ModuleCategory category_from_name(const string &name);

	shared_array<Module> children;
};


#endif /* SRC_MODULE_MODULE_H_ */
