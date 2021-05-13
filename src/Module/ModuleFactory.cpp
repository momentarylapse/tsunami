/*
 * ModuleFactory.cpp
 *
 *  Created on: 08.04.2018
 *      Author: michi
 */

#include "ModuleFactory.h"

#include "../Device/Stream/AudioInput.h"
#include "../Device/Stream/AudioOutput.h"
#include "../Device/Stream/MidiInput.h"
#include "Module.h"
#include "Audio/SongRenderer.h"
#include "Audio/AudioBackup.h"
#include "Audio/AudioChannelSelector.h"
#include "Audio/AudioJoiner.h"
#include "Audio/AudioEffect.h"
#include "Audio/AudioVisualizer.h"
#include "Audio/PeakMeter.h"
#include "Audio/AudioSucker.h"
#include "Midi/MidiEffect.h"
#include "Midi/MidiJoiner.h"
#include "Midi/MidiSource.h"
#include "Midi/MidiSucker.h"
#include "Audio/PitchDetector.h"
#include "Beats/BeatMidifier.h"
#include "Synth/Synthesizer.h"
#include "Synth/DummySynthesizer.h"
#include "../Plugins/Plugin.h"
#include "../Plugins/PluginManager.h"
#include "../Session.h"
#include "Audio/AudioAccumulator.h"
#include "Midi/MidiAccumulator.h"


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

Module* ModuleFactory::_create_dummy(ModuleCategory type) {
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
	return nullptr;
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
Module* ModuleFactory::create(Session* session, ModuleCategory type, const string& _sub_type) {
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

	if (m)
		m->set_session_etc(session, sub_type);

	// type specific initialization
	if (m and type == ModuleCategory::SYNTHESIZER)
		reinterpret_cast<Synthesizer*>(m)->set_sample_rate(session->sample_rate());
	
	if (config != "")
		m->config_from_string(Module::VERSION_LATEST, config);

	return m;
}
