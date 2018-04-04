/*
 * PluginManager.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "PluginManager.h"

#include "../Audio/Source/SongRenderer.h"
#include "../Tsunami.h"
#include "../TsunamiWindow.h"
#include "../Session.h"
#include "FastFourierTransform.h"
#include "../View/Helper/Slider.h"
#include "../Device/InputStreamAudio.h"
#include "../Device/InputStreamMidi.h"
#include "../Device/OutputStream.h"
#include "../Device/DeviceManager.h"
#include "../Audio/Synth/Synthesizer.h"
#include "../Audio/Synth/DummySynthesizer.h"
#include "../Module/Port/AudioPort.h"
#include "../Audio/Source/AudioSource.h"
#include "../Module/Port/MidiPort.h"
#include "../Midi/MidiSource.h"
#include "../Rhythm/Bar.h"
#include "../Module/Port/BeatPort.h"
#include "../Rhythm/BeatSource.h"
#include "../View/Helper/Progress.h"
#include "../Storage/Storage.h"
#include "../View/AudioView.h"
#include "../View/Dialog/ConfigurableSelectorDialog.h"
#include "../View/SideBar/SampleManagerConsole.h"
#include "../View/Mode/ViewModeCapture.h"
#include "AudioEffect.h"
#include "Plugin.h"
#include "ConfigPanel.h"
#include "ExtendedAudioBuffer.h"
#include "MidiEffect.h"
#include "SongPlugin.h"
#include "TsunamiPlugin.h"
#include "FavoriteManager.h"

#define _offsetof(CLASS, ELEMENT) (int)( (char*)&((CLASS*)1)->ELEMENT - (char*)((CLASS*)1) )

extern InputStreamAudio *export_view_input;


PluginManager::PluginManager()
{
	favorites = new FavoriteManager;

	FindPlugins();
}

PluginManager::~PluginManager()
{
	delete(favorites);
	Kaba::End();
}


bool GlobalAllowTermination()
{
	return tsunami->allowTermination();
}


void GlobalSetTempBackupFilename(const string &filename)
{
	//InputStreamAudio::setTempBackupFilename(filename);
}

Synthesizer* GlobalCreateSynthesizer(Session *session, const string &name)
{
	return session->plugin_manager->CreateSynthesizer(session, name);
}

void PluginManager::LinkAppScriptData()
{
	Kaba::config.directory = "";

	// api definition
	Kaba::LinkExternal("device_manager", &tsunami->device_manager);
	Kaba::LinkExternal("colors", &AudioView::_export_colors);
	Kaba::LinkExternal("view_input", &export_view_input);
	Kaba::LinkExternal("fft_c2c", (void*)&FastFourierTransform::fft_c2c);
	Kaba::LinkExternal("fft_r2c", (void*)&FastFourierTransform::fft_r2c);
	Kaba::LinkExternal("fft_c2r_inv", (void*)&FastFourierTransform::fft_c2r_inv);
	Kaba::LinkExternal("CreateSynthesizer", (void*)&GlobalCreateSynthesizer);
	Kaba::LinkExternal("CreateAudioEffect", (void*)&CreateAudioEffect);
	Kaba::LinkExternal("CreateAudioSource", (void*)&CreateAudioSource);
	Kaba::LinkExternal("CreateMidiEffect", (void*)&CreateMidiEffect);
	Kaba::LinkExternal("CreateMidiSource", (void*)&CreateMidiSource);
	Kaba::LinkExternal("AllowTermination", (void*)&GlobalAllowTermination);
	Kaba::LinkExternal("SetTempBackupFilename", (void*)&GlobalSetTempBackupFilename);
	Kaba::LinkExternal("SelectSample", (void*)&SampleManagerConsole::select);

	Kaba::DeclareClassSize("Range", sizeof(Range));
	Kaba::DeclareClassOffset("Range", "offset", _offsetof(Range, offset));
	Kaba::DeclareClassOffset("Range", "length", _offsetof(Range, length));


	Kaba::DeclareClassSize("Session", sizeof(Session));
	Kaba::DeclareClassOffset("Session", "id", _offsetof(Session, id));
	Kaba::DeclareClassOffset("Session", "storage", _offsetof(Session, storage));
	Kaba::DeclareClassOffset("Session", "win", _offsetof(Session, _kaba_win));
	Kaba::DeclareClassOffset("Session", "song", _offsetof(Session, song));
	Kaba::LinkExternal("Session.sample_rate", Kaba::mf(&Session::sample_rate));
	Kaba::LinkExternal("Session.i", Kaba::mf(&Session::i));
	Kaba::LinkExternal("Session.w", Kaba::mf(&Session::w));
	Kaba::LinkExternal("Session.e", Kaba::mf(&Session::e));


	ModuleConfiguration plugin_data;
	Kaba::DeclareClassSize("PluginData", sizeof(ModuleConfiguration));
	Kaba::LinkExternal("PluginData." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&ModuleConfiguration::__init__));
	Kaba::DeclareClassVirtualIndex("PluginData", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&ModuleConfiguration::__delete__), &plugin_data);
	Kaba::DeclareClassVirtualIndex("PluginData", "reset", Kaba::mf(&ModuleConfiguration::reset), &plugin_data);


	ConfigPanel config_panel;
	Kaba::DeclareClassSize("ConfigPanel", sizeof(ConfigPanel));
	Kaba::LinkExternal("ConfigPanel." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&ConfigPanel::__init__));
	Kaba::DeclareClassVirtualIndex("ConfigPanel", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&ConfigPanel::__delete__), &config_panel);
	Kaba::DeclareClassVirtualIndex("ConfigPanel", "update", Kaba::mf(&ConfigPanel::update), &config_panel);
	Kaba::LinkExternal("ConfigPanel.notify", Kaba::mf(&ConfigPanel::notify));
	Kaba::DeclareClassOffset("ConfigPanel", "c", _offsetof(ConfigPanel, c));

	AudioSource asource;
	Kaba::DeclareClassSize("AudioSource", sizeof(AudioSource));
	Kaba::DeclareClassOffset("AudioSource", "name", _offsetof(AudioSource, name));
	Kaba::DeclareClassOffset("AudioSource", "usable", _offsetof(AudioSource, usable));
	Kaba::DeclareClassOffset("AudioSource", "session", _offsetof(AudioSource, session));
	Kaba::LinkExternal("AudioSource." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&AudioSource::__init__));
	Kaba::DeclareClassVirtualIndex("AudioSource", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&AudioSource::__delete__), &asource);
	Kaba::DeclareClassVirtualIndex("AudioSource", "read", Kaba::mf(&AudioSource::read), &asource);
	Kaba::DeclareClassVirtualIndex("AudioSource", "reset", Kaba::mf(&AudioSource::reset), &asource);
	Kaba::DeclareClassVirtualIndex("AudioSource", "get_pos", Kaba::mf(&AudioSource::get_pos), &asource);
	Kaba::DeclareClassVirtualIndex("AudioSource", "create_panel", Kaba::mf(&AudioSource::create_panel), &asource);
	Kaba::LinkExternal("AudioSource.reset_config", Kaba::mf(&AudioSource::reset_config));
	Kaba::DeclareClassVirtualIndex("AudioSource", "reset_state", Kaba::mf(&AudioSource::reset_state), &asource);
	Kaba::LinkExternal("AudioSource.notify", Kaba::mf(&AudioSource::notify));
	Kaba::DeclareClassVirtualIndex("AudioSource", "on_config", Kaba::mf(&AudioSource::on_config), &asource);

	AudioEffect effect;
	Kaba::DeclareClassSize("AudioEffect", sizeof(AudioEffect));
	Kaba::DeclareClassOffset("AudioEffect", "name", _offsetof(AudioEffect, name));
	Kaba::DeclareClassOffset("AudioEffect", "usable", _offsetof(AudioEffect, usable));
	Kaba::DeclareClassOffset("AudioEffect", "session", _offsetof(AudioEffect, session));
	Kaba::DeclareClassOffset("AudioEffect", "sample_rate", _offsetof(AudioEffect, sample_rate));
	Kaba::LinkExternal("AudioEffect." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&AudioEffect::__init__));
	Kaba::DeclareClassVirtualIndex("AudioEffect", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&AudioEffect::__delete__), &effect);
	Kaba::DeclareClassVirtualIndex("AudioEffect", "process", Kaba::mf(&AudioEffect::process), &effect);
	Kaba::DeclareClassVirtualIndex("AudioEffect", "create_panel", Kaba::mf(&AudioEffect::create_panel), &effect);
	Kaba::LinkExternal("AudioEffect.reset_config", Kaba::mf(&AudioEffect::reset_config));
	Kaba::DeclareClassVirtualIndex("AudioEffect", "reset_state", Kaba::mf(&AudioEffect::reset_state), &effect);
	Kaba::LinkExternal("AudioEffect.notify", Kaba::mf(&AudioEffect::notify));
	Kaba::DeclareClassVirtualIndex("AudioEffect", "on_config", Kaba::mf(&AudioEffect::on_config), &effect);

	MidiEffect midieffect;
	Kaba::DeclareClassSize("MidiEffect", sizeof(MidiEffect));
	Kaba::DeclareClassOffset("MidiEffect", "name", _offsetof(MidiEffect, name));
	Kaba::DeclareClassOffset("MidiEffect", "only_on_selection", _offsetof(MidiEffect, only_on_selection));
	Kaba::DeclareClassOffset("MidiEffect", "range", _offsetof(MidiEffect, range));
	Kaba::DeclareClassOffset("MidiEffect", "usable", _offsetof(MidiEffect, usable));
	Kaba::DeclareClassOffset("MidiEffect", "session", _offsetof(MidiEffect, session));
	Kaba::LinkExternal("MidiEffect." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&MidiEffect::__init__));
	Kaba::DeclareClassVirtualIndex("MidiEffect", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&MidiEffect::__delete__), &midieffect);
	Kaba::DeclareClassVirtualIndex("MidiEffect", "process", Kaba::mf(&MidiEffect::process), &midieffect);
	Kaba::DeclareClassVirtualIndex("MidiEffect", "create_panel", Kaba::mf(&MidiEffect::create_panel), &midieffect);
	Kaba::LinkExternal("MidiEffect.reset_config", Kaba::mf(&MidiEffect::reset_config));
	Kaba::DeclareClassVirtualIndex("MidiEffect", "reset_state", Kaba::mf(&MidiEffect::reset_state), &midieffect);
	Kaba::LinkExternal("MidiEffect.notify", Kaba::mf(&MidiEffect::notify));
	Kaba::DeclareClassVirtualIndex("MidiEffect", "on_config", Kaba::mf(&MidiEffect::on_config), &midieffect);
	Kaba::LinkExternal("MidiEffect.note", Kaba::mf(&MidiEffect::note));
	Kaba::LinkExternal("MidiEffect.skip", Kaba::mf(&MidiEffect::skip));
	Kaba::LinkExternal("MidiEffect.note_x", Kaba::mf(&MidiEffect::note_x));
	Kaba::LinkExternal("MidiEffect.skip_x", Kaba::mf(&MidiEffect::skip_x));

	Kaba::DeclareClassSize("AudioBuffer", sizeof(AudioBuffer));
	Kaba::DeclareClassOffset("AudioBuffer", "offset", _offsetof(AudioBuffer, offset));
	Kaba::DeclareClassOffset("AudioBuffer", "length", _offsetof(AudioBuffer, length));
	Kaba::DeclareClassOffset("AudioBuffer", "channels", _offsetof(AudioBuffer, channels));
	Kaba::DeclareClassOffset("AudioBuffer", "r", _offsetof(AudioBuffer, c[0]));
	Kaba::DeclareClassOffset("AudioBuffer", "l", _offsetof(AudioBuffer, c[1]));
	Kaba::DeclareClassOffset("AudioBuffer", "peaks", _offsetof(AudioBuffer, peaks));
	Kaba::LinkExternal("AudioBuffer." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&AudioBuffer::__init__));
	Kaba::LinkExternal("AudioBuffer." + Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&AudioBuffer::__delete__));
	Kaba::LinkExternal("AudioBuffer.clear", Kaba::mf(&AudioBuffer::clear));
	Kaba::LinkExternal("AudioBuffer." + Kaba::IDENTIFIER_FUNC_ASSIGN, Kaba::mf(&AudioBuffer::__assign__));
	Kaba::LinkExternal("AudioBuffer.range", Kaba::mf(&AudioBuffer::range));
	Kaba::LinkExternal("AudioBuffer.add", Kaba::mf(&AudioBuffer::add));
	Kaba::LinkExternal("AudioBuffer.set", Kaba::mf(&AudioBuffer::set));
	Kaba::LinkExternal("AudioBuffer.get_spectrum", Kaba::mf(&ExtendedAudioBuffer::get_spectrum));


	Kaba::DeclareClassSize("RingBuffer", sizeof(RingBuffer));
	//Kaba::LinkExternal("RingBuffer." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&RingBuffer::__init__));
	Kaba::LinkExternal("RingBuffer.available", Kaba::mf(&RingBuffer::available));
	Kaba::LinkExternal("RingBuffer.read", Kaba::mf(&RingBuffer::read));
	Kaba::LinkExternal("RingBuffer.write", Kaba::mf(&RingBuffer::write));
	Kaba::LinkExternal("RingBuffer.readRef", Kaba::mf(&RingBuffer::readRef));
	Kaba::LinkExternal("RingBuffer.peekRef", Kaba::mf(&RingBuffer::peekRef));
	Kaba::LinkExternal("RingBuffer.writeRef", Kaba::mf(&RingBuffer::writeRef));
	Kaba::LinkExternal("RingBuffer.moveReadPos", Kaba::mf(&RingBuffer::moveReadPos));
	Kaba::LinkExternal("RingBuffer.moveWritePos", Kaba::mf(&RingBuffer::moveWritePos));
	Kaba::LinkExternal("RingBuffer.clear", Kaba::mf(&RingBuffer::clear));

	Kaba::DeclareClassSize("Sample", sizeof(Sample));
	Kaba::DeclareClassOffset("Sample", "name", _offsetof(Sample, name));
	Kaba::DeclareClassOffset("Sample", "type", _offsetof(Sample, type));
	Kaba::DeclareClassOffset("Sample", "buf", _offsetof(Sample, buf));
	Kaba::DeclareClassOffset("Sample", "midi", _offsetof(Sample, midi));
	Kaba::DeclareClassOffset("Sample", "volume", _offsetof(Sample, volume));
	Kaba::DeclareClassOffset("Sample", "uid", _offsetof(Sample, uid));
	Kaba::DeclareClassOffset("Sample", "tags", _offsetof(Sample, tags));
	Kaba::LinkExternal("Sample.createRef", Kaba::mf(&Sample::create_ref));
	Kaba::LinkExternal("Sample.getValue", Kaba::mf(&Sample::getValue));

	Sample sample(0);
	//sample.owner = tsunami->song;
	SampleRef sampleref(&sample);
	Kaba::DeclareClassSize("SampleRef", sizeof(SampleRef));
	Kaba::DeclareClassOffset("SampleRef", "buf", _offsetof(SampleRef, buf));
	Kaba::DeclareClassOffset("SampleRef", "midi", _offsetof(SampleRef, midi));
	Kaba::DeclareClassOffset("SampleRef", "origin", _offsetof(SampleRef, origin));
	Kaba::LinkExternal("SampleRef." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&SampleRef::__init__));
	Kaba::DeclareClassVirtualIndex("SampleRef", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&SampleRef::__delete__), &sampleref);

	MidiPort mport;
	Kaba::DeclareClassSize("MidiPort", sizeof(MidiPort));
	Kaba::LinkExternal("MidiPort." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&MidiPort::__init__));
	Kaba::DeclareClassVirtualIndex("MidiPort", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&MidiPort::__delete__), &mport);
	Kaba::DeclareClassVirtualIndex("MidiPort", "read", Kaba::mf(&MidiPort::read), &mport);
	Kaba::DeclareClassVirtualIndex("MidiPort", "reset", Kaba::mf(&MidiPort::reset), &mport);

	MidiSource msource;
	Kaba::DeclareClassSize("MidiSource", sizeof(MidiSource));
	Kaba::DeclareClassOffset("MidiSource", "out", _offsetof(MidiSource, out));
	Kaba::DeclareClassOffset("MidiSource", "session", _offsetof(MidiSource, session));
	Kaba::LinkExternal("MidiSource." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&MidiSource::__init__));
	Kaba::DeclareClassVirtualIndex("MidiSource", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&MidiSource::__delete__), &msource);
	Kaba::DeclareClassVirtualIndex("MidiSource", "read", Kaba::mf(&MidiSource::read), &msource);
	Kaba::DeclareClassVirtualIndex("MidiSource", "reset", Kaba::mf(&MidiSource::reset), &msource);
	Kaba::DeclareClassVirtualIndex("MidiSource", "create_panel", Kaba::mf(&MidiSource::create_panel), &msource);
	Kaba::LinkExternal("MidiSource.reset_config", Kaba::mf(&MidiSource::reset_config));
	Kaba::LinkExternal("MidiSource.reset_state", Kaba::mf(&MidiSource::reset_state));
	Kaba::DeclareClassVirtualIndex("MidiSource", "reset_state", Kaba::mf(&MidiSource::reset_state), &msource);
	Kaba::LinkExternal("MidiSource.notify", Kaba::mf(&MidiSource::notify));
	Kaba::DeclareClassVirtualIndex("MidiSource", "on_config", Kaba::mf(&MidiSource::on_config), &msource);

	Synthesizer synth;
	Kaba::DeclareClassSize("Synthesizer", sizeof(Synthesizer));
	Kaba::DeclareClassOffset("Synthesizer", "name", _offsetof(Synthesizer, name));
	Kaba::DeclareClassOffset("Synthesizer", "session", _offsetof(Synthesizer, session));
	Kaba::DeclareClassOffset("Synthesizer", "sample_rate", _offsetof(Synthesizer, sample_rate));
	Kaba::DeclareClassOffset("Synthesizer", "events", _offsetof(Synthesizer, events));
	Kaba::DeclareClassOffset("Synthesizer", "keep_notes", _offsetof(Synthesizer, keep_notes));
	Kaba::DeclareClassOffset("Synthesizer", "active_pitch", _offsetof(Synthesizer, active_pitch));
	Kaba::DeclareClassOffset("Synthesizer", "freq", _offsetof(Synthesizer, tuning.freq));
	Kaba::DeclareClassOffset("Synthesizer", "delta_phi", _offsetof(Synthesizer, delta_phi));
	Kaba::DeclareClassOffset("Synthesizer", "out", _offsetof(Synthesizer, out));
	Kaba::LinkExternal("Synthesizer." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&Synthesizer::__init__));
	Kaba::DeclareClassVirtualIndex("Synthesizer", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&Synthesizer::__delete__), &synth);
	Kaba::DeclareClassVirtualIndex("Synthesizer", "create_panel", Kaba::mf(&Synthesizer::create_panel), &synth);
	Kaba::LinkExternal("Synthesizer.reset_config", Kaba::mf(&Synthesizer::reset_config));
	Kaba::DeclareClassVirtualIndex("Synthesizer", "reset_state", Kaba::mf(&Synthesizer::reset_state), &synth);
	Kaba::LinkExternal("Synthesizer.enable_pitch", Kaba::mf(&Synthesizer::enablePitch));
	Kaba::DeclareClassVirtualIndex("Synthesizer", "render", Kaba::mf(&Synthesizer::render), &synth);
	Kaba::DeclareClassVirtualIndex("Synthesizer", "on_config", Kaba::mf(&Synthesizer::on_config), &synth);
	Kaba::LinkExternal("Synthesizer.set_sample_rate", Kaba::mf(&Synthesizer::setSampleRate));
	Kaba::LinkExternal("Synthesizer.notify", Kaba::mf(&Synthesizer::notify));
	Kaba::LinkExternal("Synthesizer.set_source", Kaba::mf(&Synthesizer::set_source));

	Synthesizer::Output synth_out(NULL);
	Kaba::DeclareClassSize("SynthOutput", sizeof(Synthesizer::Output));
	Kaba::DeclareClassVirtualIndex("SynthOutput", "read", Kaba::mf(&Synthesizer::Output::read), &synth_out);
	Kaba::DeclareClassVirtualIndex("SynthOutput", "reset", Kaba::mf(&Synthesizer::Output::reset), &synth_out);
	Kaba::DeclareClassVirtualIndex("SynthOutput", "get_pos", Kaba::mf(&Synthesizer::Output::get_pos), &synth_out);


	DummySynthesizer dsynth;
	Kaba::DeclareClassSize("DummySynthesizer", sizeof(DummySynthesizer));
	Kaba::LinkExternal("DummySynthesizer." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&DummySynthesizer::__init__));
	Kaba::DeclareClassVirtualIndex("DummySynthesizer", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&DummySynthesizer::__delete__), &dsynth);
	Kaba::DeclareClassVirtualIndex("DummySynthesizer", "render", Kaba::mf(&DummySynthesizer::render), &dsynth);
	Kaba::DeclareClassVirtualIndex("DummySynthesizer", "on_config", Kaba::mf(&DummySynthesizer::on_config), &dsynth);

	Kaba::DeclareClassSize("EnvelopeADSR", sizeof(EnvelopeADSR));
	Kaba::LinkExternal("EnvelopeADSR.set", Kaba::mf(&EnvelopeADSR::set));
	Kaba::LinkExternal("EnvelopeADSR.set2", Kaba::mf(&EnvelopeADSR::set2));
	Kaba::LinkExternal("EnvelopeADSR.reset", Kaba::mf(&EnvelopeADSR::reset));
	Kaba::LinkExternal("EnvelopeADSR.start", Kaba::mf(&EnvelopeADSR::start));
	Kaba::LinkExternal("EnvelopeADSR.end", Kaba::mf(&EnvelopeADSR::end));
	Kaba::LinkExternal("EnvelopeADSR.get", Kaba::mf(&EnvelopeADSR::get));
	Kaba::DeclareClassOffset("EnvelopeADSR", "just_killed", _offsetof(EnvelopeADSR, just_killed));

	Kaba::DeclareClassSize("BarPattern", sizeof(BarPattern));
	Kaba::DeclareClassOffset("BarPattern", "num_beats", _offsetof(BarPattern, num_beats));
	Kaba::DeclareClassOffset("BarPattern", "length", _offsetof(BarPattern, length));
	//Kaba::DeclareClassOffset("BarPattern", "type", _offsetof(BarPattern, type));
	//Kaba::DeclareClassOffset("BarPattern", "count", _offsetof(BarPattern, count));

	Kaba::DeclareClassSize("MidiNote", sizeof(MidiNote));
	Kaba::DeclareClassOffset("MidiNote", "range", _offsetof(MidiNote, range));
	Kaba::DeclareClassOffset("MidiNote", "pitch", _offsetof(MidiNote, pitch));
	Kaba::DeclareClassOffset("MidiNote", "volume", _offsetof(MidiNote, volume));
	Kaba::DeclareClassOffset("MidiNote", "stringno", _offsetof(MidiNote, stringno));
	Kaba::DeclareClassOffset("MidiNote", "clef_position", _offsetof(MidiNote, clef_position));
	Kaba::DeclareClassOffset("MidiNote", "modifier", _offsetof(MidiNote, modifier));

	Kaba::DeclareClassSize("MidiEventBuffer", sizeof(MidiEventBuffer));
	Kaba::DeclareClassOffset("MidiEventBuffer", "samples", _offsetof(MidiEventBuffer, samples));
	Kaba::LinkExternal("MidiEventBuffer." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&MidiEventBuffer::__init__));
	Kaba::LinkExternal("MidiEventBuffer.getEvents", Kaba::mf(&MidiEventBuffer::getEvents));
	Kaba::LinkExternal("MidiEventBuffer.getNotes", Kaba::mf(&MidiEventBuffer::getNotes));
	Kaba::LinkExternal("MidiEventBuffer.getRange", Kaba::mf(&MidiEventBuffer::getRange));
	Kaba::LinkExternal("MidiEventBuffer.addMetronomeClick", Kaba::mf(&MidiEventBuffer::addMetronomeClick));

	Kaba::DeclareClassSize("MidiNoteBuffer", sizeof(MidiNoteBuffer));
	Kaba::DeclareClassOffset("MidiNoteBuffer", "samples", _offsetof(MidiNoteBuffer, samples));
	Kaba::LinkExternal("MidiNoteBuffer." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&MidiNoteBuffer::__init__));
	Kaba::LinkExternal("MidiNoteBuffer.getEvents", Kaba::mf(&MidiNoteBuffer::getEvents));
	Kaba::LinkExternal("MidiNoteBuffer.getNotes", Kaba::mf(&MidiNoteBuffer::getNotes));
	Kaba::LinkExternal("MidiNoteBuffer.getRange", Kaba::mf(&MidiNoteBuffer::range));

	BeatPort bport;
	Kaba::DeclareClassSize("BeatPort", sizeof(BeatPort));
	Kaba::LinkExternal("BeatPort." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&BeatPort::__init__));
	Kaba::DeclareClassVirtualIndex("BeatPort", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&BeatPort::__delete__), &bport);
	Kaba::DeclareClassVirtualIndex("BeatPort", "read", Kaba::mf(&BeatPort::read), &bport);
	Kaba::DeclareClassVirtualIndex("BeatPort", "reset", Kaba::mf(&BeatPort::reset), &bport);

	BeatSource bsource;
	Kaba::DeclareClassSize("BeatSource", sizeof(BeatSource));
	Kaba::DeclareClassOffset("BeatSource", "out", _offsetof(BeatSource, out));
	Kaba::DeclareClassOffset("BeatSource", "session", _offsetof(BeatSource, session));
	Kaba::LinkExternal("BeatSource." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&BeatSource::__init__));
	Kaba::DeclareClassVirtualIndex("BeatSource", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&BeatSource::__delete__), &bsource);
	Kaba::DeclareClassVirtualIndex("BeatSource", "read", Kaba::mf(&BeatSource::read), &bsource);
	Kaba::DeclareClassVirtualIndex("BeatSource", "reset", Kaba::mf(&BeatSource::reset), &bsource);
	Kaba::DeclareClassVirtualIndex("BeatSource", "create_panel", Kaba::mf(&BeatSource::create_panel), &bsource);
	Kaba::LinkExternal("BeatSource.reset_config", Kaba::mf(&BeatSource::reset_config));
	Kaba::DeclareClassVirtualIndex("BeatSource", "reset_state", Kaba::mf(&BeatSource::reset_state), &bsource);
	Kaba::LinkExternal("BeatSource.notify", Kaba::mf(&BeatSource::notify));
	Kaba::DeclareClassVirtualIndex("BeatSource", "on_config", Kaba::mf(&BeatSource::on_config), &bsource);

	Kaba::DeclareClassSize("TrackMarker", sizeof(TrackMarker));
	Kaba::DeclareClassOffset("TrackMarker", "text", _offsetof(TrackMarker, text));
	Kaba::DeclareClassOffset("TrackMarker", "range", _offsetof(TrackMarker, range));
	Kaba::DeclareClassOffset("TrackMarker", "fx", _offsetof(TrackMarker, fx));

	Kaba::DeclareClassSize("TrackLayer", sizeof(TrackLayer));
	Kaba::DeclareClassOffset("TrackLayer", "buffers", _offsetof(TrackLayer, buffers));

	Kaba::DeclareClassSize("Track", sizeof(Track));
	Kaba::DeclareClassOffset("Track", "type", _offsetof(Track, type));
	Kaba::DeclareClassOffset("Track", "name", _offsetof(Track, name));
	Kaba::DeclareClassOffset("Track", "layers", _offsetof(Track, layers));
	Kaba::DeclareClassOffset("Track", "volume", _offsetof(Track, volume));
	Kaba::DeclareClassOffset("Track", "panning", _offsetof(Track, panning));
	Kaba::DeclareClassOffset("Track", "muted", _offsetof(Track, muted));
	Kaba::DeclareClassOffset("Track", "fx", _offsetof(Track, fx));
	Kaba::DeclareClassOffset("Track", "midi", _offsetof(Track, midi));
	Kaba::DeclareClassOffset("Track", "synth", _offsetof(Track, synth));
	Kaba::DeclareClassOffset("Track", "samples", _offsetof(Track, samples));
	Kaba::DeclareClassOffset("Track", "markers", _offsetof(Track, markers));
	Kaba::DeclareClassOffset("Track", "root", _offsetof(Track, song));
	//Kaba::DeclareClassOffset("Track", "is_selected", _offsetof(Track, is_selected));
	Kaba::LinkExternal("Track.getBuffers", Kaba::mf(&Track::getBuffers));
	Kaba::LinkExternal("Track.readBuffers", Kaba::mf(&Track::readBuffers));
	Kaba::LinkExternal("Track.setName", Kaba::mf(&Track::setName));
	Kaba::LinkExternal("Track.setMuted", Kaba::mf(&Track::setMuted));
	Kaba::LinkExternal("Track.setVolume", Kaba::mf(&Track::setVolume));
	Kaba::LinkExternal("Track.setPanning", Kaba::mf(&Track::setPanning));
	Kaba::LinkExternal("Track.insertMidiData", Kaba::mf(&Track::insertMidiData));
	Kaba::LinkExternal("Track.addEffect", Kaba::mf(&Track::addEffect));
	Kaba::LinkExternal("Track.deleteEffect", Kaba::mf(&Track::deleteEffect));
	Kaba::LinkExternal("Track.editEffect", Kaba::mf(&Track::editEffect));
	Kaba::LinkExternal("Track.enableEffect", Kaba::mf(&Track::enableEffect));
	Kaba::LinkExternal("Track.addSampleRef", Kaba::mf(&Track::addSampleRef));
	Kaba::LinkExternal("Track.deleteSampleRef", Kaba::mf(&Track::deleteSampleRef));
	Kaba::LinkExternal("Track.editSampleRef", Kaba::mf(&Track::editSampleRef));
	Kaba::LinkExternal("Track.addMidiNote", Kaba::mf(&Track::addMidiNote));
	//Kaba::LinkExternal("Track.addMidiNotes", Kaba::mf(&Track::addMidiNotes));
	Kaba::LinkExternal("Track.deleteMidiNote", Kaba::mf(&Track::deleteMidiNote));
	Kaba::LinkExternal("Track.setSynthesizer", Kaba::mf(&Track::setSynthesizer));
	Kaba::LinkExternal("Track.addMarker", Kaba::mf(&Track::addMarker));
	Kaba::LinkExternal("Track.deleteMarker", Kaba::mf(&Track::deleteMarker));
	Kaba::LinkExternal("Track.editMarker", Kaba::mf(&Track::editMarker));

	Song af = Song(Session::GLOBAL);
	Kaba::DeclareClassSize("Song", sizeof(Song));
	Kaba::DeclareClassOffset("Song", "filename", _offsetof(Song, filename));
	Kaba::DeclareClassOffset("Song", "tag", _offsetof(Song, tags));
	Kaba::DeclareClassOffset("Song", "sample_rate", _offsetof(Song, sample_rate));
	Kaba::DeclareClassOffset("Song", "volume", _offsetof(Song, volume));
	Kaba::DeclareClassOffset("Song", "fx", _offsetof(Song, fx));
	Kaba::DeclareClassOffset("Song", "tracks", _offsetof(Song, tracks));
	Kaba::DeclareClassOffset("Song", "samples", _offsetof(Song, samples));
	Kaba::DeclareClassOffset("Song", "layers", _offsetof(Song, layers));
	Kaba::DeclareClassOffset("Song", "bars", _offsetof(Song, bars));
	Kaba::LinkExternal("Song." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&Song::__init__));
	Kaba::DeclareClassVirtualIndex("Song", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&Song::__delete__), &af);
	Kaba::LinkExternal("Song.newEmpty", Kaba::mf(&Song::newEmpty));
	Kaba::LinkExternal("Song.addTrack", Kaba::mf(&Song::addTrack));
	Kaba::LinkExternal("Song.deleteTrack", Kaba::mf(&Song::deleteTrack));
	Kaba::LinkExternal("Song.getRange", Kaba::mf(&Song::getRange));
	Kaba::LinkExternal("Song.addBar", Kaba::mf(&Song::addBar));
	Kaba::LinkExternal("Song.addPause", Kaba::mf(&Song::addPause));
	Kaba::LinkExternal("Song.editBar", Kaba::mf(&Song::editBar));
	Kaba::LinkExternal("Song.deleteBar", Kaba::mf(&Song::deleteBar));
	Kaba::LinkExternal("Song.addSample", Kaba::mf(&Song::addSample));
	Kaba::LinkExternal("Song.deleteSample", Kaba::mf(&Song::deleteSample));

	AudioPort aport;
	Kaba::DeclareClassSize("AudioPort", sizeof(AudioPort));
	Kaba::LinkExternal("AudioPort." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&AudioPort::__init__));
	Kaba::DeclareClassVirtualIndex("AudioPort", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&AudioPort::__delete__), &aport);
	Kaba::DeclareClassVirtualIndex("AudioPort", "read", Kaba::mf(&AudioPort::read), &aport);
	Kaba::DeclareClassVirtualIndex("AudioPort", "reset", Kaba::mf(&AudioPort::reset), &aport);
	Kaba::DeclareClassVirtualIndex("AudioPort", "get_pos", Kaba::mf(&AudioPort::get_pos), &aport);

	SongRenderer sr(&af);
	Kaba::DeclareClassSize("SongRenderer", sizeof(SongRenderer));
	Kaba::LinkExternal("SongRenderer.prepare", Kaba::mf(&SongRenderer::prepare));
	Kaba::LinkExternal("SongRenderer.render", Kaba::mf(&SongRenderer::render));
	Kaba::LinkExternal("SongRenderer." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&SongRenderer::__init__));
	Kaba::DeclareClassVirtualIndex("SongRenderer", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&SongRenderer::__delete__), &sr);
	Kaba::DeclareClassVirtualIndex("SongRenderer", "read", Kaba::mf(&SongRenderer::read), &sr);
	Kaba::DeclareClassVirtualIndex("SongRenderer", "reset", Kaba::mf(&SongRenderer::reset), &sr);
	Kaba::DeclareClassVirtualIndex("SongRenderer", "get_pos", Kaba::mf(&SongRenderer::get_pos), &sr);
	Kaba::LinkExternal("SongRenderer.range", Kaba::mf(&SongRenderer::range));
	Kaba::LinkExternal("SongRenderer.seek", Kaba::mf(&SongRenderer::seek));

	{
	InputStreamAudio input(Session::GLOBAL);
	Kaba::DeclareClassSize("InputStreamAudio", sizeof(InputStreamAudio));
	Kaba::DeclareClassOffset("InputStreamAudio", "session", _offsetof(InputStreamAudio, session));
	Kaba::DeclareClassOffset("InputStreamAudio", "current_buffer", _offsetof(InputStreamAudio, buffer));
	Kaba::DeclareClassOffset("InputStreamAudio", "out", _offsetof(InputStreamAudio, out));
	Kaba::DeclareClassOffset("InputStreamAudio", "capturing", _offsetof(InputStreamAudio, capturing));
	Kaba::LinkExternal("InputStreamAudio." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&InputStreamAudio::__init__));
	Kaba::DeclareClassVirtualIndex("InputStreamAudio", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&InputStreamAudio::__delete__), &input);
	Kaba::LinkExternal("InputStreamAudio.start", Kaba::mf(&InputStreamAudio::start));
	Kaba::LinkExternal("InputStreamAudio.stop",	 Kaba::mf(&InputStreamAudio::stop));
	Kaba::LinkExternal("InputStreamAudio.is_capturing", Kaba::mf(&InputStreamAudio::is_capturing));
	Kaba::LinkExternal("InputStreamAudio.subscribe", Kaba::mf(&InputStreamAudio::subscribe_kaba));
	Kaba::LinkExternal("InputStreamAudio.unsubscribe", Kaba::mf(&InputStreamAudio::unsubscribe));
	Kaba::LinkExternal("InputStreamAudio.sample_rate", Kaba::mf(&InputStreamAudio::sample_rate));
	Kaba::LinkExternal("InputStreamAudio.set_backup_mode", Kaba::mf(&InputStreamAudio::set_backup_mode));
	}

	{
	OutputStream stream(Session::GLOBAL, NULL);
	Kaba::DeclareClassSize("OutputStream", sizeof(OutputStream));
	Kaba::LinkExternal("OutputStream." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&OutputStream::__init__));
	Kaba::DeclareClassVirtualIndex("OutputStream", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&OutputStream::__delete__), &stream);
	//Kaba::LinkExternal("OutputStream.setSource", Kaba::mf(&AudioStream::setSource));
	Kaba::LinkExternal("OutputStream.play", Kaba::mf(&OutputStream::play));
	Kaba::LinkExternal("OutputStream.stop", Kaba::mf(&OutputStream::stop));
	Kaba::LinkExternal("OutputStream.pause", Kaba::mf(&OutputStream::pause));
	Kaba::LinkExternal("OutputStream.is_paused", Kaba::mf(&OutputStream::is_paused));
	Kaba::LinkExternal("OutputStream.get_pos", Kaba::mf(&OutputStream::get_pos));
	//Kaba::LinkExternal("OutputStream.sample_rate", Kaba::mf(&OutputStream::sample_rate));
	Kaba::LinkExternal("OutputStream.get_volume", Kaba::mf(&OutputStream::get_volume));
	Kaba::LinkExternal("OutputStream.set_volume", Kaba::mf(&OutputStream::set_volume));
	Kaba::LinkExternal("OutputStream.set_buffer_size", Kaba::mf(&OutputStream::set_buffer_size));
//	Kaba::DeclareClassVirtualIndex("OutputStream", "", Kaba::mf(&OutputStream::__delete__), &stream);
	Kaba::LinkExternal("OutputStream.subscribe", Kaba::mf(&OutputStream::subscribe_kaba));
	Kaba::LinkExternal("OutputStream.unsubscribe", Kaba::mf(&OutputStream::unsubscribe));
	}

	Kaba::DeclareClassSize("AudioView", sizeof(AudioView));
	Kaba::DeclareClassOffset("AudioView", "sel", _offsetof(AudioView, sel));
	Kaba::DeclareClassOffset("AudioView", "stream", _offsetof(AudioView, stream));
	Kaba::DeclareClassOffset("AudioView", "renderer", _offsetof(AudioView, renderer));
	//Kaba::DeclareClassOffset("AudioView", "input", _offsetof(AudioView, input));
	Kaba::LinkExternal("AudioView.subscribe", Kaba::mf(&AudioView::subscribe_kaba));
	Kaba::LinkExternal("AudioView.unsubscribe", Kaba::mf(&AudioView::unsubscribe));

	Kaba::DeclareClassSize("ColorScheme", sizeof(ColorScheme));
	Kaba::DeclareClassOffset("ColorScheme", "background", _offsetof(ColorScheme, background));
	Kaba::DeclareClassOffset("ColorScheme", "background_track", _offsetof(ColorScheme, background_track));
	Kaba::DeclareClassOffset("ColorScheme", "background_track_selected", _offsetof(ColorScheme, background_track_selected));
	Kaba::DeclareClassOffset("ColorScheme", "text", _offsetof(ColorScheme, text));
	Kaba::DeclareClassOffset("ColorScheme", "text_soft1", _offsetof(ColorScheme, text_soft1));
	Kaba::DeclareClassOffset("ColorScheme", "text_soft2", _offsetof(ColorScheme, text_soft2));
	Kaba::DeclareClassOffset("ColorScheme", "text_soft3", _offsetof(ColorScheme, text_soft3));
	Kaba::DeclareClassOffset("ColorScheme", "grid", _offsetof(ColorScheme, grid));
	Kaba::DeclareClassOffset("ColorScheme", "selection", _offsetof(ColorScheme, selection));
	Kaba::DeclareClassOffset("ColorScheme", "hover", _offsetof(ColorScheme, hover));

	Kaba::LinkExternal("Storage.load", Kaba::mf(&Storage::load));
	Kaba::LinkExternal("Storage.save", Kaba::mf(&Storage::save));
	Kaba::DeclareClassOffset("Storage", "current_directory", _offsetof(Storage, current_directory));


	Slider slider;
	Kaba::DeclareClassSize("Slider", sizeof(Slider));
	Kaba::LinkExternal("Slider." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&Slider::__init_ext__));
	Kaba::DeclareClassVirtualIndex("Slider", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&Slider::__delete__), &slider);
	Kaba::LinkExternal("Slider.get", Kaba::mf(&Slider::get));
	Kaba::LinkExternal("Slider.set", Kaba::mf(&Slider::set));

	SongPlugin song_plugin;
	Kaba::DeclareClassSize("SongPlugin", sizeof(SongPlugin));
	Kaba::DeclareClassOffset("SongPlugin", "session", _offsetof(SongPlugin, session));
	Kaba::DeclareClassOffset("SongPlugin", "song", _offsetof(SongPlugin, song));
	Kaba::LinkExternal("SongPlugin." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&SongPlugin::__init__));
	Kaba::DeclareClassVirtualIndex("SongPlugin", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&SongPlugin::__delete__), &song_plugin);
	Kaba::DeclareClassVirtualIndex("SongPlugin", "apply", Kaba::mf(&SongPlugin::apply), &song_plugin);

	TsunamiPlugin tsunami_plugin;
	Kaba::DeclareClassSize("TsunamiPlugin", sizeof(TsunamiPlugin));
	Kaba::DeclareClassOffset("TsunamiPlugin", "session", _offsetof(TsunamiPlugin, session));
	Kaba::DeclareClassOffset("TsunamiPlugin", "song", _offsetof(TsunamiPlugin, song));
	Kaba::DeclareClassOffset("TsunamiPlugin", "args", _offsetof(TsunamiPlugin, args));
	Kaba::LinkExternal("TsunamiPlugin." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&TsunamiPlugin::__init__));
	Kaba::DeclareClassVirtualIndex("TsunamiPlugin", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&TsunamiPlugin::__delete__), &tsunami_plugin);
	Kaba::DeclareClassVirtualIndex("TsunamiPlugin", "onStart", Kaba::mf(&TsunamiPlugin::onStart), &tsunami_plugin);
	Kaba::DeclareClassVirtualIndex("TsunamiPlugin", "onStop", Kaba::mf(&TsunamiPlugin::onStop), &tsunami_plugin);
	Kaba::LinkExternal("TsunamiPlugin.stop", Kaba::mf(&TsunamiPlugin::stop_request));
}

void get_plugin_file_data(PluginManager::PluginFile &pf)
{
	pf.image = "";
	try{
		string content = FileRead(pf.filename);
		int p = content.find("// Image = hui:");
		if (p >= 0)
			pf.image = content.substr(p + 11, content.find("\n") - p - 11);
	}catch(...){}
}

void find_plugins_in_dir(const string &dir, int type, PluginManager *pm)
{
	Array<DirEntry> list = dir_search(pm->plugin_dir() + dir, "*.kaba", false);
	for (DirEntry &e : list){
		PluginManager::PluginFile pf;
		pf.type = type;
		pf.name = e.name.replace(".kaba", "");
		pf.filename = pm->plugin_dir() + dir + e.name;
		get_plugin_file_data(pf);
		pm->plugin_files.add(pf);
	}
}

void add_plugins_in_dir(const string &dir, PluginManager *pm, hui::Menu *m, const string &name_space, TsunamiWindow *win, void (TsunamiWindow::*function)())
{
	for (PluginManager::PluginFile &f: pm->plugin_files){
		if (f.filename.find(dir) >= 0){
			string id = "execute-" + name_space + "--" + f.name;
            m->addItemImage(f.name, f.image, id);
            win->event(id, std::bind(function, win));
		}
	}
}

void PluginManager::FindPlugins()
{
	Kaba::Init();

	// "AudioSource"
	find_plugins_in_dir("AudioSource/", Plugin::Type::AUDIO_SOURCE, this);

	// "AudioEffect"
	find_plugins_in_dir("AudioEffect/Channels/", Plugin::Type::AUDIO_EFFECT, this);
	find_plugins_in_dir("AudioEffect/Dynamics/", Plugin::Type::AUDIO_EFFECT, this);
	find_plugins_in_dir("AudioEffect/Echo/", Plugin::Type::AUDIO_EFFECT, this);
	find_plugins_in_dir("AudioEffect/Pitch/", Plugin::Type::AUDIO_EFFECT, this);
	find_plugins_in_dir("AudioEffect/Repair/", Plugin::Type::AUDIO_EFFECT, this);
	find_plugins_in_dir("AudioEffect/Sound/", Plugin::Type::AUDIO_EFFECT, this);
	find_plugins_in_dir("AudioEffect/Synthesizer/", Plugin::Type::AUDIO_EFFECT, this);

	// "MidiSource"
	find_plugins_in_dir("MidiSource/", Plugin::Type::MIDI_SOURCE, this);

	// "MidiEffect"
	find_plugins_in_dir("MidiEffect/", Plugin::Type::MIDI_EFFECT, this);

	// "BeatSource"
	find_plugins_in_dir("BeatSource/", Plugin::Type::BEAT_SOURCE, this);

	// "All"
	find_plugins_in_dir("All/", Plugin::Type::SONG_PLUGIN, this);

	// rest
	find_plugins_in_dir("Independent/", Plugin::Type::TSUNAMI_PLUGIN, this);

	// "Synthesizer"
	find_plugins_in_dir("Synthesizer/", Plugin::Type::SYNTHESIZER, this);
}

void PluginManager::AddPluginsToMenu(TsunamiWindow *win)
{
	hui::Menu *m = win->getMenu();

	// "Buffer"
	add_plugins_in_dir("AudioEffect/Channels/", this, m->getSubMenuByID("menu_plugins_channels"), "effect", win, &TsunamiWindow::onMenuExecuteEffect);
	add_plugins_in_dir("AudioEffect/Dynamics/", this, m->getSubMenuByID("menu_plugins_dynamics"), "effect", win, &TsunamiWindow::onMenuExecuteEffect);
	add_plugins_in_dir("AudioEffect/Echo/", this, m->getSubMenuByID("menu_plugins_echo"), "effect", win, &TsunamiWindow::onMenuExecuteEffect);
	add_plugins_in_dir("AudioEffect/Pitch/", this, m->getSubMenuByID("menu_plugins_pitch"), "effect", win, &TsunamiWindow::onMenuExecuteEffect);
	add_plugins_in_dir("AudioEffect/Repair/", this, m->getSubMenuByID("menu_plugins_repair"), "effect", win, &TsunamiWindow::onMenuExecuteEffect);
	add_plugins_in_dir("AudioEffect/Sound/", this, m->getSubMenuByID("menu_plugins_sound"), "effect", win, &TsunamiWindow::onMenuExecuteEffect);
	add_plugins_in_dir("AudioEffect/Synthesizer/", this, m->getSubMenuByID("menu_plugins_synthesizer"), "effect", win, &TsunamiWindow::onMenuExecuteEffect);

	// "Midi"
	add_plugins_in_dir("MidiEffect/", this, m->getSubMenuByID("menu_plugins_on_midi"), "midi-effect", win, &TsunamiWindow::onMenuExecuteMidiEffect);

	// "All"
	add_plugins_in_dir("All/", this, m->getSubMenuByID("menu_plugins_on_all"), "song", win, &TsunamiWindow::onMenuExecuteSongPlugin);

	// rest
	add_plugins_in_dir("Independent/", this, m->getSubMenuByID("menu_plugins_other"), "tsunami", win, &TsunamiWindow::onMenuExecuteTsunamiPlugin);
}

void PluginManager::ApplyFavorite(Module *c, const string &name)
{
	favorites->Apply(c, name);
}

void PluginManager::SaveFavorite(Module *c, const string &name)
{
	favorites->Save(c, name);
}


string PluginManager::SelectFavoriteName(hui::Window *win, Module *c, bool save)
{
	return favorites->SelectName(win, c, save);
}

// always push the script... even if an error occurred
//   don't log error...
Plugin *PluginManager::LoadAndCompilePlugin(int type, const string &filename)
{
	for (Plugin *p: plugins)
		if (filename == p->filename)
			return p;

	//InitPluginData();

	Plugin *p = new Plugin(filename, type);
	p->index = plugins.num;

	plugins.add(p);

	return p;
}

typedef void main_void_func();

void PluginManager::_ExecutePlugin(Session *session, const string &filename)
{
	Plugin *p = LoadAndCompilePlugin(Plugin::Type::OTHER, filename);
	if (!p->usable){
		session->e(p->get_error());
		return;
	}

	Kaba::Script *s = p->s;

	main_void_func *f_main = (main_void_func*)s->MatchFunction("main", "void", 0);
	if (f_main){
		f_main();
	}else{
		session->e(_("Plugin does not contain a function 'void main()'"));
	}
}


Plugin *PluginManager::GetPlugin(Session *session, int type, const string &name)
{
	for (PluginFile &pf: plugin_files){
		if ((pf.name == name) and (pf.type == type)){
			Plugin *p = LoadAndCompilePlugin(type, pf.filename);
			if (!p->usable)
				session->e(p->get_error());
			return p;
		}
	}
	session->e(format(_("Can't find plugin: %s ..."), name.c_str()));
	return NULL;
}


Array<string> PluginManager::FindSynthesizers()
{
	Array<string> names;
	Array<DirEntry> list = dir_search(plugin_dir() + "Synthesizer/", "*.kaba", false);
	for (DirEntry &e: list)
		names.add(e.name.replace(".kaba", ""));
	names.add("Dummy");
	//names.add("Sample");
	return names;
}

Synthesizer *PluginManager::__LoadSynthesizer(Session *session, const string &name)
{
	string filename = plugin_dir() + "Synthesizer/" + name + ".kaba";
	if (!file_test_existence(filename))
		return NULL;
	Kaba::Script *s;
	try{
		s = Kaba::Load(filename);
	}catch(Kaba::Exception &e){
		session->e(e.message);
		return NULL;
	}
	for (auto *t : s->syntax->classes){
		if (t->get_root()->name != "Synthesizer")
			continue;
		Synthesizer *synth = (Synthesizer*)t->create_instance();
		synth->session = session;
		synth->setSampleRate(session->song->sample_rate);
		return synth;
	}
	return NULL;
}
// factory
Synthesizer *PluginManager::CreateSynthesizer(Session *session, const string &name)
{
	if ((name == "Dummy") or (name == ""))
		return new DummySynthesizer;
	/*if (name == "Sample")
		return new SampleSynthesizer;*/
	Synthesizer *s = __LoadSynthesizer(session, name);
	if (s){
		s->name = name;
		s->session = session;
		s->reset_config();
		return s;
	}
	session->e(_("unknown synthesizer: ") + name);
	s = new DummySynthesizer;
	s->session = session;
	s->name = name;
	return s;
}


