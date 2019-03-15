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
#include "../Data/TrackLayer.h"
#include "../Data/TrackMarker.h"
#include "../Data/Sample.h"
#include "../Data/SampleRef.h"
#include "../Data/Audio/AudioBuffer.h"
#include "../Data/Audio/BufferInterpolator.h"
#include "../Data/Rhythm/Bar.h"
#include "../Module/ConfigPanel.h"
#include "../Module/SignalChain.h"
#include "../Module/ModuleConfiguration.h"
#include "../Module/Synth/Synthesizer.h"
#include "../Module/Synth/DummySynthesizer.h"
#include "../Module/Port/Port.h"
#include "../Module/Audio/AudioSource.h"
#include "../Module/Audio/SongRenderer.h"
#include "../Module/Audio/AudioVisualizer.h"
#include "../Module/Midi/MidiSource.h"
#include "../Module/Audio/AudioEffect.h"
#include "../Module/Beats/BeatSource.h"
#include "../Module/Beats/BeatMidifier.h"
#include "../Module/Midi/MidiEffect.h"
#include "../View/Helper/Progress.h"
#include "../Storage/Storage.h"
#include "../Device/DeviceManager.h"
#include "../Stream/AudioInput.h"
#include "../Stream/AudioOutput.h"
#include "../Stream/MidiInput.h"
#include "../Stuff/Clipboard.h"
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

PluginManager::PluginManager()
{
	favorites = new FavoriteManager;

	find_plugins();
}

PluginManager::~PluginManager()
{
	delete(favorites);
	Kaba::End();
}



void GlobalSetTempBackupFilename(const string &filename)
{
	//InputStreamAudio::setTempBackupFilename(filename);
}

void __play__(Session *s)
{
	s->win->on_play();
	//s->signal_chain->start();
}
void __stop__(Session *s)
{
	s->win->on_stop();
	//s->signal_chain->stop();
}

Module *_CreateBeatMidifier(Session *s)
{
	return new BeatMidifier();
}


