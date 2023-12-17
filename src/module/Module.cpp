/*
 * Module.cpp
 *
 *  Created on: 05.01.2014
 *      Author: michi
 */


#include "Module.h"
#include "ModuleConfiguration.h"
#include "ModuleFactory.h"
#include "port/Port.h"
#include "../Session.h"
#include "../lib/any/any.h"
#include "../lib/kaba/kaba.h"
#include "../lib/kaba/syntax/Class.h"
#include "../plugins/PluginManager.h"
#include "../plugins/Plugin.h"
#include "../stuff/PerformanceMonitor.h"
#include "../view/module/ConfigPanel.h"
#include "../view/module/AutoConfigPanel.h"

const int Module::COMMAND_NOT_HANDLED = 0xdeaddead;


string guess_nice_module_name(const string &_class) {
	if (_class.head(5) == "Audio")
		return _class.sub(5);
	if (_class.head(4) == "Midi")
		return _class.sub(4);
	return _class;
}


Module::Module(ModuleCategory category, const string &_class) {
	//msg_write("new Module " + p2s(this));
	//msg_write(category_to_str(category) + "   " + _class);
	module_category = category;
	module_class = _class;
	module_name = guess_nice_module_name(_class);
	session = Session::GLOBAL;
	enabled = true;
	module_x = module_y = 0;
	allow_config_in_chain = false;
	kaba_class = nullptr;
	belongs_to_system = false;
	func_edit = [this] {
		out_changed.notify();
	};
	perf_channel = PerformanceMonitor::create_channel("module", this);
}

Module::~Module() {
	//msg_write("del Module " + p2s(this));
	// unlink sources
	for (auto &pd: port_in)
		*pd.port = nullptr;

	PerformanceMonitor::delete_channel(perf_channel);
}


void Module::__init__(ModuleCategory category, const string &_class) {
	new(this) Module(category, _class);
}

void Module::__delete__() {
	this->Module::~Module();
}


// internal use for creation
void Module::set_session_etc(Session *_session, const string &_class) {
	session = _session;
	module_class = _class;
	auto *c = get_config();
	if (c) {
		c->_module = this;
		c->kaba_class = kaba::default_context->get_dynamic_type(c);
		/*msg_write("config class: " + p2s(c->_class) + "  " + sub_type);
		if (c->_class)
			msg_write(c->_class->name);*/
	}
	kaba_class = kaba::default_context->get_dynamic_type(this);

	reset_config();
	reset_state();
}


// default version:
//   look for a Module.config in the plugin class
ModuleConfiguration *Module::get_config() const {
	if (!kaba_class)
		return nullptr;
	for (auto &e: kaba_class->elements)
		if ((e.name == "config") and (e.type->get_root()->long_name() == "modules.Module.Config")) {
			auto *config = reinterpret_cast<ModuleConfiguration*>((char*)this + e.offset);
			config->kaba_class = e.type;
			return config;
		}
	return nullptr;
}

int Module::version() const {
	if (!kaba_class)
		return 1;
	for (auto *c: weak(kaba_class->constants))
		if ((c->name == "VERSION") and (c->type->name == "int"))
			return c->as_int();
	return 1;
}


string Module::config_to_string() const {
	auto *config = get_config();
	if (!config)
		return "";

	return config->to_string();
}

Any Module::config_to_any() const {
	auto *config = get_config();
	if (!config)
		return Any();

	return config->to_any();
}


// not signaling actions/gui!
//   but consistent state
void Module::config_from_string(int _version, const string &param) {
	auto *config = get_config();
	if (!config)
		return;

	if (_version == VERSION_LEGACY) {
		config->from_string_legacy(param, session);
	} else {
		if (_version != VERSION_LATEST and _version != version()) {
			// ... TODO
			session->e(format("%s: version request:%d current:%d", module_class, _version, version()));
		}

		config->from_string(param, session);
	}
	on_config();
}

// not signaling actions/gui!
//   but consistent state
void Module::config_from_any(int _version, const Any &param) {
	auto *config = get_config();
	if (!config)
		return;

	if (_version != VERSION_LATEST and _version != version()) {
		// ... TODO
		session->e(format("%s: version request:%d current:%d", module_class, _version, version()));
	}

	config->from_any(param, session);
	on_config();
}