string PluginManager::plugin_dir()
{
	if (tsunami->directory_static.find("/home/") == 0)
		return "Plugins/";
	return tsunami->directory_static + "Plugins/";
}

Array<string> PluginManager::FindAudioSources()
{
	Array<string> names;
	for (auto &pf: plugin_files)
		if (pf.type == Plugin::Type::AUDIO_SOURCE)
			names.add(pf.name);
	return names;
}

Array<string> PluginManager::FindAudioEffects()
{
	Array<string> names;
	string prefix = plugin_dir() + "AudioEffect/";
	for (auto &pf: plugin_files){
		if (pf.filename.match(prefix + "*")){
			string g = pf.filename.substr(prefix.num, -1).explode("/")[0];
			names.add(g + "/" + pf.name);
		}
	}
	return names;
}

Array<string> PluginManager::FindMidiEffects()
{
	Array<string> names;
	for (auto &pf: plugin_files)
		if (pf.type == Plugin::Type::MIDI_EFFECT)
			names.add(pf.name);
	return names;
}

Array<string> PluginManager::FindMidiSources()
{
	Array<string> names;
	for (auto &pf: plugin_files)
		if (pf.type == Plugin::Type::MIDI_SOURCE)
			names.add(pf.name);
	return names;
}

Array<string> PluginManager::FindBeatSources()
{
	Array<string> names;
	for (auto &pf: plugin_files)
		if (pf.type == Plugin::Type::BEAT_SOURCE)
			names.add(pf.name);
	return names;
}