void PluginManager::link_app_script_data()
{
	Kaba::config.directory = "";

	// api definition
	Kaba::LinkExternal("device_manager", &tsunami->device_manager);
	Kaba::LinkExternal("colors", &AudioView::colors);
	Kaba::LinkExternal("clipboard", &tsunami->clipboard);
	//Kaba::LinkExternal("view_input", &export_view_input);
	Kaba::LinkExternal("db2amp", (void*)&db2amplitude);
	Kaba::LinkExternal("amp2db", (void*)&amplitude2db);
	Kaba::LinkExternal("fft_c2c", (void*)&FastFourierTransform::fft_c2c);
	Kaba::LinkExternal("fft_r2c", (void*)&FastFourierTransform::fft_r2c);
	Kaba::LinkExternal("fft_c2r_inv", (void*)&FastFourierTransform::fft_c2r_inv);
//	Kaba::LinkExternal("CreateModule", (void*)&ModuleFactory::create);
	Kaba::LinkExternal("CreateSynthesizer", (void*)&CreateSynthesizer);
	Kaba::LinkExternal("CreateAudioEffect", (void*)&CreateAudioEffect);
	Kaba::LinkExternal("CreateAudioSource", (void*)&CreateAudioSource);
	Kaba::LinkExternal("CreateMidiEffect", (void*)&CreateMidiEffect);
	Kaba::LinkExternal("CreateMidiSource", (void*)&CreateMidiSource);
	Kaba::LinkExternal("CreateBeatSource", (void*)&CreateBeatSource);
	Kaba::LinkExternal("CreateBeatMidifier", (void*)&_CreateBeatMidifier);
	Kaba::LinkExternal("SetTempBackupFilename", (void*)&GlobalSetTempBackupFilename);
	Kaba::LinkExternal("SelectSample", (void*)&SampleManagerConsole::select);
	Kaba::LinkExternal("draw_boxed_str", (void*)&AudioView::draw_boxed_str);
	Kaba::LinkExternal("interpolate_buffer", (void*)&BufferInterpolator::interpolate);


	Kaba::DeclareClassSize("Clipboard", sizeof(Clipboard));
	Kaba::DeclareClassOffset("Clipboard", "temp", _offsetof(Clipboard, temp));
	Kaba::LinkExternal("Clipboard.has_data", Kaba::mf(&Clipboard::has_data));
	Kaba::LinkExternal("Clipboard.prepare_layer_map", Kaba::mf(&Clipboard::prepare_layer_map));

	Kaba::DeclareClassSize("Range", sizeof(Range));
	Kaba::DeclareClassOffset("Range", "offset", _offsetof(Range, offset));
	Kaba::DeclareClassOffset("Range", "length", _offsetof(Range, length));

	Kaba::DeclareClassSize("Bar", sizeof(Bar));
	Kaba::DeclareClassOffset("Bar", "beats", _offsetof(Bar, beats));
	Kaba::DeclareClassOffset("Bar", "divisor", _offsetof(Bar, divisor));
	Kaba::DeclareClassOffset("Bar", "length", _offsetof(Bar, length));
	Kaba::DeclareClassOffset("Bar", "index", _offsetof(Bar, index));
	Kaba::DeclareClassOffset("Bar", "index_text", _offsetof(Bar, index_text));
	Kaba::DeclareClassOffset("Bar", "offset", _offsetof(Bar, offset));
	Kaba::LinkExternal("Bar.range", Kaba::mf(&Bar::range));
	Kaba::LinkExternal("Bar.bpm", Kaba::mf(&Bar::bpm));

	Kaba::DeclareClassSize("Session", sizeof(Session));
	Kaba::DeclareClassOffset("Session", "id", _offsetof(Session, id));
	Kaba::DeclareClassOffset("Session", "storage", _offsetof(Session, storage));
	Kaba::DeclareClassOffset("Session", "win", _offsetof(Session, _kaba_win));
	Kaba::DeclareClassOffset("Session", "view", _offsetof(Session, view));
	Kaba::DeclareClassOffset("Session", "song", _offsetof(Session, song));
	Kaba::DeclareClassOffset("Session", "signal_chain", _offsetof(Session, signal_chain));
	Kaba::DeclareClassOffset("Session", "song_renderer", _offsetof(Session, song_renderer));
	Kaba::DeclareClassOffset("Session", "output_stream", _offsetof(Session, output_stream));
	Kaba::LinkExternal("Session.sample_rate", Kaba::mf(&Session::sample_rate));
	Kaba::LinkExternal("Session.i", Kaba::mf(&Session::i));
	Kaba::LinkExternal("Session.w", Kaba::mf(&Session::w));
	Kaba::LinkExternal("Session.e", Kaba::mf(&Session::e));
	Kaba::LinkExternal("Session.play", (void*)&__play__);
	Kaba::LinkExternal("Session.stop", (void*)&__stop__);
	Kaba::LinkExternal("Session.create_child", Kaba::mf(&Session::create_child));


	Module module(ModuleType::AUDIO_EFFECT, "");
	Kaba::DeclareClassSize("Module", sizeof(Module));
	Kaba::DeclareClassOffset("Module", "name", _offsetof(Module, module_subtype));
	Kaba::DeclareClassOffset("Module", "usable", _offsetof(Module, usable));
	Kaba::DeclareClassOffset("Module", "session", _offsetof(Module, session));
	Kaba::LinkExternal("Module.__init__", Kaba::mf(&Module::__init__));
	Kaba::DeclareClassVirtualIndex("Module", "__delete__", Kaba::mf(&Module::__delete__), &module);
	Kaba::DeclareClassVirtualIndex("Module", "create_panel", Kaba::mf(&Module::create_panel), &module);
	Kaba::LinkExternal("Module.reset_config", Kaba::mf(&Module::reset_config));
	Kaba::DeclareClassVirtualIndex("Module", "reset_state", Kaba::mf(&Module::reset_state), &module);
	Kaba::LinkExternal("Module.changed", Kaba::mf(&Module::changed));
	Kaba::DeclareClassVirtualIndex("Module", "get_config", Kaba::mf(&Module::get_config), &module);
	Kaba::DeclareClassVirtualIndex("Module", "config_to_string", Kaba::mf(&Module::config_to_string), &module);
	Kaba::DeclareClassVirtualIndex("Module", "config_from_string", Kaba::mf(&Module::config_from_string), &module);
	Kaba::DeclareClassVirtualIndex("Module", "on_config", Kaba::mf(&Module::on_config), &module);
	Kaba::DeclareClassVirtualIndex("Module", "command", Kaba::mf(&Module::command), &module);
	Kaba::DeclareClassVirtualIndex("Module", "get_pos", Kaba::mf(&Module::get_pos), &module);
	Kaba::DeclareClassVirtualIndex("Module", "set_pos", Kaba::mf(&Module::set_pos), &module);
	Kaba::LinkExternal("Module.plug", Kaba::mf(&Module::plug));
	Kaba::LinkExternal("Module.unplug", Kaba::mf(&Module::unplug));
	Kaba::LinkExternal("Module.subscribe", Kaba::mf(&Module::subscribe_kaba));
	Kaba::LinkExternal("Module.unsubscribe", Kaba::mf(&Module::unsubscribe));
	Kaba::LinkExternal("Module.copy", Kaba::mf(&Module::copy));


	ModuleConfiguration plugin_data;
	Kaba::DeclareClassSize("PluginData", sizeof(ModuleConfiguration));
	Kaba::LinkExternal("PluginData.__init__", Kaba::mf(&ModuleConfiguration::__init__));
	Kaba::DeclareClassVirtualIndex("PluginData", "__delete__", Kaba::mf(&ModuleConfiguration::__delete__), &plugin_data);
	Kaba::DeclareClassVirtualIndex("PluginData", "reset", Kaba::mf(&ModuleConfiguration::reset), &plugin_data);


	ConfigPanel config_panel;
	Kaba::DeclareClassSize("ConfigPanel", sizeof(ConfigPanel));
	Kaba::LinkExternal("ConfigPanel.__init__", Kaba::mf(&ConfigPanel::__init__));
	Kaba::DeclareClassVirtualIndex("ConfigPanel", "__delete__", Kaba::mf(&ConfigPanel::__delete__), &config_panel);
	Kaba::DeclareClassVirtualIndex("ConfigPanel", "update", Kaba::mf(&ConfigPanel::update), &config_panel);
	Kaba::DeclareClassVirtualIndex("ConfigPanel", "set_large", Kaba::mf(&ConfigPanel::set_large), &config_panel);
	Kaba::LinkExternal("ConfigPanel.changed", Kaba::mf(&ConfigPanel::changed));
	Kaba::DeclareClassOffset("ConfigPanel", "c", _offsetof(ConfigPanel, c));

	AudioSource asource;
	Kaba::DeclareClassSize("AudioSource", sizeof(AudioSource));
	Kaba::LinkExternal("AudioSource.__init__", Kaba::mf(&AudioSource::__init__));
	Kaba::DeclareClassVirtualIndex("AudioSource", "__delete__", Kaba::mf(&AudioSource::__delete__), &asource);
	Kaba::DeclareClassVirtualIndex("AudioSource", "read", Kaba::mf(&AudioSource::read), &asource);
	Kaba::DeclareClassVirtualIndex("AudioSource", "get_pos", Kaba::mf(&AudioSource::get_pos), &asource);
	Kaba::DeclareClassVirtualIndex("AudioSource", "set_pos", Kaba::mf(&AudioSource::set_pos), &asource);

	AudioEffect aeffect;
	Kaba::DeclareClassSize("AudioEffect", sizeof(AudioEffect));
	Kaba::DeclareClassOffset("AudioEffect", "sample_rate", _offsetof(AudioEffect, sample_rate));
	Kaba::DeclareClassOffset("AudioEffect", "out", _offsetof(AudioEffect, out));
	Kaba::DeclareClassOffset("AudioEffect", "source", _offsetof(AudioEffect, source));
	Kaba::LinkExternal("AudioEffect.__init__", Kaba::mf(&AudioEffect::__init__));
	Kaba::DeclareClassVirtualIndex("AudioEffect", "__delete__", Kaba::mf(&AudioEffect::__delete__), &aeffect);
	Kaba::DeclareClassVirtualIndex("AudioEffect", "process", Kaba::mf(&AudioEffect::process), &aeffect);
	Kaba::DeclareClassVirtualIndex("AudioEffect", "read", Kaba::mf(&AudioEffect::read), &aeffect);

	MidiEffect meffect;
	Kaba::DeclareClassSize("MidiEffect", sizeof(MidiEffect));
	Kaba::DeclareClassOffset("MidiEffect", "only_on_selection", _offsetof(MidiEffect, only_on_selection));
	Kaba::DeclareClassOffset("MidiEffect", "range", _offsetof(MidiEffect, range));
	Kaba::LinkExternal("MidiEffect.__init__", Kaba::mf(&MidiEffect::__init__));
	Kaba::DeclareClassVirtualIndex("MidiEffect", "__delete__", Kaba::mf(&MidiEffect::__delete__), &meffect);
	Kaba::DeclareClassVirtualIndex("MidiEffect", "process", Kaba::mf(&MidiEffect::process), &meffect);

	AudioVisualizer avis;
	Kaba::DeclareClassSize("AudioVisualizer", sizeof(AudioVisualizer));
	Kaba::DeclareClassOffset("AudioVisualizer", "chunk_size", _offsetof(AudioVisualizer, chunk_size));
	Kaba::LinkExternal("AudioVisualizer.__init__", Kaba::mf(&AudioVisualizer::__init__));
	Kaba::DeclareClassVirtualIndex("AudioVisualizer", "__delete__", Kaba::mf(&AudioVisualizer::__delete__), &avis);
	Kaba::DeclareClassVirtualIndex("AudioVisualizer", "process", Kaba::mf(&AudioVisualizer::process), &avis);
	Kaba::LinkExternal("AudioVisualizer.set_chunk_size", Kaba::mf(&AudioVisualizer::set_chunk_size));

	Kaba::DeclareClassSize("AudioBuffer", sizeof(AudioBuffer));
	Kaba::DeclareClassOffset("AudioBuffer", "offset", _offsetof(AudioBuffer, offset));
	Kaba::DeclareClassOffset("AudioBuffer", "length", _offsetof(AudioBuffer, length));
	Kaba::DeclareClassOffset("AudioBuffer", "channels", _offsetof(AudioBuffer, channels));
	Kaba::DeclareClassOffset("AudioBuffer", "c", _offsetof(AudioBuffer, c));
	Kaba::DeclareClassOffset("AudioBuffer", "r", _offsetof(AudioBuffer, c[0]));
	Kaba::DeclareClassOffset("AudioBuffer", "l", _offsetof(AudioBuffer, c[1]));
	Kaba::DeclareClassOffset("AudioBuffer", "peaks", _offsetof(AudioBuffer, peaks));
	Kaba::LinkExternal("AudioBuffer.__init__", Kaba::mf(&AudioBuffer::__init__));
	Kaba::LinkExternal("AudioBuffer.__delete__", Kaba::mf(&AudioBuffer::__delete__));
	Kaba::LinkExternal("AudioBuffer.clear", Kaba::mf(&AudioBuffer::clear));
	Kaba::LinkExternal("AudioBuffer.__assign__", Kaba::mf(&AudioBuffer::__assign__));
	Kaba::LinkExternal("AudioBuffer.range", Kaba::mf(&AudioBuffer::range));
	Kaba::LinkExternal("AudioBuffer.resize", Kaba::mf(&AudioBuffer::resize));
	Kaba::LinkExternal("AudioBuffer.add", Kaba::mf(&AudioBuffer::add));
	Kaba::LinkExternal("AudioBuffer.set", Kaba::mf(&AudioBuffer::set));
	Kaba::LinkExternal("AudioBuffer.set_as_ref", Kaba::mf(&AudioBuffer::set_as_ref));
	Kaba::LinkExternal("AudioBuffer." + Kaba::IDENTIFIER_FUNC_SUBARRAY, Kaba::mf(&AudioBuffer::ref));
	Kaba::LinkExternal("AudioBuffer.get_spectrum", Kaba::mf(&ExtendedAudioBuffer::get_spectrum));


	Kaba::DeclareClassSize("RingBuffer", sizeof(RingBuffer));
	Kaba::LinkExternal("RingBuffer.__init__", Kaba::mf(&RingBuffer::__init__));
	Kaba::LinkExternal("RingBuffer.__delete__", Kaba::mf(&RingBuffer::__delete__));
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
	Kaba::LinkExternal("Sample.get_value", Kaba::mf(&Sample::get_value));
	Kaba::LinkExternal("Sample.set_value", Kaba::mf(&Sample::set_value));

	Sample sample(SignalType::AUDIO);
	sample._pointer_ref(); // stack allocated... don't auto-delete!
	//sample.owner = tsunami->song;
	SampleRef sampleref(&sample);
	Kaba::DeclareClassSize("SampleRef", sizeof(SampleRef));
	Kaba::DeclareClassOffset("SampleRef", "buf", _offsetof(SampleRef, buf));
	Kaba::DeclareClassOffset("SampleRef", "midi", _offsetof(SampleRef, midi));
	Kaba::DeclareClassOffset("SampleRef", "origin", _offsetof(SampleRef, origin));
	Kaba::LinkExternal("SampleRef.__init__", Kaba::mf(&SampleRef::__init__));
	Kaba::DeclareClassVirtualIndex("SampleRef", "__delete__", Kaba::mf(&SampleRef::__delete__), &sampleref);



	Port port(SignalType::AUDIO, "");
	Kaba::DeclareClassSize("Port", sizeof(Port));
	Kaba::LinkExternal("Port.__init__", Kaba::mf(&Port::__init__));
	Kaba::DeclareClassVirtualIndex("Port", "__delete__", Kaba::mf(&Port::__delete__), &port);
	Kaba::DeclareClassVirtualIndex("Port", "read_audio", Kaba::mf(&Port::read_audio), &port);
	Kaba::DeclareClassVirtualIndex("Port", "read_midi", Kaba::mf(&Port::read_midi), &port);
	Kaba::DeclareClassVirtualIndex("Port", "read_beats", Kaba::mf(&Port::read_beats), &port);

	MidiSource msource;
	Kaba::DeclareClassSize("MidiSource", sizeof(MidiSource));
	Kaba::DeclareClassOffset("MidiSource", "bh_midi", _offsetof(MidiSource, bh_midi));
	Kaba::LinkExternal("MidiSource.__init__", Kaba::mf(&MidiSource::__init__));
	Kaba::DeclareClassVirtualIndex("MidiSource", "__delete__", Kaba::mf(&MidiSource::__delete__), &msource);
	Kaba::DeclareClassVirtualIndex("MidiSource", "read", Kaba::mf(&MidiSource::read), &msource);
	Kaba::DeclareClassVirtualIndex("MidiSource", "reset", Kaba::mf(&MidiSource::reset), &msource);
	Kaba::DeclareClassVirtualIndex("MidiSource", "get_pos", Kaba::mf(&MidiSource::get_pos), &msource);
	Kaba::DeclareClassVirtualIndex("MidiSource", "set_pos", Kaba::mf(&MidiSource::set_pos), &msource);
	Kaba::LinkExternal("MidiSource.note", Kaba::mf(&MidiSource::note));
	Kaba::LinkExternal("MidiSource.skip", Kaba::mf(&MidiSource::skip));
	Kaba::LinkExternal("MidiSource.note_x", Kaba::mf(&MidiSource::note_x));
	Kaba::LinkExternal("MidiSource.skip_x", Kaba::mf(&MidiSource::skip_x));


	BeatMidifier bmidifier;
	Kaba::DeclareClassSize("BeatMidifier", sizeof(BeatMidifier));
	Kaba::LinkExternal("BeatMidifier.__init__", Kaba::mf(&BeatMidifier::__init__));
	//Kaba::DeclareClassVirtualIndex("BeatMidifier", "__delete__", Kaba::mf(&MidiSource::__delete__), &bmidifier);
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
	Kaba::DeclareClassOffset("Synthesizer", "auto_generate_stereo", _offsetof(Synthesizer, auto_generate_stereo));
	Kaba::LinkExternal("Synthesizer.__init__", Kaba::mf(&Synthesizer::__init__));
	Kaba::DeclareClassVirtualIndex("Synthesizer", "__delete__", Kaba::mf(&Synthesizer::__delete__), &synth);
	Kaba::DeclareClassVirtualIndex("Synthesizer", "render", Kaba::mf(&Synthesizer::render), &synth);
	Kaba::DeclareClassVirtualIndex("Synthesizer", "create_pitch_renderer", Kaba::mf(&Synthesizer::create_pitch_renderer), &synth);
	Kaba::DeclareClassVirtualIndex("Synthesizer", "on_config", Kaba::mf(&Synthesizer::on_config), &synth);
	Kaba::DeclareClassVirtualIndex("Synthesizer", "reset_state", Kaba::mf(&Synthesizer::reset_state), &synth);
	Kaba::LinkExternal("Synthesizer.set_sample_rate", Kaba::mf(&Synthesizer::set_sample_rate));

	PitchRenderer pren(&synth, 0);
	Kaba::DeclareClassSize("PitchRenderer", sizeof(PitchRenderer));
	Kaba::DeclareClassOffset("PitchRenderer", "synth", _offsetof(PitchRenderer, synth));
	Kaba::DeclareClassOffset("PitchRenderer", "pitch", _offsetof(PitchRenderer, pitch));
	Kaba::DeclareClassOffset("PitchRenderer", "delta_phi", _offsetof(PitchRenderer, delta_phi));
	Kaba::LinkExternal("PitchRenderer.__init__", Kaba::mf(&PitchRenderer::__init__));
	Kaba::DeclareClassVirtualIndex("PitchRenderer", "__delete__", Kaba::mf(&PitchRenderer::__delete__), &pren);
	Kaba::DeclareClassVirtualIndex("PitchRenderer", "render", Kaba::mf(&PitchRenderer::render), &pren);
	Kaba::DeclareClassVirtualIndex("PitchRenderer", "on_start", Kaba::mf(&PitchRenderer::on_start), &pren);
	Kaba::DeclareClassVirtualIndex("PitchRenderer", "on_end", Kaba::mf(&PitchRenderer::on_end), &pren);
	Kaba::DeclareClassVirtualIndex("PitchRenderer", "on_config", Kaba::mf(&PitchRenderer::on_config), &pren);


	DummySynthesizer dsynth;
	Kaba::DeclareClassSize("DummySynthesizer", sizeof(DummySynthesizer));
	Kaba::LinkExternal("DummySynthesizer.__init__", Kaba::mf(&DummySynthesizer::__init__));
	Kaba::DeclareClassVirtualIndex("DummySynthesizer", "__delete__", Kaba::mf(&DummySynthesizer::__delete__), &dsynth);
	Kaba::DeclareClassVirtualIndex("DummySynthesizer", "render", Kaba::mf(&DummySynthesizer::render), &dsynth);
	Kaba::DeclareClassVirtualIndex("DummySynthesizer", "create_pitch_renderer", Kaba::mf(&DummySynthesizer::create_pitch_renderer), &dsynth);
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
	Kaba::DeclareClassOffset("BarPattern", "beats", _offsetof(BarPattern, beats));
	Kaba::DeclareClassOffset("BarPattern", "divisor", _offsetof(BarPattern, divisor));
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
	Kaba::DeclareClassOffset("MidiNote", "flags", _offsetof(MidiNote, flags));
	Kaba::LinkExternal("MidiNote.copy", Kaba::mf(&MidiNote::copy));
	Kaba::LinkExternal("MidiNote.is", Kaba::mf(&MidiNote::is));
	Kaba::LinkExternal("MidiNote.set", Kaba::mf(&MidiNote::set));
	
	Kaba::DeclareClassSize("MidiEvent", sizeof(MidiEvent));
	Kaba::DeclareClassOffset("MidiEvent", "pos", _offsetof(MidiEvent, pos));
	Kaba::DeclareClassOffset("MidiEvent", "pitch", _offsetof(MidiEvent, pitch));
	Kaba::DeclareClassOffset("MidiEvent", "volume", _offsetof(MidiEvent, volume));
	Kaba::DeclareClassOffset("MidiEvent", "stringno", _offsetof(MidiEvent, stringno));
	Kaba::DeclareClassOffset("MidiEvent", "clef_position", _offsetof(MidiEvent, clef_position));
	Kaba::DeclareClassOffset("MidiEvent", "flags", _offsetof(MidiEvent, flags));

	Kaba::DeclareClassSize("MidiEventBuffer", sizeof(MidiEventBuffer));
	Kaba::DeclareClassOffset("MidiEventBuffer", "samples", _offsetof(MidiEventBuffer, samples));
	Kaba::LinkExternal("MidiEventBuffer.__init__", Kaba::mf(&MidiEventBuffer::__init__));
	Kaba::LinkExternal("MidiEventBuffer.get_events", Kaba::mf(&MidiEventBuffer::get_events));
	Kaba::LinkExternal("MidiEventBuffer.get_notes", Kaba::mf(&MidiEventBuffer::get_notes));
	Kaba::LinkExternal("MidiEventBuffer.get_range", Kaba::mf(&MidiEventBuffer::range));
	Kaba::LinkExternal("MidiEventBuffer.add_metronome_click", Kaba::mf(&MidiEventBuffer::add_metronome_click));

	Kaba::DeclareClassSize("MidiNoteBuffer", sizeof(MidiNoteBuffer));
	Kaba::DeclareClassOffset("MidiNoteBuffer", "samples", _offsetof(MidiNoteBuffer, samples));
	Kaba::LinkExternal("MidiNoteBuffer.__init__", Kaba::mf(&MidiNoteBuffer::__init__));
	Kaba::LinkExternal("MidiNoteBuffer.get_events", Kaba::mf(&MidiNoteBuffer::get_events));
	Kaba::LinkExternal("MidiNoteBuffer.get_notes", Kaba::mf(&MidiNoteBuffer::get_notes));
	Kaba::LinkExternal("MidiNoteBuffer.get_range", Kaba::mf(&MidiNoteBuffer::range));

	BeatSource bsource;
	Kaba::DeclareClassSize("BeatSource", sizeof(BeatSource));
	Kaba::LinkExternal("BeatSource.__init__", Kaba::mf(&BeatSource::__init__));
	Kaba::DeclareClassVirtualIndex("BeatSource", "__delete__", Kaba::mf(&BeatSource::__delete__), &bsource);
	Kaba::DeclareClassVirtualIndex("BeatSource", "read", Kaba::mf(&BeatSource::read), &bsource);
	Kaba::DeclareClassVirtualIndex("BeatSource", "reset", Kaba::mf(&BeatSource::reset), &bsource);
	Kaba::DeclareClassVirtualIndex("BeatSource", "get_pos", Kaba::mf(&BeatSource::get_pos), &bsource);
	Kaba::DeclareClassVirtualIndex("BeatSource", "set_pos", Kaba::mf(&BeatSource::set_pos), &bsource);
	Kaba::DeclareClassVirtualIndex("BeatSource", "beats_per_bar", Kaba::mf(&BeatSource::beats_per_bar), &bsource);
	Kaba::DeclareClassVirtualIndex("BeatSource", "cur_beat", Kaba::mf(&BeatSource::cur_beat), &bsource);
	Kaba::DeclareClassVirtualIndex("BeatSource", "cur_bar", Kaba::mf(&BeatSource::cur_bar), &bsource);
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
	Kaba::LinkExternal("TrackLayer.get_buffers", Kaba::mf(&TrackLayer::get_buffers));
	Kaba::LinkExternal("TrackLayer.read_buffers", Kaba::mf(&TrackLayer::read_buffers));
	Kaba::LinkExternal("TrackLayer.insert_midi_data", Kaba::mf(&TrackLayer::insert_midi_data));
	Kaba::LinkExternal("TrackLayer.add_midi_note", Kaba::mf(&TrackLayer::add_midi_note));
	//Kaba::LinkExternal("TrackLayer.add_midi_notes", Kaba::mf(&TrackLayer::addMidiNotes));
	Kaba::LinkExternal("TrackLayer.delete_midi_note", Kaba::mf(&TrackLayer::delete_midi_note));
	Kaba::LinkExternal("TrackLayer.add_sample_ref", Kaba::mf(&TrackLayer::add_sample_ref));
	Kaba::LinkExternal("TrackLayer.delete_sample_ref", Kaba::mf(&TrackLayer::delete_sample_ref));
	Kaba::LinkExternal("TrackLayer.edit_sample_ref", Kaba::mf(&TrackLayer::edit_sample_ref));

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
	Kaba::LinkExternal("Track.nice_name", Kaba::mf(&Track::nice_name));
	Kaba::LinkExternal("Track.set_name", Kaba::mf(&Track::set_name));
	Kaba::LinkExternal("Track.set_muted", Kaba::mf(&Track::set_muted));
	Kaba::LinkExternal("Track.set_volume", Kaba::mf(&Track::set_volume));
	Kaba::LinkExternal("Track.set_panning", Kaba::mf(&Track::set_panning));
	Kaba::LinkExternal("Track.add_effect", Kaba::mf(&Track::add_effect));
	Kaba::LinkExternal("Track.delete_effect", Kaba::mf(&Track::delete_effect));
	Kaba::LinkExternal("Track.edit_effect", Kaba::mf(&Track::edit_effect));
	Kaba::LinkExternal("Track.enable_effect", Kaba::mf(&Track::enable_effect));
	Kaba::LinkExternal("Track.set_synthesizer", Kaba::mf(&Track::set_synthesizer));
	Kaba::LinkExternal("Track.add_marker", Kaba::mf(&Track::add_marker));
	Kaba::LinkExternal("Track.delete_marker", Kaba::mf(&Track::delete_marker));
	Kaba::LinkExternal("Track.edit_marker", Kaba::mf(&Track::edit_marker));

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
	Kaba::LinkExternal("Song.__init__", Kaba::mf(&Song::__init__));
	Kaba::DeclareClassVirtualIndex("Song", "__delete__", Kaba::mf(&Song::__delete__), &af);
	Kaba::LinkExternal("Song.add_track", Kaba::mf(&Song::add_track));
	Kaba::LinkExternal("Song.delete_track", Kaba::mf(&Song::delete_track));
	Kaba::LinkExternal("Song.range", Kaba::mf(&Song::range));
	Kaba::LinkExternal("Song.layers", Kaba::mf(&Song::layers));
	Kaba::LinkExternal("Song.add_bar", Kaba::mf(&Song::add_bar));
	Kaba::LinkExternal("Song.add_pause", Kaba::mf(&Song::add_pause));
	Kaba::LinkExternal("Song.edit_bar", Kaba::mf(&Song::edit_bar));
	Kaba::LinkExternal("Song.delete_bar", Kaba::mf(&Song::delete_bar));
	Kaba::LinkExternal("Song.add_sample", Kaba::mf(&Song::add_sample));
	Kaba::LinkExternal("Song.delete_sample", Kaba::mf(&Song::delete_sample));
	Kaba::LinkExternal("Song.time_track", Kaba::mf(&Song::time_track));
	Kaba::LinkExternal("Song.begin_action_group", Kaba::mf(&Song::begin_action_group));
	Kaba::LinkExternal("Song.end_action_group", Kaba::mf(&Song::end_action_group));

	SongRenderer sr(&af);
	Kaba::DeclareClassSize("SongRenderer", sizeof(SongRenderer));
	Kaba::LinkExternal("SongRenderer.prepare", Kaba::mf(&SongRenderer::prepare));
	Kaba::LinkExternal("SongRenderer.render", Kaba::mf(&SongRenderer::render));
	Kaba::LinkExternal("SongRenderer.__init__", Kaba::mf(&SongRenderer::__init__));
	Kaba::DeclareClassVirtualIndex("SongRenderer", "__delete__", Kaba::mf(&SongRenderer::__delete__), &sr);
	Kaba::DeclareClassVirtualIndex("SongRenderer", "read", Kaba::mf(&SongRenderer::read), &sr);
	Kaba::DeclareClassVirtualIndex("SongRenderer", "reset", Kaba::mf(&SongRenderer::reset_state), &sr);
	Kaba::DeclareClassVirtualIndex("SongRenderer", "get_pos", Kaba::mf(&SongRenderer::get_pos), &sr);
	Kaba::LinkExternal("SongRenderer.range", Kaba::mf(&SongRenderer::range));
	Kaba::DeclareClassVirtualIndex("SongRenderer", "set_pos", Kaba::mf(&SongRenderer::set_pos), &sr);
	Kaba::DeclareClassVirtualIndex("SongRenderer", "get_pos", Kaba::mf(&SongRenderer::get_pos), &sr);
	Kaba::LinkExternal("SongRenderer.get_beat_source", Kaba::mf(&SongRenderer::get_beat_source));

	{
	AudioInput input(Session::GLOBAL);
	Kaba::DeclareClassSize("InputStreamAudio", sizeof(AudioInput));
	Kaba::DeclareClassOffset("InputStreamAudio", "current_buffer", _offsetof(AudioInput, buffer));
	Kaba::DeclareClassOffset("InputStreamAudio", "out", _offsetof(AudioInput, out));
//	Kaba::DeclareClassOffset("InputStreamAudio", "capturing", _offsetof(InputStreamAudio, capturing));
	Kaba::LinkExternal("InputStreamAudio.__init__", Kaba::mf(&AudioInput::__init__));
	Kaba::DeclareClassVirtualIndex("InputStreamAudio", "__delete__", Kaba::mf(&AudioInput::__delete__), &input);
	Kaba::LinkExternal("InputStreamAudio.start", Kaba::mf(&AudioInput::start));
	Kaba::LinkExternal("InputStreamAudio.stop",	 Kaba::mf(&AudioInput::stop));
	Kaba::LinkExternal("InputStreamAudio.is_capturing", Kaba::mf(&AudioInput::is_capturing));
	Kaba::LinkExternal("InputStreamAudio.sample_rate", Kaba::mf(&AudioInput::sample_rate));
	//Kaba::LinkExternal("InputStreamAudio.set_backup_mode", Kaba::mf(&InputStreamAudio::set_backup_mode));
	}

	{
	AudioOutput stream(Session::GLOBAL);
	Kaba::DeclareClassSize("OutputStream", sizeof(AudioOutput));
	Kaba::LinkExternal("OutputStream.__init__", Kaba::mf(&AudioOutput::__init__));
	Kaba::DeclareClassVirtualIndex("OutputStream", "__delete__", Kaba::mf(&AudioOutput::__delete__), &stream);
	//Kaba::LinkExternal("OutputStream.setSource", Kaba::mf(&AudioStream::setSource));
	Kaba::LinkExternal("OutputStream.start", Kaba::mf(&AudioOutput::start));
	Kaba::LinkExternal("OutputStream.stop", Kaba::mf(&AudioOutput::stop));
	Kaba::LinkExternal("OutputStream.is_playing", Kaba::mf(&AudioOutput::is_playing));
	//Kaba::LinkExternal("OutputStream.sample_rate", Kaba::mf(&OutputStream::sample_rate));
	Kaba::LinkExternal("OutputStream.get_volume", Kaba::mf(&AudioOutput::get_volume));
	Kaba::LinkExternal("OutputStream.set_volume", Kaba::mf(&AudioOutput::set_volume));
	Kaba::DeclareClassVirtualIndex("OutputStream", "reset_state", Kaba::mf(&AudioOutput::reset_state), &stream);
	}

	SignalChain chain(Session::GLOBAL, "");
	Kaba::DeclareClassSize("SignalChain", sizeof(SignalChain));
	Kaba::LinkExternal("SignalChain.set_update_dt", Kaba::mf(&SignalChain::set_tick_dt));
	Kaba::LinkExternal("SignalChain.set_buffer_size", Kaba::mf(&SignalChain::set_buffer_size));
	Kaba::DeclareClassVirtualIndex("SignalChain", "set_pos", Kaba::mf(&SignalChain::set_pos), &chain);
	Kaba::DeclareClassVirtualIndex("SignalChain", "get_pos", Kaba::mf(&SignalChain::get_pos), &chain);

	Kaba::DeclareClassSize("AudioView", sizeof(AudioView));
	Kaba::DeclareClassOffset("AudioView", "sel", _offsetof(AudioView, sel));
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
	Kaba::LinkExternal("Storage.save_via_renderer", Kaba::mf(&Storage::save_via_renderer));
	Kaba::LinkExternal("Storage.load_buffer", Kaba::mf(&Storage::load_buffer));
	Kaba::DeclareClassOffset("Storage", "current_directory", _offsetof(Storage, current_directory));


	Slider slider;
	Kaba::DeclareClassSize("Slider", sizeof(Slider));
	Kaba::LinkExternal("Slider.__init__", Kaba::mf(&Slider::__init_ext__));
	Kaba::DeclareClassVirtualIndex("Slider", "__delete__", Kaba::mf(&Slider::__delete__), &slider);
	Kaba::LinkExternal("Slider.get", Kaba::mf(&Slider::get));
	Kaba::LinkExternal("Slider.set", Kaba::mf(&Slider::set));

	SongPlugin song_plugin;
	Kaba::DeclareClassSize("SongPlugin", sizeof(SongPlugin));
	Kaba::DeclareClassOffset("SongPlugin", "session", _offsetof(SongPlugin, session));
	Kaba::DeclareClassOffset("SongPlugin", "song", _offsetof(SongPlugin, song));
	Kaba::LinkExternal("SongPlugin.__init__", Kaba::mf(&SongPlugin::__init__));
	Kaba::DeclareClassVirtualIndex("SongPlugin", "__delete__", Kaba::mf(&SongPlugin::__delete__), &song_plugin);
	Kaba::DeclareClassVirtualIndex("SongPlugin", "apply", Kaba::mf(&SongPlugin::apply), &song_plugin);

	TsunamiPlugin tsunami_plugin;
	Kaba::DeclareClassSize("TsunamiPlugin", sizeof(TsunamiPlugin));
	//Kaba::DeclareClassOffset("TsunamiPlugin", "session", _offsetof(TsunamiPlugin, session));
	Kaba::DeclareClassOffset("TsunamiPlugin", "args", _offsetof(TsunamiPlugin, args));
	Kaba::LinkExternal("TsunamiPlugin.__init__", Kaba::mf(&TsunamiPlugin::__init__));
	Kaba::DeclareClassVirtualIndex("TsunamiPlugin", "__delete__", Kaba::mf(&TsunamiPlugin::__delete__), &tsunami_plugin);
	Kaba::DeclareClassVirtualIndex("TsunamiPlugin", "on_start", Kaba::mf(&TsunamiPlugin::on_start), &tsunami_plugin);
	Kaba::DeclareClassVirtualIndex("TsunamiPlugin", "on_stop", Kaba::mf(&TsunamiPlugin::on_stop), &tsunami_plugin);
	Kaba::LinkExternal("TsunamiPlugin.stop", Kaba::mf(&TsunamiPlugin::stop_request));

	Kaba::DeclareClassSize("Progress", sizeof(Progress));
	Kaba::LinkExternalClassFunc("Progress.__init__", &Progress::__init__);
	Kaba::LinkExternalClassFunc("Progress.__delete__", &Progress::__delete__);
	Kaba::LinkExternalClassFunc("Progress.set", &Progress::set_kaba);

	ProgressCancelable prog_can;
	Kaba::DeclareClassSize("ProgressX", sizeof(ProgressCancelable));
	Kaba::LinkExternalClassFunc("ProgressX.__init__", &ProgressCancelable::__init__);
	Kaba::DeclareClassVirtualIndex("ProgressX", "__delete__", Kaba::mf(&ProgressCancelable::__delete__), &prog_can);
	Kaba::LinkExternalClassFunc("ProgressX.set", &ProgressCancelable::set_kaba);
	Kaba::LinkExternalClassFunc("ProgressX.cancel", &ProgressCancelable::cancel);
	Kaba::LinkExternalClassFunc("ProgressX.is_cancelled", &ProgressCancelable::is_cancelled);
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

void find_plugins_in_dir_absolute(const string &_dir, const string &group, ModuleType type, PluginManager *pm)
{
	string dir = _dir;
	if (group.num > 0)
		dir += group + "/";
	Array<DirEntry> list = dir_search(dir, "*.kaba", false);
	for (DirEntry &e : list){
		PluginManager::PluginFile pf;
		pf.type = type;
		pf.name = e.name.replace(".kaba", "");
		pf.filename = dir + e.name;
		pf.group = group;
		get_plugin_file_data(pf);
		pm->plugin_files.add(pf);
	}
}

void find_plugins_in_dir(const string &rel, const string &group, ModuleType type, PluginManager *pm)
{
	find_plugins_in_dir_absolute(pm->plugin_dir_static() + rel, group, type, pm);
	if (pm->plugin_dir_local() != pm->plugin_dir_static())
		find_plugins_in_dir_absolute(pm->plugin_dir_local() + rel, group, type, pm);
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

void PluginManager::find_plugins()
{
	Kaba::Init();
	Kaba::config.show_compiler_stats = false;
	Kaba::config.compile_silently = true;

	// "AudioSource"
	find_plugins_in_dir("AudioSource/", "", ModuleType::AUDIO_SOURCE, this);

	// "AudioEffect"
	find_plugins_in_dir("AudioEffect/", "Channels", ModuleType::AUDIO_EFFECT, this);
	find_plugins_in_dir("AudioEffect/", "Dynamics", ModuleType::AUDIO_EFFECT, this);
	find_plugins_in_dir("AudioEffect/", "Echo", ModuleType::AUDIO_EFFECT, this);
	find_plugins_in_dir("AudioEffect/", "Pitch", ModuleType::AUDIO_EFFECT, this);
	find_plugins_in_dir("AudioEffect/", "Repair", ModuleType::AUDIO_EFFECT, this);
	find_plugins_in_dir("AudioEffect/", "Sound", ModuleType::AUDIO_EFFECT, this);
	find_plugins_in_dir("AudioEffect/", "Synthesizer", ModuleType::AUDIO_EFFECT, this);

	// "AudioVisualizer"
	find_plugins_in_dir("AudioVisualizer/", "", ModuleType::AUDIO_VISUALIZER, this);

	// "MidiSource"
	find_plugins_in_dir("MidiSource/", "", ModuleType::MIDI_SOURCE, this);

	// "MidiEffect"
	find_plugins_in_dir("MidiEffect/", "", ModuleType::MIDI_EFFECT, this);

	// "BeatSource"
	find_plugins_in_dir("BeatSource/", "", ModuleType::BEAT_SOURCE, this);

	// "All"
	find_plugins_in_dir("All/", "", ModuleType::SONG_PLUGIN, this);

	// rest
	find_plugins_in_dir("Independent/", "", ModuleType::TSUNAMI_PLUGIN, this);

	// "Synthesizer"
	find_plugins_in_dir("Synthesizer/", "", ModuleType::SYNTHESIZER, this);
}

void PluginManager::add_plugins_to_menu(TsunamiWindow *win)
{
	hui::Menu *m = win->get_menu();

	// "Buffer"
	add_plugins_in_dir("AudioEffect/Channels/", this, m->get_sub_menu_by_id("menu_plugins_channels"), "audio-effect", win, &TsunamiWindow::on_menu_execute_audio_effect);
	add_plugins_in_dir("AudioEffect/Dynamics/", this, m->get_sub_menu_by_id("menu_plugins_dynamics"), "audio-effect", win, &TsunamiWindow::on_menu_execute_audio_effect);
	add_plugins_in_dir("AudioEffect/Echo/", this, m->get_sub_menu_by_id("menu_plugins_echo"), "audio-effect", win, &TsunamiWindow::on_menu_execute_audio_effect);
	add_plugins_in_dir("AudioEffect/Pitch/", this, m->get_sub_menu_by_id("menu_plugins_pitch"), "audio-effect", win, &TsunamiWindow::on_menu_execute_audio_effect);
	add_plugins_in_dir("AudioEffect/Repair/", this, m->get_sub_menu_by_id("menu_plugins_repair"), "audio-effect", win, &TsunamiWindow::on_menu_execute_audio_effect);
	add_plugins_in_dir("AudioEffect/Sound/", this, m->get_sub_menu_by_id("menu_plugins_sound"), "audio-effect", win, &TsunamiWindow::on_menu_execute_audio_effect);
	add_plugins_in_dir("AudioEffect/Synthesizer/", this, m->get_sub_menu_by_id("menu_plugins_synthesizer"), "audio-effect", win, &TsunamiWindow::on_menu_execute_audio_effect);

	add_plugins_in_dir("AudioSource/", this, m->get_sub_menu_by_id("menu_plugins_audio_source"), "source", win, &TsunamiWindow::on_menu_execute_audio_source);

	// "Midi"
	add_plugins_in_dir("MidiEffect/", this, m->get_sub_menu_by_id("menu_plugins_midi_effects"), "midi-effect", win, &TsunamiWindow::on_menu_execute_midi_effect);
	add_plugins_in_dir("MidiSource/", this, m->get_sub_menu_by_id("menu_plugins_midi_source"), "midi-source", win, &TsunamiWindow::on_menu_execute_midi_source);

	// "All"
	add_plugins_in_dir("All/", this, m->get_sub_menu_by_id("menu_plugins_on_all"), "song", win, &TsunamiWindow::on_menu_execute_song_plugin);

	// rest
	add_plugins_in_dir("Independent/", this, m->get_sub_menu_by_id("menu_plugins_other"), "tsunami", win, &TsunamiWindow::on_menu_execute_tsunami_plugin);
}

void PluginManager::apply_favorite(Module *c, const string &name)
{
	favorites->apply(c, name);
}

void PluginManager::save_favorite(Module *c, const string &name)
{
	favorites->save(c, name);
}


string PluginManager::select_favorite_name(hui::Window *win, Module *c, bool save)
{
	return favorites->select_name(win, c, save);
}

// always push the script... even if an error occurred
//   don't log error...
Plugin *PluginManager::load_and_compile_plugin(ModuleType type, const string &filename)
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


Plugin *PluginManager::get_plugin(Session *session, ModuleType type, const string &name)
{
	for (PluginFile &pf: plugin_files){
		if ((pf.name.replace(" ", "") == name.replace(" ", "")) and (pf.type == type)){
			Plugin *p = load_and_compile_plugin(type, pf.filename);
			return p;
		}
	}
	session->e(format(_("Can't find %s plugin: %s ..."), Module::type_to_name(type).c_str(), name.c_str()));
	return nullptr;
}

string PluginManager::plugin_dir_static()
{
	if (tsunami->installed)
		return tsunami->directory_static + "Plugins/";
	return "Plugins/";
}

string PluginManager::plugin_dir_local()
{
	if (tsunami->installed)
		return tsunami->directory + "Plugins/";
	return "Plugins/";
}


Array<string> PluginManager::find_module_sub_types(ModuleType type)
{
	Array<string> names;
	for (auto &pf: plugin_files)
		if (pf.type == type)
			names.add(pf.name);

	if (type == ModuleType::AUDIO_SOURCE){
		names.add("SongRenderer");
		//names.add("BufferStreamer");
	}else if (type == ModuleType::MIDI_EFFECT){
		names.add("Dummy");
	}else if (type == ModuleType::BEAT_SOURCE){
		//names.add("BarStreamer");
	}else if (type == ModuleType::AUDIO_VISUALIZER){
		names.add("PeakMeter");
	}else if (type == ModuleType::SYNTHESIZER){
		names.add("Dummy");
		//names.add("Sample");
	}else if (type == ModuleType::STREAM){
		names.add("AudioInput");
		names.add("AudioOutput");
		names.add("MidiInput");
	}else if (type == ModuleType::PLUMBING){
		names.add("AudioBackup");
		names.add("AudioJoiner");
		names.add("AudioRecorder");
		names.add("AudioSucker");
		names.add("BeatMidifier");
		names.add("MidiRecorder");
		names.add("MidiSucker");
	}
	return names;
}

Array<string> PluginManager::find_module_sub_types_grouped(ModuleType type)
{
	if (type == ModuleType::AUDIO_EFFECT){
		Array<string> names;
		for (auto &pf: plugin_files)
			if (pf.type == type)
				names.add(pf.group + "/" + pf.name);
		return names;
	}
	return find_module_sub_types(type);
}

string PluginManager::choose_module(hui::Panel *parent, Session *session, ModuleType type, const string &old_name)
{
	Array<string> names = session->plugin_manager->find_module_sub_types(type);
	if (names.num == 1)
		return names[0];
	if (names.num == 0)
		return "";

	ConfigurableSelectorDialog *dlg = new ConfigurableSelectorDialog(parent->win, type, session, old_name);
	dlg->run();
	string name = dlg->_return;
	delete(dlg);
	return name;
}


