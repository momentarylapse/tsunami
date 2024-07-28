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

namespace tsunami {


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
		pd->source = nullptr;

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
		return {};

	return config->to_any();
}


// not signaling actions/gui!
//   but consistent state
void Module::config_from_string(int _version, const string &param) {
	auto *config = get_config();
	if (!config)
		return;

	if (_version == VersionNumber::Legacy) {
		config->from_string_legacy(param, session);
	} else {
		if (_version != VersionNumber::Latest and _version != version()) {
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

	if (_version != VersionNumber::Latest and _version != version()) {
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
	clone->config_from_string(Module::VersionNumber::Latest, param);
	clone->_config_latest_history = param;
	return clone;
}


string Module::category_to_str(ModuleCategory cat) {
	if (cat == ModuleCategory::AudioSource)
		return "AudioSource";
	if (cat == ModuleCategory::AudioEffect)
		return "AudioEffect";
	if (cat == ModuleCategory::Synthesizer)
		return "Synthesizer";
	if (cat == ModuleCategory::MidiSource)
		return "MidiSource";
	if (cat == ModuleCategory::MidiEffect)
		return "MidiEffect";
	if (cat == ModuleCategory::BeatSource)
		return "BeatSource";
	if (cat == ModuleCategory::AudioVisualizer)
		return "AudioVisualizer";
	if (cat == ModuleCategory::PitchDetector)
		return "PitchDetector";
	if (cat == ModuleCategory::Stream)
		return "Stream";
	if (cat == ModuleCategory::Plumbing)
		return "Plumbing";
	if (cat == ModuleCategory::SignalChain)
		return "SignalChain";
	if (cat == ModuleCategory::TsunamiPlugin)
		return "TsunamiPlugin";
	if (cat == ModuleCategory::Other)
		return "Other";
	return "???";
}


ModuleCategory Module::category_from_str(const string &str) {
	if (str == "AudioSource")
		return ModuleCategory::AudioSource;
	if (str == "Plumbing")
		return ModuleCategory::Plumbing;
	if (str == "Stream")
		return ModuleCategory::Stream;
	if (str == "AudioEffect" or str == "Effect")
		return ModuleCategory::AudioEffect;
	if (str == "Synthesizer" or str == "Synth")
		return ModuleCategory::Synthesizer;
	if (str == "MidiEffect")
		return ModuleCategory::MidiEffect;
	if (str == "MidiSource")
		return ModuleCategory::MidiSource;
	if (str == "BeatSource")
		return ModuleCategory::BeatSource;
	if (str == "PitchDetector")
		return ModuleCategory::PitchDetector;
	if (str == "AudioVisualizer")
		return ModuleCategory::AudioVisualizer;
	if (str == "SignalChain")
		return ModuleCategory::SignalChain;
	if (str == "TsunamiPlugin")
		return ModuleCategory::TsunamiPlugin;
	if (str == "Other")
		return ModuleCategory::Other;
	return (ModuleCategory)-1;
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

}

