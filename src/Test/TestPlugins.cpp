/*
 * TestPlugins.cpp
 *
 *  Created on: 24.07.2018
 *      Author: michi
 */

#ifndef NDEBUG

#include "TestPlugins.h"
#include "../Data/Audio/AudioBuffer.h"
#include "../Module/Audio/AudioEffect.h"
#include "../Module/Audio/AudioSource.h"
#include "../Module/Midi/MidiEffect.h"
#include "../Module/Midi/MidiSource.h"
#include "../Module/Synth/Synthesizer.h"
#include "../Module/SignalChain.h"
#include "../Plugins/TsunamiPlugin.h"
#include "../Plugins/PluginManager.h"
#include "../Plugins/Plugin.h"
#include "../Session.h"

TestPlugins::TestPlugins() : UnitTest("plugins") {
}


Array<UnitTest::Test> TestPlugins::tests() {
	Array<Test> list;
	auto *pm = Session::GLOBAL->plugin_manager;
	for (auto &pf: pm->plugin_files)
		list.add({"compile:" + pf.filename.str(), [pf]{ test_compile(pf.type, pf.filename); }});
	auto names = Session::GLOBAL->plugin_manager->find_module_sub_types(ModuleType::AUDIO_EFFECT);
	for (auto &name: names)
		if (name != "Folding" and name != "Equalizer 31")
			list.add({"audio-effect:" + name, [name]{ test_audio_effect(name); }});
	names = Session::GLOBAL->plugin_manager->find_module_sub_types(ModuleType::AUDIO_SOURCE);
	for (auto &name: names)
		list.add({"audio-source:" + name, [name]{ test_audio_source(name); }});

	names = Session::GLOBAL->plugin_manager->find_module_sub_types(ModuleType::MIDI_EFFECT);
	for (auto &name: names)
		list.add({"midi-effect:" + name, [name]{ test_midi_effect(name); }});

	names = Session::GLOBAL->plugin_manager->find_module_sub_types(ModuleType::MIDI_SOURCE);
	for (auto &name: names)
		list.add({"midi-source:" + name, [name]{ test_midi_source(name); }});

	names = Session::GLOBAL->plugin_manager->find_module_sub_types(ModuleType::SYNTHESIZER);
	for (auto &name: names)
		list.add({"synthesizer:" + name, [name]{ test_synthesizer(name); }});

	//list.add({"tsunami-plugins", TestPlugins::test_tsunami_plugin});
	return list;
}


void TestPlugins::test_compile(ModuleType type, const Path &filename) {

	try {
		auto *s = Kaba::Load(filename);
	} catch (Kaba::Exception &e) {
		throw Failure(e.message());
	}
	//Kaba::config.verbose = true;
	//Plugin *p = Session::GLOBAL->plugin_manager->load_and_compile_plugin(type, filename);
	//if (!p->usable(Session::GLOBAL))
	//	throw Failure(p->error_message);

	// hmmm, right now, some tsunami internals are in Kaba::_public_scripts_
	//Kaba::DeleteAllScripts(true, true);
}

void TestPlugins::test_audio_effect(const string &name) {
	auto *fx = CreateAudioEffect(Session::GLOBAL, name);

	AudioBuffer buf;
	buf.resize(1 << 20);

	fx->process(buf);

	delete fx;
}

void TestPlugins::test_audio_source(const string &name) {
	auto *source = CreateAudioSource(Session::GLOBAL, name);

	AudioBuffer buf;
	buf.resize(1 << 20);

	source->read(buf);

	delete source;
}

void TestPlugins::test_midi_effect(const string &name) {
	auto *fx = CreateMidiEffect(Session::GLOBAL, name);

	MidiNoteBuffer buf;
	buf.samples = 1 << 20;

	fx->process(&buf);

	delete fx;
}

void TestPlugins::test_midi_source(const string &name) {
	auto *source = CreateMidiSource(Session::GLOBAL, name);

	MidiEventBuffer buf;
	buf.samples = 1 << 20;

	source->read(buf);

	delete source;
}

void TestPlugins::test_synthesizer(const string &name) {
	auto *chain = new SignalChain(Session::GLOBAL, "chain");
	auto *source = chain->add(ModuleType::MIDI_SOURCE, "Metronome");
	auto *synth = chain->add(ModuleType::SYNTHESIZER, name);
	chain->connect(source, 0, synth, 0);

	AudioBuffer buf;
	buf.resize(1 << 12);

	for (int i=0; i<16; i++)
		synth->port_out[0]->read_audio(buf);

	delete chain;
}

void TestPlugins::test_tsunami_plugin(const string &name) {
}

#endif
