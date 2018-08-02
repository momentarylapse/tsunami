/*
 * PluginManager.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "PluginManager.h"

#include "../Tsunami.h"
#include "../TsunamiWindow.h"
#include "../Session.h"
#include "FastFourierTransform.h"
#include "../View/Helper/Slider.h"
#include "../Data/base.h"
#include "../Data/Song.h"
#include "../Data/Track.h"
#include "../Data/Sample.h"
#include "../Data/SampleRef.h"
#include "../Data/Audio/AudioBuffer.h"
#include "../Data/Rhythm/Bar.h"
#include "../Module/ConfigPanel.h"
#include "../Device/InputStreamAudio.h"
#include "../Device/InputStreamMidi.h"
#include "../Device/OutputStream.h"
#include "../Device/DeviceManager.h"
#include "../Module/Synth/Synthesizer.h"
#include "../Module/Synth/DummySynthesizer.h"
#include "../Module/Port/AudioPort.h"
#include "../Module/Audio/AudioSource.h"
#include "../Module/Audio/SongRenderer.h"
#include "../Module/Audio/AudioVisualizer.h"
#include "../Module/Port/MidiPort.h"
#include "../Module/Midi/MidiSource.h"
#include "../Module/Audio/AudioEffect.h"
#include "../Module/Beats/BeatSource.h"
#include "../Module/Beats/BeatMidifier.h"
#include "../Module/Midi/MidiEffect.h"
#include "../Module/Port/BeatPort.h"
#include "../View/Helper/Progress.h"
#include "../Storage/Storage.h"
#include "../View/AudioView.h"
#include "../View/Dialog/ConfigurableSelectorDialog.h"
#include "../View/SideBar/SampleManagerConsole.h"
#include "../View/Mode/ViewModeCapture.h"
#include "Plugin.h"
#include "ExtendedAudioBuffer.h"
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

void PluginManager::LinkAppScriptData()
{
	Kaba::config.directory = "";

	// api definition
	Kaba::LinkExternal("device_manager", &tsunami->device_manager);
	Kaba::LinkExternal("colors", &AudioView::colors);
	Kaba::LinkExternal("view_input", &export_view_input);
	Kaba::LinkExternal("fft_c2c", (void*)&FastFourierTransform::fft_c2c);
	Kaba::LinkExternal("fft_r2c", (void*)&FastFourierTransform::fft_r2c);
	Kaba::LinkExternal("fft_c2r_inv", (void*)&FastFourierTransform::fft_c2r_inv);
	Kaba::LinkExternal("CreateSynthesizer", (void*)&CreateSynthesizer);
	Kaba::LinkExternal("CreateAudioEffect", (void*)&CreateAudioEffect);
	Kaba::LinkExternal("CreateAudioSource", (void*)&CreateAudioSource);
	Kaba::LinkExternal("CreateMidiEffect", (void*)&CreateMidiEffect);
	Kaba::LinkExternal("CreateMidiSource", (void*)&CreateMidiSource);
	Kaba::LinkExternal("CreateBeatMidifier", (void*)&CreateBeatMidifier);
	Kaba::LinkExternal("CreateBeatSource", (void*)&CreateBeatSource);
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
	Kaba::DeclareClassOffset("Session", "song_renderer", _offsetof(Session, song_renderer));
	Kaba::DeclareClassOffset("Session", "output_stream", _offsetof(Session, output_stream));

	Kaba::LinkExternal("Session.sample_rate", Kaba::mf(&Session::sample_rate));
	Kaba::LinkExternal("Session.i", Kaba::mf(&Session::i));
	Kaba::LinkExternal("Session.w", Kaba::mf(&Session::w));
	Kaba::LinkExternal("Session.e", Kaba::mf(&Session::e));


	Module module(ModuleType::AUDIO_EFFECT);
	Kaba::DeclareClassSize("Module", sizeof(Module));
	Kaba::DeclareClassOffset("Module", "name", _offsetof(Module, module_subtype));
	Kaba::DeclareClassOffset("Module", "usable", _offsetof(Module, usable));
	Kaba::DeclareClassOffset("Module", "session", _offsetof(Module, session));
	Kaba::LinkExternal("Module." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&Module::__init__));
	Kaba::DeclareClassVirtualIndex("Module", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&Module::__delete__), &module);
	Kaba::DeclareClassVirtualIndex("Module", "create_panel", Kaba::mf(&Module::create_panel), &module);
	Kaba::LinkExternal("Module.reset_config", Kaba::mf(&Module::reset_config));
	Kaba::DeclareClassVirtualIndex("Module", "reset_state", Kaba::mf(&Module::reset_state), &module);
	Kaba::LinkExternal("Module.changed", Kaba::mf(&Module::changed));
	Kaba::DeclareClassVirtualIndex("Module", "get_config", Kaba::mf(&Module::get_config), &module);
	Kaba::DeclareClassVirtualIndex("Module", "config_to_string", Kaba::mf(&Module::config_to_string), &module);
	Kaba::DeclareClassVirtualIndex("Module", "config_from_string", Kaba::mf(&Module::config_from_string), &module);
	Kaba::DeclareClassVirtualIndex("Module", "on_config", Kaba::mf(&Module::on_config), &module);
	Kaba::DeclareClassVirtualIndex("Module", "module_start", Kaba::mf(&Module::module_start), &module);
	Kaba::DeclareClassVirtualIndex("Module", "module_stop", Kaba::mf(&Module::module_stop), &module);
	Kaba::DeclareClassVirtualIndex("Module", "module_pause", Kaba::mf(&Module::module_pause), &module);
	Kaba::LinkExternal("Module.subscribe", Kaba::mf(&Module::subscribe_kaba));
	Kaba::LinkExternal("Module.unsubscribe", Kaba::mf(&Module::unsubscribe));


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
	Kaba::LinkExternal("ConfigPanel.changed", Kaba::mf(&ConfigPanel::changed));
	Kaba::DeclareClassOffset("ConfigPanel", "c", _offsetof(ConfigPanel, c));

	AudioSource asource;
	Kaba::DeclareClassSize("AudioSource", sizeof(AudioSource));
	Kaba::LinkExternal("AudioSource." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&AudioSource::__init__));
	Kaba::DeclareClassVirtualIndex("AudioSource", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&AudioSource::__delete__), &asource);
	Kaba::DeclareClassVirtualIndex("AudioSource", "read", Kaba::mf(&AudioSource::read), &asource);
	Kaba::DeclareClassVirtualIndex("AudioSource", "reset", Kaba::mf(&AudioSource::reset), &asource);
	Kaba::DeclareClassVirtualIndex("AudioSource", "get_pos", Kaba::mf(&AudioSource::get_pos), &asource);

	AudioEffect aeffect;
	Kaba::DeclareClassSize("AudioEffect", sizeof(AudioEffect));
	Kaba::DeclareClassOffset("AudioEffect", "sample_rate", _offsetof(AudioEffect, sample_rate));
	Kaba::LinkExternal("AudioEffect." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&AudioEffect::__init__));
	Kaba::DeclareClassVirtualIndex("AudioEffect", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&AudioEffect::__delete__), &aeffect);
	Kaba::DeclareClassVirtualIndex("AudioEffect", "process", Kaba::mf(&AudioEffect::process), &aeffect);

	MidiEffect meffect;
	Kaba::DeclareClassSize("MidiEffect", sizeof(MidiEffect));
	Kaba::DeclareClassOffset("MidiEffect", "only_on_selection", _offsetof(MidiEffect, only_on_selection));
	Kaba::DeclareClassOffset("MidiEffect", "range", _offsetof(MidiEffect, range));
	Kaba::LinkExternal("MidiEffect." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&MidiEffect::__init__));
	Kaba::DeclareClassVirtualIndex("MidiEffect", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&MidiEffect::__delete__), &meffect);
	Kaba::DeclareClassVirtualIndex("MidiEffect", "process", Kaba::mf(&MidiEffect::process), &meffect);
	Kaba::LinkExternal("MidiEffect.note", Kaba::mf(&MidiEffect::note));
	Kaba::LinkExternal("MidiEffect.skip", Kaba::mf(&MidiEffect::skip));
	Kaba::LinkExternal("MidiEffect.note_x", Kaba::mf(&MidiEffect::note_x));
	Kaba::LinkExternal("MidiEffect.skip_x", Kaba::mf(&MidiEffect::skip_x));

	AudioVisualizer avis;
	Kaba::DeclareClassSize("AudioVisualizer", sizeof(AudioVisualizer));
	Kaba::DeclareClassOffset("AudioVisualizer", "chunk_size", _offsetof(AudioVisualizer, chunk_size));
	Kaba::LinkExternal("AudioVisualizer." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&AudioVisualizer::__init__));
	Kaba::DeclareClassVirtualIndex("AudioVisualizer", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&AudioVisualizer::__delete__), &avis);
	Kaba::DeclareClassVirtualIndex("AudioVisualizer", "process", Kaba::mf(&AudioVisualizer::process), &avis);
	Kaba::DeclareClassVirtualIndex("AudioVisualizer", "reset", Kaba::mf(&AudioVisualizer::reset), &avis);
	Kaba::LinkExternal("AudioVisualizer.set_chunk_size", Kaba::mf(&AudioVisualizer::set_chunk_size));

	Kaba::DeclareClassSize("AudioBuffer", sizeof(AudioBuffer));
	Kaba::DeclareClassOffset("AudioBuffer", "offset", _offsetof(AudioBuffer, offset));
	Kaba::DeclareClassOffset("AudioBuffer", "length", _offsetof(AudioBuffer, length));
	Kaba::DeclareClassOffset("AudioBuffer", "channels", _offsetof(AudioBuffer, channels));
	Kaba::DeclareClassOffset("AudioBuffer", "c", _offsetof(AudioBuffer, c));
	Kaba::DeclareClassOffset("AudioBuffer", "r", _offsetof(AudioBuffer, c[0]));
	Kaba::DeclareClassOffset("AudioBuffer", "l", _offsetof(AudioBuffer, c[1]));
	Kaba::DeclareClassOffset("AudioBuffer", "peaks", _offsetof(AudioBuffer, peaks));
	Kaba::LinkExternal("AudioBuffer." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&AudioBuffer::__init__));
	Kaba::LinkExternal("AudioBuffer." + Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&AudioBuffer::__delete__));
	Kaba::LinkExternal("AudioBuffer.clear", Kaba::mf(&AudioBuffer::clear));
	Kaba::LinkExternal("AudioBuffer." + Kaba::IDENTIFIER_FUNC_ASSIGN, Kaba::mf(&AudioBuffer::__assign__));
	Kaba::LinkExternal("AudioBuffer.range", Kaba::mf(&AudioBuffer::range));
	Kaba::LinkExternal("AudioBuffer.resize", Kaba::mf(&AudioBuffer::resize));
	Kaba::LinkExternal("AudioBuffer.add", Kaba::mf(&AudioBuffer::add));
	Kaba::LinkExternal("AudioBuffer.set", Kaba::mf(&AudioBuffer::set));
	Kaba::LinkExternal("AudioBuffer.get_spectrum", Kaba::mf(&ExtendedAudioBuffer::get_spectrum));


	Kaba::DeclareClassSize("RingBuffer", sizeof(RingBuffer));
	//Kaba::LinkExternal("RingBuffer." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&RingBuffer::__init__));
	Kaba::LinkExternal("RingBuffer.available", Kaba::mf(&RingBuffer::available));
	Kaba::LinkExternal("RingBuffer.read", Kaba::mf(&RingBuffer::read));
	Kaba::LinkExternal("RingBuffer.write", Kaba::mf(&RingBuffer::write));
	Kaba::LinkExternal("RingBuffer.read_ref", Kaba::mf(&RingBuffer::read_ref));
	Kaba::LinkExternal("RingBuffer.read_ref_done", Kaba::mf(&RingBuffer::read_ref_done));
	Kaba::LinkExternal("RingBuffer.peek_ref", Kaba::mf(&RingBuffer::peek_ref));
	Kaba::LinkExternal("RingBuffer.write_ref", Kaba::mf(&RingBuffer::write_ref));
	Kaba::LinkExternal("RingBuffer.write_ref_done", Kaba::mf(&RingBuffer::write_ref_done));
//	Kaba::LinkExternal("RingBuffer.move_read_pos", Kaba::mf(&RingBuffer::move_read_pos));
//	Kaba::LinkExternal("RingBuffer.move_write_pos", Kaba::mf(&RingBuffer::move_write_pos));
	Kaba::LinkExternal("RingBuffer.clear", Kaba::mf(&RingBuffer::clear));

	Kaba::DeclareClassSize("Sample", sizeof(Sample));
	Kaba::DeclareClassOffset("Sample", "name", _offsetof(Sample, name));
	Kaba::DeclareClassOffset("Sample", "type", _offsetof(Sample, type));
	Kaba::DeclareClassOffset("Sample", "buf", _offsetof(Sample, buf));
	Kaba::DeclareClassOffset("Sample", "midi", _offsetof(Sample, midi));
	Kaba::DeclareClassOffset("Sample", "volume", _offsetof(Sample, volume));
	Kaba::DeclareClassOffset("Sample", "uid", _offsetof(Sample, uid));
	Kaba::DeclareClassOffset("Sample", "tags", _offsetof(Sample, tags));
	Kaba::LinkExternal("Sample.create_ref", Kaba::mf(&Sample::create_ref));
	Kaba::LinkExternal("Sample.get_value", Kaba::mf(&Sample::getValue));

	Sample sample(SignalType::AUDIO);
	sample._pointer_ref(); // stack allocated... don't auto-delete!
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
	Kaba::LinkExternal("MidiSource." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&MidiSource::__init__));
	Kaba::DeclareClassVirtualIndex("MidiSource", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&MidiSource::__delete__), &msource);
	Kaba::DeclareClassVirtualIndex("MidiSource", "read", Kaba::mf(&MidiSource::read), &msource);
	Kaba::DeclareClassVirtualIndex("MidiSource", "reset", Kaba::mf(&MidiSource::reset), &msource);
	Kaba::LinkExternal("MidiSource.set_beat_source", Kaba::mf(&MidiSource::set_beat_source));


	BeatMidifier bmidifier;
	Kaba::DeclareClassSize("BeatMidifier", sizeof(BeatMidifier));
	Kaba::LinkExternal("BeatMidifier." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&BeatMidifier::__init__));
	//Kaba::DeclareClassVirtualIndex("BeatMidifier", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&MidiSource::__delete__), &bmidifier);
	Kaba::DeclareClassVirtualIndex("BeatMidifier", "read", Kaba::mf(&BeatMidifier::read), &bmidifier);
	Kaba::DeclareClassVirtualIndex("BeatMidifier", "reset", Kaba::mf(&BeatMidifier::reset), &bmidifier);
	Kaba::DeclareClassOffset("BeatMidifier", "volume", _offsetof(BeatMidifier, volume));

	Synthesizer synth;
	Kaba::DeclareClassSize("Synthesizer", sizeof(Synthesizer));
	Kaba::DeclareClassOffset("Synthesizer", "sample_rate", _offsetof(Synthesizer, sample_rate));
	Kaba::DeclareClassOffset("Synthesizer", "events", _offsetof(Synthesizer, events));
	Kaba::DeclareClassOffset("Synthesizer", "keep_notes", _offsetof(Synthesizer, keep_notes));
	Kaba::DeclareClassOffset("Synthesizer", "active_pitch", _offsetof(Synthesizer, active_pitch));
	Kaba::DeclareClassOffset("Synthesizer", "freq", _offsetof(Synthesizer, tuning.freq));
	Kaba::DeclareClassOffset("Synthesizer", "delta_phi", _offsetof(Synthesizer, delta_phi));
	Kaba::DeclareClassOffset("Synthesizer", "out", _offsetof(Synthesizer, out));
	Kaba::LinkExternal("Synthesizer." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&Synthesizer::__init__));
	Kaba::DeclareClassVirtualIndex("Synthesizer", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&Synthesizer::__delete__), &synth);
	Kaba::LinkExternal("Synthesizer.enable_pitch", Kaba::mf(&Synthesizer::enablePitch));
	Kaba::DeclareClassVirtualIndex("Synthesizer", "render", Kaba::mf(&Synthesizer::render), &synth);
	Kaba::LinkExternal("Synthesizer.set_sample_rate", Kaba::mf(&Synthesizer::setSampleRate));
	Kaba::LinkExternal("Synthesizer.set_source", Kaba::mf(&Synthesizer::set_source));


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
	Kaba::LinkExternal("MidiEventBuffer.get_events", Kaba::mf(&MidiEventBuffer::getEvents));
	Kaba::LinkExternal("MidiEventBuffer.get_notes", Kaba::mf(&MidiEventBuffer::getNotes));
	Kaba::LinkExternal("MidiEventBuffer.get_range", Kaba::mf(&MidiEventBuffer::getRange));
	Kaba::LinkExternal("MidiEventBuffer.add_metronome_click", Kaba::mf(&MidiEventBuffer::addMetronomeClick));

	Kaba::DeclareClassSize("MidiNoteBuffer", sizeof(MidiNoteBuffer));
	Kaba::DeclareClassOffset("MidiNoteBuffer", "samples", _offsetof(MidiNoteBuffer, samples));
	Kaba::LinkExternal("MidiNoteBuffer." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&MidiNoteBuffer::__init__));
	Kaba::LinkExternal("MidiNoteBuffer.get_events", Kaba::mf(&MidiNoteBuffer::getEvents));
	Kaba::LinkExternal("MidiNoteBuffer.get_notes", Kaba::mf(&MidiNoteBuffer::getNotes));
	Kaba::LinkExternal("MidiNoteBuffer.get_range", Kaba::mf(&MidiNoteBuffer::range));

	BeatPort bport;
	Kaba::DeclareClassSize("BeatPort", sizeof(BeatPort));
	Kaba::LinkExternal("BeatPort." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&BeatPort::__init__));
	Kaba::DeclareClassVirtualIndex("BeatPort", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&BeatPort::__delete__), &bport);
	Kaba::DeclareClassVirtualIndex("BeatPort", "read", Kaba::mf(&BeatPort::read), &bport);
	Kaba::DeclareClassVirtualIndex("BeatPort", "reset", Kaba::mf(&BeatPort::reset), &bport);

	BeatSource bsource;
	Kaba::DeclareClassSize("BeatSource", sizeof(BeatSource));
	Kaba::DeclareClassOffset("BeatSource", "out", _offsetof(BeatSource, out));
	Kaba::LinkExternal("BeatSource." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&BeatSource::__init__));
	Kaba::DeclareClassVirtualIndex("BeatSource", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&BeatSource::__delete__), &bsource);
	Kaba::DeclareClassVirtualIndex("BeatSource", "read", Kaba::mf(&BeatSource::read), &bsource);
	Kaba::DeclareClassVirtualIndex("BeatSource", "reset", Kaba::mf(&BeatSource::reset), &bsource);
	Kaba::DeclareClassVirtualIndex("BeatSource", "beats_per_bar", Kaba::mf(&BeatSource::beats_per_bar), &bsource);
	Kaba::DeclareClassVirtualIndex("BeatSource", "cur_beat", Kaba::mf(&BeatSource::cur_beat), &bsource);
	Kaba::DeclareClassVirtualIndex("BeatSource", "beat_fraction", Kaba::mf(&BeatSource::beat_fraction), &bsource);

	Kaba::DeclareClassSize("TrackMarker", sizeof(TrackMarker));
	Kaba::DeclareClassOffset("TrackMarker", "text", _offsetof(TrackMarker, text));
	Kaba::DeclareClassOffset("TrackMarker", "range", _offsetof(TrackMarker, range));
	Kaba::DeclareClassOffset("TrackMarker", "fx", _offsetof(TrackMarker, fx));

	Kaba::DeclareClassSize("TrackLayer", sizeof(TrackLayer));
	Kaba::DeclareClassOffset("TrackLayer", "buffers", _offsetof(TrackLayer, buffers));
	Kaba::DeclareClassOffset("TrackLayer", "midi", _offsetof(TrackLayer, midi));
	Kaba::DeclareClassOffset("TrackLayer", "samples", _offsetof(TrackLayer, samples));
	Kaba::DeclareClassOffset("TrackLayer", "track", _offsetof(TrackLayer, track));
	Kaba::DeclareClassOffset("TrackLayer", "muted", _offsetof(TrackLayer, muted));
	Kaba::LinkExternal("TrackLayer.get_buffers", Kaba::mf(&TrackLayer::getBuffers));
	Kaba::LinkExternal("TrackLayer.read_buffers", Kaba::mf(&TrackLayer::readBuffers));
	Kaba::LinkExternal("TrackLayer.set_muted", Kaba::mf(&TrackLayer::setMuted));
	Kaba::LinkExternal("TrackLayer.insert_midi_data", Kaba::mf(&TrackLayer::insertMidiData));
	Kaba::LinkExternal("TrackLayer.add_midi_note", Kaba::mf(&TrackLayer::addMidiNote));
	//Kaba::LinkExternal("TrackLayer.add_midi_notes", Kaba::mf(&TrackLayer::addMidiNotes));
	Kaba::LinkExternal("TrackLayer.delete_midi_note", Kaba::mf(&TrackLayer::deleteMidiNote));
	Kaba::LinkExternal("TrackLayer.add_sample_ref", Kaba::mf(&TrackLayer::addSampleRef));
	Kaba::LinkExternal("TrackLayer.delete_sample_ref", Kaba::mf(&TrackLayer::deleteSampleRef));
	Kaba::LinkExternal("TrackLayer.edit_sample_ref", Kaba::mf(&TrackLayer::editSampleRef));

	Kaba::DeclareClassSize("Track", sizeof(Track));
	Kaba::DeclareClassOffset("Track", "type", _offsetof(Track, type));
	Kaba::DeclareClassOffset("Track", "name", _offsetof(Track, name));
	Kaba::DeclareClassOffset("Track", "layers", _offsetof(Track, layers));
	Kaba::DeclareClassOffset("Track", "volume", _offsetof(Track, volume));
	Kaba::DeclareClassOffset("Track", "panning", _offsetof(Track, panning));
	Kaba::DeclareClassOffset("Track", "muted", _offsetof(Track, muted));
	Kaba::DeclareClassOffset("Track", "fx", _offsetof(Track, fx));
	Kaba::DeclareClassOffset("Track", "synth", _offsetof(Track, synth));
	Kaba::DeclareClassOffset("Track", "markers", _offsetof(Track, markers));
	Kaba::DeclareClassOffset("Track", "root", _offsetof(Track, song));
	Kaba::LinkExternal("Track.set_name", Kaba::mf(&Track::setName));
	Kaba::LinkExternal("Track.set_muted", Kaba::mf(&Track::setMuted));
	Kaba::LinkExternal("Track.set_volume", Kaba::mf(&Track::setVolume));
	Kaba::LinkExternal("Track.set_panning", Kaba::mf(&Track::setPanning));
	Kaba::LinkExternal("Track.add_effect", Kaba::mf(&Track::addEffect));
	Kaba::LinkExternal("Track.delete_effect", Kaba::mf(&Track::deleteEffect));
	Kaba::LinkExternal("Track.edit_effect", Kaba::mf(&Track::editEffect));
	Kaba::LinkExternal("Track.enable_effect", Kaba::mf(&Track::enableEffect));
	Kaba::LinkExternal("Track.set_synthesizer", Kaba::mf(&Track::setSynthesizer));
	Kaba::LinkExternal("Track.add_marker", Kaba::mf(&Track::addMarker));
	Kaba::LinkExternal("Track.delete_marker", Kaba::mf(&Track::deleteMarker));
	Kaba::LinkExternal("Track.edit_marker", Kaba::mf(&Track::editMarker));

	Song af(Session::GLOBAL, DEFAULT_SAMPLE_RATE);
	Kaba::DeclareClassSize("Song", sizeof(Song));
	Kaba::DeclareClassOffset("Song", "filename", _offsetof(Song, filename));
	Kaba::DeclareClassOffset("Song", "tag", _offsetof(Song, tags));
	Kaba::DeclareClassOffset("Song", "sample_rate", _offsetof(Song, sample_rate));
	Kaba::DeclareClassOffset("Song", "volume", _offsetof(Song, volume));
	Kaba::DeclareClassOffset("Song", "fx", _offsetof(Song, fx));
	Kaba::DeclareClassOffset("Song", "tracks", _offsetof(Song, tracks));
	Kaba::DeclareClassOffset("Song", "samples", _offsetof(Song, samples));
//	Kaba::DeclareClassOffset("Song", "layers", _offsetof(Song, layers));
	Kaba::DeclareClassOffset("Song", "bars", _offsetof(Song, bars));
	Kaba::LinkExternal("Song." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&Song::__init__));
	Kaba::DeclareClassVirtualIndex("Song", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&Song::__delete__), &af);
	Kaba::LinkExternal("Song.add_track", Kaba::mf(&Song::addTrack));
	Kaba::LinkExternal("Song.delete_track", Kaba::mf(&Song::deleteTrack));
	Kaba::LinkExternal("Song.range", Kaba::mf(&Song::range));
	Kaba::LinkExternal("Song.add_bar", Kaba::mf(&Song::addBar));
	Kaba::LinkExternal("Song.add_pause", Kaba::mf(&Song::addPause));
	Kaba::LinkExternal("Song.edit_bar", Kaba::mf(&Song::editBar));
	Kaba::LinkExternal("Song.delete_bar", Kaba::mf(&Song::deleteBar));
	Kaba::LinkExternal("Song.add_sample", Kaba::mf(&Song::addSample));
	Kaba::LinkExternal("Song.delete_sample", Kaba::mf(&Song::deleteSample));

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
	Kaba::LinkExternal("SongRenderer.get_beat_source", Kaba::mf(&SongRenderer::get_beat_source));

	{
	InputStreamAudio input(Session::GLOBAL);
	Kaba::DeclareClassSize("InputStreamAudio", sizeof(InputStreamAudio));
	Kaba::DeclareClassOffset("InputStreamAudio", "current_buffer", _offsetof(InputStreamAudio, buffer));
	Kaba::DeclareClassOffset("InputStreamAudio", "out", _offsetof(InputStreamAudio, out));
	Kaba::DeclareClassOffset("InputStreamAudio", "capturing", _offsetof(InputStreamAudio, capturing));
	Kaba::LinkExternal("InputStreamAudio." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&InputStreamAudio::__init__));
	Kaba::DeclareClassVirtualIndex("InputStreamAudio", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&InputStreamAudio::__delete__), &input);
	Kaba::LinkExternal("InputStreamAudio.start", Kaba::mf(&InputStreamAudio::start));
	Kaba::LinkExternal("InputStreamAudio.stop",	 Kaba::mf(&InputStreamAudio::stop));
	Kaba::LinkExternal("InputStreamAudio.is_capturing", Kaba::mf(&InputStreamAudio::is_capturing));
	Kaba::LinkExternal("InputStreamAudio.sample_rate", Kaba::mf(&InputStreamAudio::sample_rate));
	Kaba::LinkExternal("InputStreamAudio.set_backup_mode", Kaba::mf(&InputStreamAudio::set_backup_mode));
	}

	{
	OutputStream stream(Session::GLOBAL, nullptr);
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
	Kaba::LinkExternal("OutputStream.set_update_dt", Kaba::mf(&OutputStream::set_update_dt));
//	Kaba::DeclareClassVirtualIndex("OutputStream", "", Kaba::mf(&OutputStream::__delete__), &stream);
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
	Kaba::LinkExternal("Storage.save_via_renderer", Kaba::mf(&Storage::saveViaRenderer));
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
	Kaba::DeclareClassVirtualIndex("TsunamiPlugin", "on_start", Kaba::mf(&TsunamiPlugin::onStart), &tsunami_plugin);
	Kaba::DeclareClassVirtualIndex("TsunamiPlugin", "on_stop", Kaba::mf(&TsunamiPlugin::onStop), &tsunami_plugin);
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

void find_plugins_in_dir(const string &dir, ModuleType type, PluginManager *pm)
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
            m->add_with_image(f.name, f.image, id);
            win->event(id, std::bind(function, win));
		}
	}
}

void PluginManager::FindPlugins()
{
	Kaba::Init();

	// "AudioSource"
	find_plugins_in_dir("AudioSource/", ModuleType::AUDIO_SOURCE, this);

	// "AudioEffect"
	find_plugins_in_dir("AudioEffect/Channels/", ModuleType::AUDIO_EFFECT, this);
	find_plugins_in_dir("AudioEffect/Dynamics/", ModuleType::AUDIO_EFFECT, this);
	find_plugins_in_dir("AudioEffect/Echo/", ModuleType::AUDIO_EFFECT, this);
	find_plugins_in_dir("AudioEffect/Pitch/", ModuleType::AUDIO_EFFECT, this);
	find_plugins_in_dir("AudioEffect/Repair/", ModuleType::AUDIO_EFFECT, this);
	find_plugins_in_dir("AudioEffect/Sound/", ModuleType::AUDIO_EFFECT, this);
	find_plugins_in_dir("AudioEffect/Synthesizer/", ModuleType::AUDIO_EFFECT, this);

	// "AudioVisualizer"
	find_plugins_in_dir("AudioVisualizer/", ModuleType::AUDIO_VISUALIZER, this);

	// "MidiSource"
	find_plugins_in_dir("MidiSource/", ModuleType::MIDI_SOURCE, this);

	// "MidiEffect"
	find_plugins_in_dir("MidiEffect/", ModuleType::MIDI_EFFECT, this);

	// "BeatSource"
	find_plugins_in_dir("BeatSource/", ModuleType::BEAT_SOURCE, this);

	// "All"
	find_plugins_in_dir("All/", ModuleType::SONG_PLUGIN, this);

	// rest
	find_plugins_in_dir("Independent/", ModuleType::TSUNAMI_PLUGIN, this);

	// "Synthesizer"
	find_plugins_in_dir("Synthesizer/", ModuleType::SYNTHESIZER, this);
}

void PluginManager::AddPluginsToMenu(TsunamiWindow *win)
{
	hui::Menu *m = win->getMenu();

	// "Buffer"
	add_plugins_in_dir("AudioEffect/Channels/", this, m->get_sub_menu_by_id("menu_plugins_channels"), "audio-effect", win, &TsunamiWindow::onMenuExecuteAudioEffect);
	add_plugins_in_dir("AudioEffect/Dynamics/", this, m->get_sub_menu_by_id("menu_plugins_dynamics"), "audio-effect", win, &TsunamiWindow::onMenuExecuteAudioEffect);
	add_plugins_in_dir("AudioEffect/Echo/", this, m->get_sub_menu_by_id("menu_plugins_echo"), "audio-effect", win, &TsunamiWindow::onMenuExecuteAudioEffect);
	add_plugins_in_dir("AudioEffect/Pitch/", this, m->get_sub_menu_by_id("menu_plugins_pitch"), "audio-effect", win, &TsunamiWindow::onMenuExecuteAudioEffect);
	add_plugins_in_dir("AudioEffect/Repair/", this, m->get_sub_menu_by_id("menu_plugins_repair"), "audio-effect", win, &TsunamiWindow::onMenuExecuteAudioEffect);
	add_plugins_in_dir("AudioEffect/Sound/", this, m->get_sub_menu_by_id("menu_plugins_sound"), "audio-effect", win, &TsunamiWindow::onMenuExecuteAudioEffect);
	add_plugins_in_dir("AudioEffect/Synthesizer/", this, m->get_sub_menu_by_id("menu_plugins_synthesizer"), "audio-effect", win, &TsunamiWindow::onMenuExecuteAudioEffect);

	add_plugins_in_dir("AudioSource/", this, m->get_sub_menu_by_id("menu_plugins_audio_source"), "source", win, &TsunamiWindow::onMenuExecuteAudioSource);

	// "Midi"
	add_plugins_in_dir("MidiEffect/", this, m->get_sub_menu_by_id("menu_plugins_midi_effects"), "midi-effect", win, &TsunamiWindow::onMenuExecuteMidiEffect);
	add_plugins_in_dir("MidiSource/", this, m->get_sub_menu_by_id("menu_plugins_midi_source"), "midi-source", win, &TsunamiWindow::onMenuExecuteMidiSource);

	// "All"
	add_plugins_in_dir("All/", this, m->get_sub_menu_by_id("menu_plugins_on_all"), "song", win, &TsunamiWindow::onMenuExecuteSongPlugin);

	// rest
	add_plugins_in_dir("Independent/", this, m->get_sub_menu_by_id("menu_plugins_other"), "tsunami", win, &TsunamiWindow::onMenuExecuteTsunamiPlugin);
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
Plugin *PluginManager::LoadAndCompilePlugin(ModuleType type, const string &filename)
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


Plugin *PluginManager::GetPlugin(Session *session, ModuleType type, const string &name)
{
	for (PluginFile &pf: plugin_files){
		if ((pf.name == name) and (pf.type == type)){
			Plugin *p = LoadAndCompilePlugin(type, pf.filename);
			if (!p->usable)
				session->e(p->get_error());
			return p;
		}
	}
	session->e(format(_("Can't find %s plugin: %s ..."), Module::type_to_name(type).c_str(), name.c_str()));
	return nullptr;
}

string PluginManager::plugin_dir()
{
	if (tsunami->installed)
		return tsunami->directory_static + "Plugins/";
	return "Plugins/";
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


Array<string> PluginManager::FindModuleSubTypes(ModuleType type)
{
	if (type == ModuleType::AUDIO_EFFECT)
		return FindAudioEffects();

	Array<string> names;
	for (auto &pf: plugin_files)
		if (pf.type == type)
			names.add(pf.name);

	if (type == ModuleType::AUDIO_SOURCE){
		names.add("SongRenderer");
		//names.add("BufferStreamer");
	}
	if (type == ModuleType::MIDI_EFFECT)
		names.add("Dummy");
	if (type == ModuleType::BEAT_SOURCE){
		//names.add("BarStreamer");
	}
	if (type == ModuleType::AUDIO_VISUALIZER)
		names.add("PeakMeter");
	if (type == ModuleType::SYNTHESIZER){
		names.add("Dummy");
		//names.add("Sample");
	}
	return names;
}


string PluginManager::ChooseModule(hui::Panel *parent, Session *session, ModuleType type, const string &old_name)
{
	ConfigurableSelectorDialog *dlg = new ConfigurableSelectorDialog(parent->win, type, session, old_name);
	dlg->run();
	string name = dlg->_return;
	delete(dlg);
	return name;
}


