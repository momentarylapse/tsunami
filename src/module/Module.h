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
#include "../lib/base/optional.h"
#include "../lib/pattern/Observable.h"

namespace kaba {
	class Script;
	class Class;
};

namespace hui {
	class Panel;
}

class Any;

namespace tsunami {

class ModuleConfiguration;
class ConfigPanel;
class Session;
class Plugin;
struct OutPort;
struct InPort;
class AudioBuffer;
class MidiEventBuffer;
class Beat;

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
	SET_INPUT_CHANNELS,
	SAMPLE_COUNT_MODE,
	GET_SAMPLE_COUNT,
};

enum class SampleCountMode {
	NONE,
	CONSUMER,
	PRODUCER,
	TRANSLATOR
};

class Module : public Sharable<obs::Node<VirtualBase>> {
public:

	Module(ModuleCategory category, const string &_class);
	virtual ~Module();
	void _cdecl __init__(ModuleCategory category, const string &_class);
	void _cdecl __delete__() override;

	void set_session_etc(Session *session, const string &_class);

	obs::source out_state_changed{this, "state-changed"};
	obs::source out_read_end_of_stream{this, "read-end-of-stream"};
	obs::source out_play_end_of_stream{this, "play-end-of-stream"};
	obs::source out_tick{this, "tick"};


	// basic
	ModuleCategory module_category;
	string module_class;
	string module_name;
	Session *session;
	float module_x, module_y;
	const kaba::Class *kaba_class;

	xfer<Module> copy() const;

	bool enabled;
	bool belongs_to_system;
	
	int perf_channel;
	void perf_start();
	void perf_end();
	void perf_set_parent(Module *m);



	// config data
	bool allow_config_in_chain;
	void _cdecl reset_config();
	virtual xfer<ConfigPanel> create_panel();
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


	virtual base::optional<int64> command(ModuleCommand cmd, int64 param) { return base::None; }



	static const int END_OF_STREAM;
	static const int NOT_ENOUGH_DATA;
	static const int NO_SOURCE;

	// ports
	Array<InPort*> port_in;
	Array<OutPort*> port_out;

	virtual int read_audio(int port, AudioBuffer &buf){ return 0; }
	virtual int read_midi(int port, MidiEventBuffer &midi){ return 0; };
	virtual int read_beats(int port, Array<Beat> &beats, int samples){ return 0; };




	static string category_to_str(ModuleCategory cat);
	static ModuleCategory category_from_str(const string &str);

	shared_array<Module> children;
};

}


#endif /* SRC_MODULE_MODULE_H_ */
