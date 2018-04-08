/*
 * ModuleFactory.cpp
 *
 *  Created on: 08.04.2018
 *      Author: michi
 */

#include "ModuleFactory.h"
#include "Module.h"
#include "Audio/SongRenderer.h"
#include "Audio/AudioJoiner.h"
#include "Audio/AudioEffect.h"
#include "Audio/AudioVisualizer.h"
#include "Audio/PeakMeter.h"
#include "Audio/AudioSucker.h"
#include "Midi/MidiEffect.h"
#include "Midi/MidiSource.h"
#include "Audio/PitchDetector.h"
#include "Beats/BeatMidifier.h"
#include "Synth/Synthesizer.h"
#include "Synth/DummySynthesizer.h"
#include "Synth/SampleSynthesizer.h"
#include "../Device/OutputStream.h"
#include "../Device/InputStreamAudio.h"
#include "../Device/InputStreamMidi.h"
#include "../Plugins/Plugin.h"
#include "../Plugins/PluginManager.h"
#include "../Session.h"


Module* ModuleFactory::_create_special(Session* session, int type, const string& sub_type)
{
	if (type == Module::Type::BEAT_MIDIFIER){
		return new BeatMidifier;
	}else if (type == Module::Type::AUDIO_JOINER){
		return new AudioJoiner(session);
	}else if (type == Module::Type::AUDIO_SUCKER){
		return new AudioSucker(session);
	}else if (type == Module::Type::AUDIO_SOURCE){
		if (sub_type == "SongRenderer")
			return new SongRenderer(session->song);
	}else if (type == Module::Type::AUDIO_EFFECT){
		if (sub_type == "Dummy" or sub_type == "")
			return new AudioEffect;
	}else if (type == Module::Type::MIDI_EFFECT){
		if (sub_type == "Dummy" or sub_type == "")
			return new MidiEffect;
	}else if (type == Module::Type::SYNTHESIZER){
		if (sub_type == "Dummy" or sub_type == "")
			return new DummySynthesizer;
		//if (sub_type == "Sample")
		//	return new SampleSynthesizer;
	}else if (type == Module::Type::PITCH_DETECTOR){
		return new PitchDetector;
	}else if (type == Module::Type::BEAT_SOURCE){
		if (sub_type == "BarStreamer")
			{}//return new BarStreamer()
	}else if (type == Module::Type::AUDIO_VISUALIZER){
		if (sub_type == "PeakMeter")
			return new PeakMeter(session);
	}else if (type == Module::Type::OUTPUT_STREAM_AUDIO){
		return new OutputStream(session, NULL);
	}else if (type == Module::Type::INPUT_STREAM_AUDIO){
		return new InputStreamAudio(session);
	}else if (type == Module::Type::INPUT_STREAM_MIDI){
		return new InputStreamMidi(session);
	}
	return NULL;
}

Module* ModuleFactory::_create_dummy(int type)
{
	if (type == Module::Type::SYNTHESIZER)
		return new DummySynthesizer;
	if (type == Module::Type::AUDIO_SOURCE)
		return new AudioSource;
	if (type == Module::Type::AUDIO_VISUALIZER)
		return new AudioVisualizer;
	if (type == Module::Type::MIDI_SOURCE)
		return new MidiSource;
	if (type == Module::Type::AUDIO_EFFECT)
		return new AudioEffect;
	if (type == Module::Type::MIDI_EFFECT)
		return new MidiEffect;
	return NULL;
}

string ModuleFactory::base_class(int type)
{
	return Module::type_to_name(type);
}

Module* ModuleFactory::create(Session* session, int type, const string& sub_type)
{
	Module *m = NULL;
	Plugin *p = NULL;

	// non plug-ins
	m = _create_special(session, type, sub_type);

	// plug-ins
	if (!m){
		p = session->plugin_manager->GetPlugin(session, type, sub_type);
		if (p and p->usable)
			m = (Module*)p->create_instance(session, base_class(type));
	}

	// plug-in failed? -> default
	if (!m)
		m = _create_dummy(type);

	if (m)
		m->set_session_etc(session, sub_type, p);

	// type specific initialization
	if (m and type == Module::Type::SYNTHESIZER)
		((Synthesizer*)m)->sample_rate = session->sample_rate();

	return m;
}
