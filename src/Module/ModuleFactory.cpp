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
#include "Audio/AudioJoiner.h"
#include "Audio/AudioRecorder.h"
#include "Audio/AudioEffect.h"
#include "Audio/AudioVisualizer.h"
#include "Audio/PeakMeter.h"
#include "Audio/AudioSucker.h"
#include "Midi/MidiEffect.h"
#include "Midi/MidiJoiner.h"
#include "Midi/MidiSource.h"
#include "Midi/MidiRecorder.h"
#include "Midi/MidiSucker.h"
#include "Audio/PitchDetector.h"
#include "Beats/BeatMidifier.h"
#include "Synth/Synthesizer.h"
#include "Synth/DummySynthesizer.h"
#include "../Plugins/Plugin.h"
#include "../Plugins/PluginManager.h"
#include "../Session.h"


Module* ModuleFactory::_create_special(Session* session, ModuleCategory type, const string& sub_type) {
	if (type == ModuleCategory::PLUMBING) {
		if (sub_type == "BeatMidifier")
			return new BeatMidifier;
		if (sub_type == "AudioJoiner")
			return new AudioJoiner;
		if (sub_type == "AudioSucker")
			return new AudioSucker;
		if (sub_type == "AudioBackup")
			return new AudioBackup(session);
		if (sub_type == "AudioRecorder")
			return new AudioRecorder;
		if (sub_type == "MidiJoiner")
			return new MidiJoiner;
		if (sub_type == "MidiRecorder")
			return new MidiRecorder;
		if (sub_type == "MidiSucker")
			return new MidiSucker;
	} else if (type == ModuleCategory::AUDIO_SOURCE) {
		if (sub_type == "SongRenderer")
			return new SongRenderer(session->song.get(), true);
	} else if (type == ModuleCategory::AUDIO_EFFECT) {
		if (sub_type == "Dummy" or sub_type == "")
			return new AudioEffect;
	} else if (type == ModuleCategory::MIDI_EFFECT) {
		if (sub_type == "Dummy" or sub_type == "")
			return new MidiEffect;
	} else if (type == ModuleCategory::SYNTHESIZER) {
		if (sub_type == "Dummy" or sub_type == "")
			return new DummySynthesizer;
	} else if (type == ModuleCategory::PITCH_DETECTOR) {
		if (sub_type == "Dummy" or sub_type == "")
			return new DummyPitchDetector;
	} else if (type == ModuleCategory::BEAT_SOURCE) {
		if (sub_type == "BarStreamer")
			{}//return new BarStreamer()
	} else if (type == ModuleCategory::AUDIO_VISUALIZER) {
		if (sub_type == "PeakMeter")
			return new PeakMeter;
	} else if (type == ModuleCategory::STREAM) {
		if (sub_type == "AudioOutput")
			return new AudioOutput(session);
		if (sub_type == "AudioInput")
			return new AudioInput(session);
		if (sub_type == "MidiInput")
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
	return Module::category_to_name(type);
}

void _extract_subtype_and_config(ModuleCategory type, const string &s, string &subtype, string &config) {
	subtype = s;
	config = "";
	int pp = s.find(":");
	if (pp >= 0) {
		subtype = s.head(pp);
		config = s.substr(pp + 1, -1);
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