// default version
//   try to execute   Module.config.reset()
// not signaling actions/gui!
//   but consistent state
void Module::reset_config() {
	auto *config = get_config();
	if (config)
		config->reset();
	on_config();
}


// default version
//   try to create an AutoConfigPanel
xfer<ConfigPanel> Module::create_panel() {
	auto *config = get_config();
	if (!config)
		return nullptr;
	auto aa = get_auto_conf(config, session);
	if (aa.num == 0)
		return nullptr;
	return new AutoConfigPanel(aa, this);
}

// call after editing (by ConfigPanel etc) to signal a config change
//   -> signaling ui, actions
void Module::changed() {
	on_config();
	//out_changed.notify();
	func_edit();
}


// don't copy the func_edit
xfer<Module> Module::copy() const {
	Module *clone = ModuleFactory::create(session, module_category, module_class);
	string param = config_to_string();
	clone->config_from_string(Module::VERSION_LATEST, param);
	clone->_config_latest_history = param;
	return clone;
}


string Module::category_to_str(ModuleCategory cat) {
	if (cat == ModuleCategory::AUDIO_SOURCE)
		return "AudioSource";
	if (cat == ModuleCategory::AUDIO_EFFECT)
		return "AudioEffect";
	if (cat == ModuleCategory::SYNTHESIZER)
		return "Synthesizer";
	if (cat == ModuleCategory::MIDI_SOURCE)
		return "MidiSource";
	if (cat == ModuleCategory::MIDI_EFFECT)
		return "MidiEffect";
	if (cat == ModuleCategory::BEAT_SOURCE)
		return "BeatSource";
	if (cat == ModuleCategory::AUDIO_VISUALIZER)
		return "AudioVisualizer";
	if (cat == ModuleCategory::PITCH_DETECTOR)
		return "PitchDetector";
	if (cat == ModuleCategory::STREAM)
		return "Stream";
	if (cat == ModuleCategory::PLUMBING)
		return "Plumbing";
	if (cat == ModuleCategory::SIGNAL_CHAIN)
		return "SignalChain";
	if (cat == ModuleCategory::TSUNAMI_PLUGIN)
		return "TsunamiPlugin";
	if (cat == ModuleCategory::OTHER)
		return "Other";
	return "???";
}


ModuleCategory Module::category_from_str(const string &str) {
	if (str == "AudioSource")
		return ModuleCategory::AUDIO_SOURCE;
	if (str == "Plumbing")
		return ModuleCategory::PLUMBING;
	if (str == "Stream")
		return ModuleCategory::STREAM;
	if (str == "AudioEffect" or str == "Effect")
		return ModuleCategory::AUDIO_EFFECT;
	if (str == "Synthesizer" or str == "Synth")
		return ModuleCategory::SYNTHESIZER;
	if (str == "MidiEffect")
		return ModuleCategory::MIDI_EFFECT;
	if (str == "MidiSource")
		return ModuleCategory::MIDI_SOURCE;
	if (str == "BeatSource")
		return ModuleCategory::BEAT_SOURCE;
	if (str == "PitchDetector")
		return ModuleCategory::PITCH_DETECTOR;
	if (str == "AudioVisualizer")
		return ModuleCategory::AUDIO_VISUALIZER;
	if (str == "SignalChain")
		return ModuleCategory::SIGNAL_CHAIN;
	if (str == "TsunamiPlugin")
		return ModuleCategory::TSUNAMI_PLUGIN;
	if (str == "Other")
		return ModuleCategory::OTHER;
	return (ModuleCategory)-1;
}

void Module::_plug_in(int in_port, Module *source, int source_port) {
	Port *sp = source->port_out[source_port];
	auto tp = port_in[in_port];
	if (sp->type != tp.type)
		throw Exception("connect: port type mismatch");

	*tp.port = sp;
}

void Module::_unplug_in(int in_port) {
	auto tp = port_in[in_port];
	*tp.port = nullptr;
}


void Module::set_func_edit(std::function<void()> f) {
	func_edit = f;
}

void Module::perf_start() {
	PerformanceMonitor::start_busy(perf_channel);
}

void Module::perf_end() {
	PerformanceMonitor::end_busy(perf_channel);
}

// only for PerformanceMonitor
void Module::perf_set_parent(Module *m) {
	if (m)
		PerformanceMonitor::set_parent(perf_channel, m->perf_channel);
	else
		PerformanceMonitor::set_parent(perf_channel, -1);
}

