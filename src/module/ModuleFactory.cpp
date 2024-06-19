/*
 * ModuleFactory.cpp
 *
 *  Created on: 08.04.2018
 *      Author: michi
 */

#include "ModuleFactory.h"
#include "Module.h"
#include "audio/SongRenderer.h"
#include "audio/AudioBackup.h"
#include "audio/AudioChannelSelector.h"
#include "audio/AudioJoiner.h"
#include "audio/AudioEffect.h"
#include "audio/AudioVisualizer.h"
#include "audio/PeakMeter.h"
#include "audio/AudioSucker.h"
#include "midi/MidiEffect.h"
#include "midi/MidiEventStreamer.h"
#include "midi/MidiJoiner.h"
#include "midi/MidiSource.h"
#include "midi/MidiSplitter.h"
#include "midi/MidiSucker.h"
#include "audio/PitchDetector.h"
#include "beats/BeatMidifier.h"
#include "synthesizer/Synthesizer.h"
#include "synthesizer/DummySynthesizer.h"
#include "audio/AudioAccumulator.h"
#include "midi/MidiAccumulator.h"
#include "stream/AudioInput.h"
#include "stream/AudioOutput.h"
#include "stream/MidiInput.h"
#include "../plugins/Plugin.h"
#include "../plugins/PluginManager.h"
#include "../Session.h"

#include "../lib/os/msg.h"

namespace tsunami {

Module* ModuleFactory::_create_special(Session* session, ModuleCategory category, const string& _class) {
	if (category == ModuleCategory::PLUMBING) {
		if (_class == "BeatMidifier")
			return new BeatMidifier;
		if (_class == "AudioJoiner")
			return new AudioJoiner;
		if (_class == "AudioSucker")
			return new AudioSucker(session);
		if (_class == "AudioBackup")
			return new AudioBackup(session);
		if (_class == "AudioAccumulator")
			return new AudioAccumulator;
		if (_class == "AudioChannelSelector")
			return new AudioChannelSelector;
		if (_class == "MidiJoiner")
			return new MidiJoiner;
		if (_class == "MidiSplitter")
			return new MidiSplitter;
		if (_class == "MidiAccumulator")
			return new MidiAccumulator;
		if (_class == "MidiSucker")
			return new MidiSucker;
	} else if (category == ModuleCategory::AUDIO_SOURCE) {
		if (_class == "SongRenderer")
			return new SongRenderer(session->song.get(), true);
	} else if (category == ModuleCategory::AUDIO_EFFECT) {
		if (_class == "Dummy" or _class == "")
			return new AudioEffect;
	} else if (category == ModuleCategory::MIDI_EFFECT) {
		if (_class == "Dummy" or _class == "")
			return new MidiEffect;
	} else if (category == ModuleCategory::MIDI_SOURCE) {
		if (_class == "MidiEventStreamer")
			return new MidiEventStreamer;
	} else if (category == ModuleCategory::SYNTHESIZER) {
		if (_class == "Dummy" or _class == "")
			return new DummySynthesizer;
	} else if (category == ModuleCategory::PITCH_DETECTOR) {
		if (_class == "Dummy" or _class == "")
			return new DummyPitchDetector;
	} else if (category == ModuleCategory::BEAT_SOURCE) {
		if (_class == "BarStreamer")
			{}//return new BarStreamer()
	} else if (category == ModuleCategory::AUDIO_VISUALIZER) {
		if (_class == "PeakMeter")
			return new PeakMeter;
	} else if (category == ModuleCategory::STREAM) {
		if (_class == "AudioOutput")
			return new AudioOutput(session);
		if (_class == "AudioInput")
			return new AudioInput(session);
		if (_class == "MidiInput")
			return new MidiInput(session);
	}
	return nullptr;
}

xfer<Module> ModuleFactory::_create_dummy(ModuleCategory type) {
	if (type == ModuleCategory::SYNTHESIZER)
		return new DummySynthesizer;
	if (type == ModuleCategory::AUDIO_SOURCE)
		return new AudioSource;
	if (type == ModuleCategory::AUDIO_VISUALIZER)
		return new AudioVisualizer;
	if (type == ModuleCategory::MIDI_SOURCE)
		return new MidiSource;
	if (type == ModuleCategory::AUDIO_EFFECT)
		return new AudioEffect;
	if (type == ModuleCategory::MIDI_EFFECT)
		return new MidiEffect;
	return new Module(type, "<dummy>");
}

string ModuleFactory::base_class(ModuleCategory type) {
	return Module::category_to_str(type);
}

void _extract_subtype_and_config(ModuleCategory type, const string &s, string &subtype, string &config) {
	subtype = s;
	config = "";
	int pp = s.find(":");
	if (pp >= 0) {
		subtype = s.head(pp);
		config = s.sub(pp + 1);
	}
	if ((type == ModuleCategory::SYNTHESIZER) and (subtype == ""))
		subtype = "Dummy";
}

// can be pre-configured with sub_type="ModuleName:config..."
// TODO return shared<Module>
xfer<Module> ModuleFactory::create(Session* session, ModuleCategory type, const string& _sub_type) {
	string sub_type, config;
	_extract_subtype_and_config(type, _sub_type, sub_type, config);

	Module *m = nullptr;
	Plugin *p = nullptr;

	// non plug-ins
	m = _create_special(session, type, sub_type);

	// plug-ins
	if (!m) {
		p = session->plugin_manager->get_plugin(session, type, sub_type);
		if (p)
			m = reinterpret_cast<Module*>(p->create_instance(session, "*." + base_class(type)));
	}

	// plug-in failed? -> default
	if (!m)
		m = _create_dummy(type);

	m->set_session_etc(session, sub_type);

	// type specific initialization
	if (type == ModuleCategory::SYNTHESIZER)
		reinterpret_cast<Synthesizer*>(m)->set_sample_rate(session->sample_rate());
	
	if (config != "")
		m->config_from_string(Module::VERSION_LATEST, config);

	return m;
}

xfer<Module> ModuleFactory::create_by_class(Session* session, const kaba::Class *type) {
	//msg_error("CREATE MODULE BY CLASS");
	//msg_write(type->long_name());
	//msg_write(type->owner->module->filename.basename_no_ext());

	auto m = reinterpret_cast<Module*>(type->create_instance());
	if (!m) {
		session->e("failed to instanciate class: " + type->long_name());
		m = new Module(ModuleCategory::OTHER, "");
	}

	m->set_session_etc(session, type->owner->module->filename.basename_no_ext());

	// type specific initialization
	if (m->module_category == ModuleCategory::SYNTHESIZER)
		reinterpret_cast<Synthesizer*>(m)->set_sample_rate(session->sample_rate());

	return m;
}

}