Array<string> PluginManager::FindConfigurable(int type)
{
	if (type == Module::Type::AUDIO_SOURCE)
		return FindAudioSources();
	if (type == Module::Type::AUDIO_EFFECT)
		return FindAudioEffects();
	if (type == Module::Type::MIDI_SOURCE)
		return FindMidiSources();
	if (type == Module::Type::MIDI_EFFECT)
		return FindMidiEffects();
	if (type == Module::Type::SYNTHESIZER)
		return FindSynthesizers();
	if (type == Module::Type::BEAT_SOURCE)
		return FindBeatSources();
	return Array<string>();
}


AudioEffect* PluginManager::ChooseEffect(hui::Panel *parent, Session *session)
{
	ConfigurableSelectorDialog *dlg = new ConfigurableSelectorDialog(parent->win, Module::Type::AUDIO_EFFECT, session);
	dlg->run();
	AudioEffect *e = NULL;
	if (dlg->_return.num > 0)
		e = CreateAudioEffect(session, dlg->_return);
	delete(dlg);
	return e;
}

MidiEffect* PluginManager::ChooseMidiEffect(hui::Panel *parent, Session *session)
{
	ConfigurableSelectorDialog *dlg = new ConfigurableSelectorDialog(parent->win, Module::Type::MIDI_EFFECT, session);
	dlg->run();
	MidiEffect *e = NULL;
	if (dlg->_return.num > 0)
		e = CreateMidiEffect(session, dlg->_return);
	delete(dlg);
	return e;
}


Synthesizer *PluginManager::ChooseSynthesizer(hui::Window *parent, Session *session, const string &old_name)
{
	ConfigurableSelectorDialog *dlg = new ConfigurableSelectorDialog(parent, Module::Type::SYNTHESIZER, session, old_name);
	dlg->run();
	Synthesizer *s = NULL;
	if (dlg->_return.num > 0)
		s = CreateSynthesizer(session, dlg->_return);
	delete(dlg);
	return s;
}

/*Synthesizer* PluginManager::ChooseSynthesizer(HuiPanel *parent)
{
	string name = ChooseConfigurable(parent, Module::Type::SYNTHESIZER);
	if (name == "")
		return NULL;
	return CreateSynthesizer(name);
}*/


