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
	AudioSource,
	AudioEffect,
	MidiSource,
	MidiEffect,
	Synthesizer,
	BeatSource,
	AudioVisualizer,
	// other
	Stream,
	PitchDetector,
	Plumbing,
	// recursion!
	SignalChain,
	PortIn,
	PortOut,
	// plug-in (not really Modules)
	SongPlugin,
	TsunamiPlugin,
	// internal stuff
	Other,
};

enum class ModuleCommand {
	Start,
	Stop,
	PrepareStart,
	AccumulationStart,
	AccumulationStop,
	AccumulationClear,
	AccumulationGetSize,
	Suck,
	SetInputChannels,
	SampleCountMode,
	GetSampleCount,
};

enum class SampleCountMode {
	None,
	Consumer,
	Producer,
	Translator
};

class Module : public Sharable<obs::Node<VirtualBase>> {
public:
	Module(ModuleCategory category, const string &_class);
	~Module() override;
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
	enum VersionNumber {
		Latest = -1,
		Legacy = -2
	};

	string config_to_string() const;
	Any config_to_any() const;
	void config_from_string(int version, const string &options);
	void config_from_any(int version, const Any &options);


	string _config_latest_history;
	void set_func_edit(std::function<void()> f);
	std::function<void()> func_edit;



	virtual void _cdecl reset_state() {}


	virtual base::optional<int64> command(ModuleCommand cmd, int64 param) { return base::None; }


	enum Return {
		EndOfStream = -2,
		NotEnoughData = 0,
		NoSource = 0
	};

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
