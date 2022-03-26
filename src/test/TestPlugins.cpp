/*
 * TestPlugins.cpp
 *
 *  Created on: 24.07.2018
 *      Author: michi
 */

#ifndef NDEBUG

#include "TestPlugins.h"
#include "../data/audio/AudioBuffer.h"
#include "../module/audio/AudioEffect.h"
#include "../module/audio/AudioSource.h"
#include "../module/midi/MidiEffect.h"
#include "../module/midi/MidiSource.h"
#include "../module/synthesizer/Synthesizer.h"
#include "../module/SignalChain.h"
#include "../plugins/TsunamiPlugin.h"
#include "../plugins/PluginManager.h"
#include "../plugins/Plugin.h"
#include "../Session.h"

TestPlugins::TestPlugins() : UnitTest("plugins") {
}


Array<UnitTest::Test> TestPlugins::tests() {
	Array<Test> list;
	auto *pm = Session::GLOBAL->plugin_manager;
	for (auto &pf: pm->plugin_files)
		list.add({"compile:" + pf.filename.str(), [pf]{ test_compile(pf.type, pf.filename); }});
	auto names = Session::GLOBAL->plugin_manager->find_module_sub_types(ModuleCategory::AUDIO_EFFECT);
	for (auto &name: names)
		if (name != "Folding" and name != "Equalizer 31")
			list.add({"audio-effect:" + name, [name]{ test_audio_effect(name); }});
	names = Session::GLOBAL->plugin_manager->find_module_sub_types(ModuleCategory::AUDIO_SOURCE);
	for (auto &name: names)
		list.add({"audio-source:" + name, [name]{ test_audio_source(name); }});

	names = Session::GLOBAL->plugin_manager->find_module_sub_types(ModuleCategory::MIDI_EFFECT);
	for (auto &name: names)
		list.add({"midi-effect:" + name, [name]{ test_midi_effect(name); }});

	names = Session::GLOBAL->plugin_manager->find_module_sub_types(ModuleCategory::MIDI_SOURCE);
	for (auto &name: names)
		list.add({"midi-source:" + name, [name]{ test_midi_source(name); }});

	names = Session::GLOBAL->plugin_manager->find_module_sub_types(ModuleCategory::SYNTHESIZER);
	for (auto &name: names)
		list.add({"synthesizer:" + name, [name]{ test_synthesizer(name); }});

	//list.add({"tsunami-plugins", TestPlugins::test_tsunami_plugin});
	return list;
}


void TestPlugins::test_compile(ModuleCategory type, const Path &filename) {

	try {
		auto s = kaba::load(filename);
	} catch (kaba::Exception &e) {
		throw Failure(e.message());
	}
	//kaba::config.verbose = true;
	//Plugin *p = Session::GLOBAL->plugin_manager->load_and_compile_plugin(type, filename);
	//if (!p->usable(Session::GLOBAL))
	//	throw Failure(p->error_message);

	// hmmm, right now, some tsunami internals are in kaba::_public_scripts_
	//kaba::DeleteAllScripts(true, true);
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
	auto *source = chain->add(ModuleCategory::MIDI_SOURCE, "Metronome");
	auto *synth = chain->add(ModuleCategory::SYNTHESIZER, name);
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
