/*
 * PluginManager.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "PluginManager.h"
#include "../Tsunami.h"
#include "../TsunamiWindow.h"
#include "FastFourierTransform.h"
#include "ExtendedBufferBox.h"
#include "../View/Helper/Slider.h"
#include "../Device/InputStreamAudio.h"
#include "../Device/InputStreamMidi.h"
#include "../Device/InputStreamAny.h"
#include "../Device/OutputStream.h"
#include "../Device/DeviceManager.h"
#include "../Audio/Renderer/MidiRenderer.h"
#include "../Audio/Synth/Synthesizer.h"
#include "../Audio/Synth/DummySynthesizer.h"
#include "../Audio/Renderer/SongRenderer.h"
#include "../Midi/MidiSource.h"
#include "../View/Helper/Progress.h"
#include "../Storage/Storage.h"
#include "../Stuff/Log.h"
#include "../View/AudioView.h"
#include "../View/Dialog/ConfigurableSelectorDialog.h"
#include "../View/SideBar/SampleManagerConsole.h"
#include "Plugin.h"
#include "Effect.h"
#include "ConfigPanel.h"
#include "MidiEffect.h"
#include "SongPlugin.h"
#include "TsunamiPlugin.h"
#include "FavoriteManager.h"

#define _offsetof(CLASS, ELEMENT) (int)( (char*)&((CLASS*)1)->ELEMENT - (char*)((CLASS*)1) )



PluginManager::PluginManager()
{
	favorites = new FavoriteManager;

	FindPlugins();
	LinkAppScriptData();
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

Song* getCurSong()
{
	return tsunami->song;
}


void GlobalSetTempBackupFilename(const string &filename)
{
	InputStreamAudio::setTempBackupFilename(filename);
}

void PluginManager::LinkAppScriptData()
{
	Kaba::config.directory = "";

	// api definition
	Kaba::LinkExternal("device_manager", &tsunami->device_manager);
	Kaba::LinkExternal("storage", &tsunami->storage);
	Kaba::LinkExternal("logging", &tsunami->log);
	Kaba::LinkExternal("colors", &tsunami->_view->colors);
	Kaba::LinkExternal("fft_c2c", (void*)&FastFourierTransform::fft_c2c);
	Kaba::LinkExternal("fft_r2c", (void*)&FastFourierTransform::fft_r2c);
	Kaba::LinkExternal("fft_c2r_inv", (void*)&FastFourierTransform::fft_c2r_inv);
	Kaba::LinkExternal("getCurSong", (void*)&getCurSong);
	Kaba::LinkExternal("CreateSynthesizer", (void*)&CreateSynthesizer);
	Kaba::LinkExternal("CreateAudioEffect", (void*)&CreateEffect);
	Kaba::LinkExternal("CreateMidiEffect", (void*)&CreateMidiEffect);
	Kaba::LinkExternal("AllowTermination", (void*)&GlobalAllowTermination);
	Kaba::LinkExternal("SetTempBackupFilename", (void*)&GlobalSetTempBackupFilename);
	Kaba::LinkExternal("SelectSample", (void*)&SampleManagerConsole::select);

	Kaba::DeclareClassSize("Range", sizeof(Range));
	Kaba::DeclareClassOffset("Range", "offset", _offsetof(Range, offset));
	Kaba::DeclareClassOffset("Range", "length", _offsetof(Range, length));


	PluginData plugin_data;
	Kaba::DeclareClassSize("PluginData", sizeof(PluginData));
	Kaba::LinkExternal("PluginData." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&PluginData::__init__));
	Kaba::DeclareClassVirtualIndex("PluginData", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&PluginData::__delete__), &plugin_data);
	Kaba::DeclareClassVirtualIndex("PluginData", "reset", Kaba::mf(&PluginData::reset), &plugin_data);


	ConfigPanel config_panel;
	Kaba::DeclareClassSize("ConfigPanel", sizeof(ConfigPanel));
	Kaba::LinkExternal("ConfigPanel." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&ConfigPanel::__init__));
	Kaba::DeclareClassVirtualIndex("ConfigPanel", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&ConfigPanel::__delete__), &config_panel);
	Kaba::DeclareClassVirtualIndex("ConfigPanel", "update", Kaba::mf(&ConfigPanel::update), &config_panel);
	Kaba::LinkExternal("ConfigPanel.notify", Kaba::mf(&ConfigPanel::notify));
	Kaba::DeclareClassOffset("ConfigPanel", "c", _offsetof(ConfigPanel, c));


	Effect effect;
	Kaba::DeclareClassSize("AudioEffect", sizeof(Effect));
	Kaba::DeclareClassOffset("AudioEffect", "name", _offsetof(Effect, name));
	Kaba::DeclareClassOffset("AudioEffect", "only_on_selection", _offsetof(Effect, only_on_selection));
	Kaba::DeclareClassOffset("AudioEffect", "range", _offsetof(Effect, range));
	Kaba::DeclareClassOffset("AudioEffect", "usable", _offsetof(Effect, usable));
	Kaba::DeclareClassOffset("AudioEffect", "song", _offsetof(Effect, song));
	Kaba::DeclareClassOffset("AudioEffect", "track", _offsetof(Effect, track));
	Kaba::DeclareClassOffset("AudioEffect", "range", _offsetof(Effect, range));
	Kaba::DeclareClassOffset("AudioEffect", "layer", _offsetof(Effect, layer));
	Kaba::LinkExternal("AudioEffect." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&Effect::__init__));
	Kaba::DeclareClassVirtualIndex("AudioEffect", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&Effect::__delete__), &effect);
	Kaba::DeclareClassVirtualIndex("AudioEffect", "process", Kaba::mf(&Effect::processTrack), &effect);
	Kaba::DeclareClassVirtualIndex("AudioEffect", "createPanel", Kaba::mf(&Effect::createPanel), &effect);
	Kaba::LinkExternal("AudioEffect.resetConfig", Kaba::mf(&Effect::resetConfig));
	Kaba::LinkExternal("AudioEffect.resetState", Kaba::mf(&Effect::resetState));
	//Kaba::DeclareClassVirtualIndex("AudioEffect", "updateDialog", Kaba::mf(&Effect::UpdateDialog), &effect);
	Kaba::LinkExternal("AudioEffect.notify", Kaba::mf(&Effect::notify));
	Kaba::DeclareClassVirtualIndex("AudioEffect", "onConfig", Kaba::mf(&Effect::onConfig), &effect);

	MidiEffect midieffect;
	Kaba::DeclareClassSize("MidiEffect", sizeof(MidiEffect));
	Kaba::DeclareClassOffset("MidiEffect", "name", _offsetof(MidiEffect, name));
	Kaba::DeclareClassOffset("MidiEffect", "only_on_selection", _offsetof(MidiEffect, only_on_selection));
	Kaba::DeclareClassOffset("MidiEffect", "range", _offsetof(MidiEffect, range));
	Kaba::DeclareClassOffset("MidiEffect", "usable", _offsetof(MidiEffect, usable));
	Kaba::DeclareClassOffset("MidiEffect", "song", _offsetof(MidiEffect, song));
	Kaba::LinkExternal("MidiEffect." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&MidiEffect::__init__));
	Kaba::DeclareClassVirtualIndex("MidiEffect", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&MidiEffect::__delete__), &midieffect);
	Kaba::DeclareClassVirtualIndex("MidiEffect", "process", Kaba::mf(&MidiEffect::process), &midieffect);
	Kaba::DeclareClassVirtualIndex("MidiEffect", "createPanel", Kaba::mf(&MidiEffect::createPanel), &midieffect);
	Kaba::LinkExternal("MidiEffect.resetConfig", Kaba::mf(&MidiEffect::resetConfig));
	Kaba::LinkExternal("MidiEffect.resetState", Kaba::mf(&MidiEffect::resetState));
	//Kaba::DeclareClassVirtualIndex("MidiEffect", "updateDialog", Kaba::mf(&MidiEffect::UpdateDialog), &midieffect);
	Kaba::LinkExternal("MidiEffect.notify", Kaba::mf(&MidiEffect::notify));
	Kaba::DeclareClassVirtualIndex("MidiEffect", "onConfig", Kaba::mf(&MidiEffect::onConfig), &midieffect);

	Kaba::DeclareClassSize("BufferBox", sizeof(BufferBox));
	Kaba::DeclareClassOffset("BufferBox", "offset", _offsetof(BufferBox, offset));
	Kaba::DeclareClassOffset("BufferBox", "length", _offsetof(BufferBox, length));
	Kaba::DeclareClassOffset("BufferBox", "channels", _offsetof(BufferBox, channels));
	Kaba::DeclareClassOffset("BufferBox", "r", _offsetof(BufferBox, c[0]));
	Kaba::DeclareClassOffset("BufferBox", "l", _offsetof(BufferBox, c[1]));
	Kaba::DeclareClassOffset("BufferBox", "peaks", _offsetof(BufferBox, peaks));
	Kaba::LinkExternal("BufferBox." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&BufferBox::__init__));
	Kaba::LinkExternal("BufferBox." + Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&BufferBox::__delete__));
	Kaba::LinkExternal("BufferBox.clear", Kaba::mf(&BufferBox::clear));
	Kaba::LinkExternal("BufferBox." + Kaba::IDENTIFIER_FUNC_ASSIGN, Kaba::mf(&BufferBox::__assign__));
	Kaba::LinkExternal("BufferBox.range", Kaba::mf(&BufferBox::range));
	Kaba::LinkExternal("BufferBox.add", Kaba::mf(&BufferBox::add));
	Kaba::LinkExternal("BufferBox.set", Kaba::mf(&BufferBox::set));
	Kaba::LinkExternal("BufferBox.get_spectrum", Kaba::mf(&ExtendedBufferBox::get_spectrum));


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
	sample.owner = tsunami->song;
	SampleRef sampleref(&sample);
	Kaba::DeclareClassSize("SampleRef", sizeof(SampleRef));
	Kaba::DeclareClassOffset("SampleRef", "buf", _offsetof(SampleRef, buf));
	Kaba::DeclareClassOffset("SampleRef", "midi", _offsetof(SampleRef, midi));
	Kaba::DeclareClassOffset("SampleRef", "origin", _offsetof(SampleRef, origin));
	Kaba::LinkExternal("SampleRef." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&SampleRef::__init__));
	Kaba::DeclareClassVirtualIndex("SampleRef", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&SampleRef::__delete__), &sampleref);

	MidiSource midi_source;
	Kaba::DeclareClassSize("MidiSource", sizeof(MidiSource));
	Kaba::LinkExternal("MidiSource." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&MidiSource::__init__));
	Kaba::DeclareClassVirtualIndex("MidiSource", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&MidiSource::__delete__), &midi_source);
	Kaba::DeclareClassVirtualIndex("MidiSource", "read", Kaba::mf(&MidiSource::read), &midi_source);

	Synthesizer synth;
	Kaba::DeclareClassSize("Synthesizer", sizeof(Synthesizer));
	Kaba::DeclareClassOffset("Synthesizer", "name", _offsetof(Synthesizer, name));
	Kaba::DeclareClassOffset("Synthesizer", "sample_rate", _offsetof(Synthesizer, sample_rate));
	Kaba::DeclareClassOffset("Synthesizer", "events", _offsetof(Synthesizer, events));
	Kaba::DeclareClassOffset("Synthesizer", "keep_notes", _offsetof(Synthesizer, keep_notes));
	Kaba::DeclareClassOffset("Synthesizer", "active_pitch", _offsetof(Synthesizer, active_pitch));
	Kaba::DeclareClassOffset("Synthesizer", "freq", _offsetof(Synthesizer, tuning.freq));
	Kaba::DeclareClassOffset("Synthesizer", "delta_phi", _offsetof(Synthesizer, delta_phi));
	Kaba::LinkExternal("Synthesizer." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&Synthesizer::__init__));
	Kaba::DeclareClassVirtualIndex("Synthesizer", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&Synthesizer::__delete__), &synth);
	Kaba::DeclareClassVirtualIndex("Synthesizer", "createPanel", Kaba::mf(&Synthesizer::createPanel), &synth);
	Kaba::LinkExternal("Synthesizer.resetConfig", Kaba::mf(&Synthesizer::resetConfig));
	Kaba::LinkExternal("Synthesizer.resetState", Kaba::mf(&Synthesizer::resetState));
	Kaba::LinkExternal("Synthesizer.enablePitch", Kaba::mf(&Synthesizer::enablePitch));
	Kaba::DeclareClassVirtualIndex("Synthesizer", "render", Kaba::mf(&Synthesizer::render), &synth);
	Kaba::DeclareClassVirtualIndex("Synthesizer", "onConfig", Kaba::mf(&Synthesizer::onConfig), &synth);
	Kaba::LinkExternal("Synthesizer.setSampleRate", Kaba::mf(&Synthesizer::setSampleRate));
	Kaba::LinkExternal("Synthesizer.notify", Kaba::mf(&Synthesizer::notify));

	MidiRenderer midiren(NULL, NULL);
	Kaba::DeclareClassSize("MidiRenderer", sizeof(MidiRenderer));
	Kaba::LinkExternal("MidiRenderer." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&MidiRenderer::__init__));
	Kaba::DeclareClassVirtualIndex("MidiRenderer", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&MidiRenderer::__delete__), &midiren);
	Kaba::DeclareClassVirtualIndex("MidiRenderer", "read", Kaba::mf(&MidiRenderer::read), &midiren);
	Kaba::DeclareClassVirtualIndex("MidiRenderer", "reset", Kaba::mf(&MidiRenderer::reset), &midiren);
	Kaba::DeclareClassVirtualIndex("MidiRenderer", "getSampleRate", Kaba::mf(&MidiRenderer::getSampleRate), &midiren);
	Kaba::LinkExternal("MidiRenderer.setSynthesizer", Kaba::mf(&MidiRenderer::setSynthesizer));


	DummySynthesizer dsynth;
	Kaba::DeclareClassSize("DummySynthesizer", sizeof(DummySynthesizer));
	Kaba::LinkExternal("DummySynthesizer." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&DummySynthesizer::__init__));
	Kaba::DeclareClassVirtualIndex("DummySynthesizer", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&DummySynthesizer::__delete__), &dsynth);
	Kaba::DeclareClassVirtualIndex("DummySynthesizer", "render", Kaba::mf(&DummySynthesizer::render), &dsynth);
	Kaba::DeclareClassVirtualIndex("DummySynthesizer", "onConfig", Kaba::mf(&DummySynthesizer::onConfig), &dsynth);

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
	Kaba::DeclareClassOffset("BarPattern", "type", _offsetof(BarPattern, type));
	//Kaba::DeclareClassOffset("BarPattern", "count", _offsetof(BarPattern, count));

	Kaba::DeclareClassSize("MidiNote", sizeof(MidiNote));
	Kaba::DeclareClassOffset("MidiNote", "range", _offsetof(MidiNote, range));
	Kaba::DeclareClassOffset("MidiNote", "pitch", _offsetof(MidiNote, pitch));
	Kaba::DeclareClassOffset("MidiNote", "volume", _offsetof(MidiNote, volume));
	Kaba::DeclareClassOffset("MidiNote", "stringno", _offsetof(MidiNote, stringno));
	Kaba::DeclareClassOffset("MidiNote", "clef_position", _offsetof(MidiNote, clef_position));
	Kaba::DeclareClassOffset("MidiNote", "modifier", _offsetof(MidiNote, modifier));

	Kaba::DeclareClassSize("MidiRawData", sizeof(MidiRawData));
	Kaba::DeclareClassOffset("MidiRawData", "samples", _offsetof(MidiRawData, samples));
	Kaba::LinkExternal("MidiRawData." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&MidiRawData::__init__));
	Kaba::LinkExternal("MidiRawData.getEvents", Kaba::mf(&MidiRawData::getEvents));
	Kaba::LinkExternal("MidiRawData.getNotes", Kaba::mf(&MidiRawData::getNotes));
	Kaba::LinkExternal("MidiRawData.getRange", Kaba::mf(&MidiRawData::getRange));
	Kaba::LinkExternal("MidiRawData.addMetronomeClick", Kaba::mf(&MidiRawData::addMetronomeClick));

	Kaba::DeclareClassSize("MidiData", sizeof(MidiData));
	Kaba::DeclareClassOffset("MidiData", "samples", _offsetof(MidiData, samples));
	Kaba::LinkExternal("MidiData." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&MidiData::__init__));
	Kaba::LinkExternal("MidiData.getEvents", Kaba::mf(&MidiData::getEvents));
	Kaba::LinkExternal("MidiData.getNotes", Kaba::mf(&MidiData::getNotes));
	Kaba::LinkExternal("MidiData.getRange", Kaba::mf(&MidiData::getRange));

	Kaba::DeclareClassSize("TrackMarker", sizeof(TrackMarker));
	Kaba::DeclareClassOffset("TrackMarker", "text", _offsetof(TrackMarker, pos));
	Kaba::DeclareClassOffset("TrackMarker", "pos", _offsetof(TrackMarker, text));

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
	Kaba::LinkExternal("Track.moveMarker", Kaba::mf(&Track::moveMarker));

	Song af;
	Kaba::DeclareClassSize("Song", sizeof(Song));
	Kaba::DeclareClassOffset("Song", "filename", _offsetof(Song, filename));
	Kaba::DeclareClassOffset("Song", "tag", _offsetof(Song, tags));
	Kaba::DeclareClassOffset("Song", "sample_rate", _offsetof(Song, sample_rate));
	Kaba::DeclareClassOffset("Song", "volume", _offsetof(Song, volume));
	Kaba::DeclareClassOffset("Song", "fx", _offsetof(Song, fx));
	Kaba::DeclareClassOffset("Song", "tracks", _offsetof(Song, tracks));
	Kaba::DeclareClassOffset("Song", "samples", _offsetof(Song, samples));
	Kaba::DeclareClassOffset("Song", "layer_names", _offsetof(Song, layer_names));
	Kaba::DeclareClassOffset("Song", "bars", _offsetof(Song, bars));
	Kaba::LinkExternal("Song." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&Song::__init__));
	Kaba::DeclareClassVirtualIndex("Song", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&Song::__delete__), &af);
	Kaba::LinkExternal("Song.newEmpty", Kaba::mf(&Song::newEmpty));
	Kaba::LinkExternal("Song.addTrack", Kaba::mf(&Song::addTrack));
	Kaba::LinkExternal("Song.deleteTrack", Kaba::mf(&Song::deleteTrack));
	Kaba::LinkExternal("Song.getRange", Kaba::mf(&Song::getRange));
	Kaba::LinkExternal("Song.getNextBeat", Kaba::mf(&Song::getNextBeat));
	Kaba::LinkExternal("Song.addBar", Kaba::mf(&Song::addBar));
	Kaba::LinkExternal("Song.addPause", Kaba::mf(&Song::addPause));
	Kaba::LinkExternal("Song.editBar", Kaba::mf(&Song::editBar));
	Kaba::LinkExternal("Song.deleteBar", Kaba::mf(&Song::deleteBar));
	Kaba::LinkExternal("Song.addSample", Kaba::mf(&Song::addSample));
	Kaba::LinkExternal("Song.deleteSample", Kaba::mf(&Song::deleteSample));

	AudioRenderer ar;
	Kaba::DeclareClassSize("AudioRenderer", sizeof(AudioRenderer));
	//Kaba::DeclareClassOffset("AudioRenderer", "sample_rate", _offsetof(AudioRenderer, sample_rate));
	Kaba::LinkExternal("AudioRenderer." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&AudioRenderer::__init__));
	Kaba::DeclareClassVirtualIndex("AudioRenderer", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&AudioRenderer::__delete__), &ar);
	Kaba::DeclareClassVirtualIndex("AudioRenderer", "read", Kaba::mf(&AudioRenderer::read), &ar);
	Kaba::DeclareClassVirtualIndex("AudioRenderer", "reset", Kaba::mf(&AudioRenderer::reset), &ar);
	Kaba::DeclareClassVirtualIndex("AudioRenderer", "range", Kaba::mf(&AudioRenderer::range), &ar);
	Kaba::DeclareClassVirtualIndex("AudioRenderer", "getPos", Kaba::mf(&AudioRenderer::getPos), &ar);
	Kaba::DeclareClassVirtualIndex("AudioRenderer", "seek", Kaba::mf(&AudioRenderer::seek), &ar);
	Kaba::DeclareClassVirtualIndex("AudioRenderer", "getSampleRate", Kaba::mf(&AudioRenderer::getSampleRate), &ar);

	SongRenderer sr(&af, NULL);
	Kaba::DeclareClassSize("SongRenderer", sizeof(SongRenderer));
	Kaba::LinkExternal("SongRenderer.prepare", Kaba::mf(&SongRenderer::prepare));
	Kaba::LinkExternal("SongRenderer.render", Kaba::mf(&SongRenderer::render));
	Kaba::LinkExternal("SongRenderer." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&SongRenderer::__init__));
	Kaba::DeclareClassVirtualIndex("SongRenderer", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&SongRenderer::__delete__), &sr);
	Kaba::DeclareClassVirtualIndex("SongRenderer", "read", Kaba::mf(&SongRenderer::read), &sr);
	Kaba::DeclareClassVirtualIndex("SongRenderer", "reset", Kaba::mf(&SongRenderer::reset), &sr);
	Kaba::DeclareClassVirtualIndex("SongRenderer", "range", Kaba::mf(&SongRenderer::range), &sr);
	Kaba::DeclareClassVirtualIndex("SongRenderer", "getPos", Kaba::mf(&SongRenderer::getPos), &sr);
	Kaba::DeclareClassVirtualIndex("SongRenderer", "seek", Kaba::mf(&SongRenderer::seek), &sr);
	Kaba::DeclareClassVirtualIndex("SongRenderer", "getSampleRate", Kaba::mf(&SongRenderer::getSampleRate), &sr);

	{
	InputStreamAudio input(0);
	Kaba::DeclareClassSize("InputStreamAny", sizeof(InputStreamAny));
	Kaba::DeclareClassOffset("InputStreamAny", "sample_rate", _offsetof(InputStreamAny, sample_rate));
	Kaba::DeclareClassVirtualIndex("InputStreamAny", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&InputStreamAny::__delete__), &input);
	Kaba::DeclareClassVirtualIndex("InputStreamAny", "start", Kaba::mf(&InputStreamAny::start), &input);
	Kaba::DeclareClassVirtualIndex("InputStreamAny", "stop",	 Kaba::mf(&InputStreamAny::stop), &input);
	Kaba::DeclareClassVirtualIndex("InputStreamAny", "isCapturing", Kaba::mf(&InputStreamAny::isCapturing), &input);
	Kaba::DeclareClassVirtualIndex("InputStreamAny", "getSampleCount", Kaba::mf(&InputStreamAny::getSampleCount), &input);
	Kaba::DeclareClassVirtualIndex("InputStreamAny", "accumulate", Kaba::mf(&InputStreamAny::accumulate), &input);
	Kaba::DeclareClassVirtualIndex("InputStreamAny", "resetAccumulation", Kaba::mf(&InputStreamAny::resetAccumulation), &input);
	Kaba::LinkExternal("InputStreamAny.addObserver", Kaba::mf(&InputStreamAny::addWrappedObserver));
	Kaba::LinkExternal("InputStreamAny.removeObserver", Kaba::mf(&InputStreamAny::removeWrappedObserver));
	Kaba::DeclareClassVirtualIndex("InputStreamAny", "getSampleRate", Kaba::mf(&InputStreamAny::getSampleRate), &input);
	Kaba::DeclareClassVirtualIndex("InputStreamAny", "getSomeSamples", Kaba::mf(&InputStreamAny::getSomeSamples), &input);
	Kaba::DeclareClassVirtualIndex("InputStreamAny", "getState", Kaba::mf(&InputStreamAny::getState), &input);
	Kaba::DeclareClassVirtualIndex("InputStreamAny", "setBackupMode", Kaba::mf(&InputStreamAny::setBackupMode), &input);
	}

	{
	InputStreamAudio input(0);
	Kaba::DeclareClassSize("InputStreamAudio", sizeof(InputStreamAudio));
	Kaba::DeclareClassOffset("InputStreamAudio", "current_buffer", _offsetof(InputStreamAudio, current_buffer));
	Kaba::DeclareClassOffset("InputStreamAudio", "buffer", _offsetof(InputStreamAudio, buffer));
	Kaba::DeclareClassOffset("InputStreamAudio", "accumulating", _offsetof(InputStreamAudio, accumulating));
	Kaba::DeclareClassOffset("InputStreamAudio", "capturing", _offsetof(InputStreamAudio, capturing));
	Kaba::LinkExternal("InputStreamAudio." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&InputStreamAudio::__init__));
	Kaba::DeclareClassVirtualIndex("InputStreamAudio", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&InputStreamAudio::__delete__), &input);
	Kaba::DeclareClassVirtualIndex("InputStreamAudio", "start", Kaba::mf(&InputStreamAudio::start), &input);
	Kaba::DeclareClassVirtualIndex("InputStreamAudio", "stop",	 Kaba::mf(&InputStreamAudio::stop), &input);
	Kaba::DeclareClassVirtualIndex("InputStreamAudio", "isCapturing", Kaba::mf(&InputStreamAudio::isCapturing), &input);
	Kaba::DeclareClassVirtualIndex("InputStreamAudio", "getSampleCount", Kaba::mf(&InputStreamAudio::getSampleCount), &input);
	Kaba::DeclareClassVirtualIndex("InputStreamAudio", "accumulate", Kaba::mf(&InputStreamAudio::accumulate), &input);
	Kaba::DeclareClassVirtualIndex("InputStreamAudio", "resetAccumulation", Kaba::mf(&InputStreamAudio::resetAccumulation), &input);
	Kaba::LinkExternal("InputStreamAudio.addObserver", Kaba::mf(&InputStreamAudio::addWrappedObserver));
	Kaba::LinkExternal("InputStreamAudio.removeObserver", Kaba::mf(&InputStreamAudio::removeWrappedObserver));
	Kaba::DeclareClassVirtualIndex("InputStreamAudio", "getSampleRate", Kaba::mf(&InputStreamAudio::getSampleRate), &input);
	Kaba::DeclareClassVirtualIndex("InputStreamAudio", "getSomeSamples", Kaba::mf(&InputStreamAudio::getSomeSamples), &input);
	Kaba::DeclareClassVirtualIndex("InputStreamAudio", "getState", Kaba::mf(&InputStreamAudio::getState), &input);
	Kaba::DeclareClassVirtualIndex("InputStreamAudio", "setBackupMode", Kaba::mf(&InputStreamAudio::setBackupMode), &input);
	}

	{
	OutputStream stream(NULL);
	Kaba::DeclareClassSize("OutputStream", sizeof(OutputStream));
	Kaba::LinkExternal("OutputStream." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&OutputStream::__init__));
	Kaba::DeclareClassVirtualIndex("OutputStream", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&OutputStream::__delete__), &stream);
	//Kaba::LinkExternal("OutputStream.setSource", Kaba::mf(&AudioStream::setSource));
	Kaba::LinkExternal("OutputStream.play", Kaba::mf(&OutputStream::play));
	Kaba::LinkExternal("OutputStream.stop", Kaba::mf(&OutputStream::stop));
	Kaba::LinkExternal("OutputStream.pause", Kaba::mf(&OutputStream::pause));
	Kaba::LinkExternal("OutputStream.isPlaying", Kaba::mf(&OutputStream::isPlaying));
	Kaba::LinkExternal("OutputStream.getPos", Kaba::mf(&OutputStream::getPos));
	Kaba::LinkExternal("OutputStream.getSampleRate", Kaba::mf(&OutputStream::getSampleRate));
	Kaba::LinkExternal("OutputStream.getVolume", Kaba::mf(&OutputStream::getVolume));
	Kaba::LinkExternal("OutputStream.setVolume", Kaba::mf(&OutputStream::setVolume));
	Kaba::LinkExternal("OutputStream.setBufferSize", Kaba::mf(&OutputStream::setBufferSize));
	}

	Kaba::DeclareClassSize("AudioView", sizeof(AudioView));
	Kaba::DeclareClassOffset("AudioView", "sel", _offsetof(AudioView, sel));
	Kaba::DeclareClassOffset("AudioView", "sel_raw", _offsetof(AudioView, sel_raw));
	Kaba::DeclareClassOffset("AudioView", "stream", _offsetof(AudioView, stream));
	Kaba::DeclareClassOffset("AudioView", "renderer", _offsetof(AudioView, renderer));
	Kaba::DeclareClassOffset("AudioView", "input", _offsetof(AudioView, input));
	Kaba::LinkExternal("AudioView.addObserver", Kaba::mf(&AudioView::addWrappedObserver));
	Kaba::LinkExternal("AudioView.removeObserver", Kaba::mf(&AudioView::removeWrappedObserver));

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

	Kaba::LinkExternal("Log.error", Kaba::mf(&Log::error));
	Kaba::LinkExternal("Log.warn", Kaba::mf(&Log::warn));
	Kaba::LinkExternal("Log.info", Kaba::mf(&Log::info));

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
	Kaba::DeclareClassOffset("SongPlugin", "win", _offsetof(SongPlugin, win));
	Kaba::DeclareClassOffset("SongPlugin", "view", _offsetof(SongPlugin, view));
	Kaba::LinkExternal("SongPlugin." + Kaba::IDENTIFIER_FUNC_INIT, Kaba::mf(&SongPlugin::__init__));
	Kaba::DeclareClassVirtualIndex("SongPlugin", Kaba::IDENTIFIER_FUNC_DELETE, Kaba::mf(&SongPlugin::__delete__), &song_plugin);
	Kaba::DeclareClassVirtualIndex("SongPlugin", "apply", Kaba::mf(&SongPlugin::apply), &song_plugin);

	TsunamiPlugin tsunami_plugin;
	Kaba::DeclareClassSize("TsunamiPlugin", sizeof(TsunamiPlugin));
	Kaba::DeclareClassOffset("TsunamiPlugin", "win", _offsetof(TsunamiPlugin, win));
	Kaba::DeclareClassOffset("TsunamiPlugin", "view", _offsetof(TsunamiPlugin, view));
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
	SilentFiles = true;
	string content = FileRead(pf.filename);
	int p = content.find("// Image = hui:");
	if (p >= 0)
		pf.image = content.substr(p + 11, content.find("\n") - p - 11);
	SilentFiles = false;
}

void find_plugins_in_dir(const string &dir, int type, PluginManager *pm)
{
	Array<DirEntry> list = dir_search(tsunami->directory_static + "Plugins/" + dir, "*.kaba", false);
	for (DirEntry &e : list){
		PluginManager::PluginFile pf;
		pf.type = type;
		pf.name = e.name.replace(".kaba", "");
		pf.filename = tsunami->directory_static + "Plugins/" + dir + e.name;
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

	// "Buffer"
	find_plugins_in_dir("Buffer/Channels/", Plugin::TYPE_EFFECT, this);
	find_plugins_in_dir("Buffer/Dynamics/", Plugin::TYPE_EFFECT, this);
	find_plugins_in_dir("Buffer/Echo/", Plugin::TYPE_EFFECT, this);
	find_plugins_in_dir("Buffer/Pitch/", Plugin::TYPE_EFFECT, this);
	find_plugins_in_dir("Buffer/Repair/", Plugin::TYPE_EFFECT, this);
	find_plugins_in_dir("Buffer/Sound/", Plugin::TYPE_EFFECT, this);
	find_plugins_in_dir("Buffer/Synthesizer/", Plugin::TYPE_EFFECT, this);

	// "Midi"
	find_plugins_in_dir("Midi/", Plugin::TYPE_MIDI_EFFECT, this);

	// "All"
	find_plugins_in_dir("All/", Plugin::TYPE_SONG_PLUGIN, this);

	// rest
	find_plugins_in_dir("Independent/", Plugin::TYPE_TSUNAMI_PLUGIN, this);

	// "Synthesizer"
	find_plugins_in_dir("Synthesizer/", Plugin::TYPE_SYNTHESIZER, this);
}

void PluginManager::AddPluginsToMenu(TsunamiWindow *win)
{
	hui::Menu *m = win->getMenu();

	// "Buffer"
	add_plugins_in_dir("Buffer/Channels/", this, m->getSubMenuByID("menu_plugins_channels"), "effect", win, &TsunamiWindow::onMenuExecuteEffect);
	add_plugins_in_dir("Buffer/Dynamics/", this, m->getSubMenuByID("menu_plugins_dynamics"), "effect", win, &TsunamiWindow::onMenuExecuteEffect);
	add_plugins_in_dir("Buffer/Echo/", this, m->getSubMenuByID("menu_plugins_echo"), "effect", win, &TsunamiWindow::onMenuExecuteEffect);
	add_plugins_in_dir("Buffer/Pitch/", this, m->getSubMenuByID("menu_plugins_pitch"), "effect", win, &TsunamiWindow::onMenuExecuteEffect);
	add_plugins_in_dir("Buffer/Repair/", this, m->getSubMenuByID("menu_plugins_repair"), "effect", win, &TsunamiWindow::onMenuExecuteEffect);
	add_plugins_in_dir("Buffer/Sound/", this, m->getSubMenuByID("menu_plugins_sound"), "effect", win, &TsunamiWindow::onMenuExecuteEffect);
	add_plugins_in_dir("Buffer/Synthesizer/", this, m->getSubMenuByID("menu_plugins_synthesizer"), "effect", win, &TsunamiWindow::onMenuExecuteEffect);

	// "Midi"
	add_plugins_in_dir("Midi/", this, m->getSubMenuByID("menu_plugins_on_midi"), "midi-effect", win, &TsunamiWindow::onMenuExecuteMidiEffect);

	// "All"
	add_plugins_in_dir("All/", this, m->getSubMenuByID("menu_plugins_on_all"), "song", win, &TsunamiWindow::onMenuExecuteSongPlugin);

	// rest
	add_plugins_in_dir("Independent/", this, m->getSubMenuByID("menu_plugins_other"), "tsunami", win, &TsunamiWindow::onMenuExecuteTsunamiPlugin);
}

void PluginManager::ApplyFavorite(Configurable *c, const string &name)
{
	favorites->Apply(c, name);
}

void PluginManager::SaveFavorite(Configurable *c, const string &name)
{
	favorites->Save(c, name);
}


string PluginManager::SelectFavoriteName(hui::Window *win, Configurable *c, bool save)
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

void PluginManager::_ExecutePlugin(TsunamiWindow *win, const string &filename)
{
	Plugin *p = LoadAndCompilePlugin(Plugin::TYPE_OTHER, filename);
	if (!p->usable){
		tsunami->log->error(p->getError());
		return;
	}

	Kaba::Script *s = p->s;

	main_void_func *f_main = (main_void_func*)s->MatchFunction("main", "void", 0);
	if (f_main){
		f_main();
	}else{
		tsunami->log->error(_("Plugin does not contain a function 'void main()'"));
	}
}


Plugin *PluginManager::GetPlugin(int type, const string &name)
{
	for (PluginFile &pf: plugin_files){
		if ((pf.name == name) and (pf.type == type)){
			Plugin *p = LoadAndCompilePlugin(type, pf.filename);
			if (!p->usable)
				tsunami->log->error(p->getError());
			return p;
		}
	}
	tsunami->log->error(format(_("Can't find plugin: %s ..."), name.c_str()));
	return NULL;
}


Array<string> PluginManager::FindSynthesizers()
{
	Array<string> names;
	Array<DirEntry> list = dir_search(tsunami->directory_static + "Plugins/Synthesizer/", "*.kaba", false);
	for (DirEntry &e : list)
		names.add(e.name.replace(".kaba", ""));
	return names;
}

Synthesizer *PluginManager::LoadSynthesizer(const string &name, Song *song)
{
	string filename = tsunami->directory_static + "Plugins/Synthesizer/" + name + ".kaba";
	if (!file_test_existence(filename))
		return NULL;
	Kaba::Script *s;
	try{
		s = Kaba::Load(filename);
	}catch(Kaba::Exception &e){
		tsunami->log->error(e.message);
		return NULL;
	}
	for (auto *t : s->syntax->classes){
		if (t->GetRoot()->name != "Synthesizer")
			continue;
		Synthesizer *synth = (Synthesizer*)t->CreateInstance();
		synth->song = song;
		synth->setSampleRate(song->sample_rate);
		return synth;
	}
	return NULL;
}

Effect* PluginManager::ChooseEffect(hui::Panel *parent, Song *song)
{
	ConfigurableSelectorDialog *dlg = new ConfigurableSelectorDialog(parent->win, Configurable::TYPE_EFFECT, song);
	dlg->run();
	Effect *e = (Effect*)dlg->_return;
	delete(dlg);
	return e;
}

MidiEffect* PluginManager::ChooseMidiEffect(hui::Panel *parent, Song *song)
{
	ConfigurableSelectorDialog *dlg = new ConfigurableSelectorDialog(parent->win, Configurable::TYPE_MIDI_EFFECT, song);
	dlg->run();
	MidiEffect *e = (MidiEffect*)dlg->_return;
	delete(dlg);
	return e;
}

/*Synthesizer* PluginManager::ChooseSynthesizer(HuiPanel *parent)
{
	string name = ChooseConfigurable(parent, Configurable::TYPE_SYNTHESIZER);
	if (name == "")
		return NULL;
	return CreateSynthesizer(name);
}*/


