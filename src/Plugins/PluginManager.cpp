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
}


bool GlobalAllowTermination()
{
	return tsunami->allowTermination();
}

void PluginManager::LinkAppScriptData()
{
	msg_db_f("LinkAppScriptData", 2);
	Script::config.directory = "";

	// api definition
	Script::LinkExternal("device_manager", &tsunami->device_manager);
	Script::LinkExternal("storage", &tsunami->storage);
	Script::LinkExternal("logging", &tsunami->log);
	Script::LinkExternal("colors", &AudioView::colors);
	Script::LinkExternal("fft_c2c", (void*)&FastFourierTransform::fft_c2c);
	Script::LinkExternal("fft_r2c", (void*)&FastFourierTransform::fft_r2c);
	Script::LinkExternal("fft_c2r_inv", (void*)&FastFourierTransform::fft_c2r_inv);
	Script::LinkExternal("CreateSynthesizer", (void*)&CreateSynthesizer);
	Script::LinkExternal("AllowTermination", (void*)&GlobalAllowTermination);
	Script::LinkExternal("SelectSample", (void*)&SampleManagerConsole::select);

	Script::DeclareClassSize("Range", sizeof(Range));
	Script::DeclareClassOffset("Range", "offset", _offsetof(Range, offset));
	Script::DeclareClassOffset("Range", "length", _offsetof(Range, length));


	PluginData plugin_data;
	Script::DeclareClassSize("PluginData", sizeof(PluginData));
	Script::LinkExternal("PluginData." + Script::IDENTIFIER_FUNC_INIT, Script::mf(&PluginData::__init__));
	Script::DeclareClassVirtualIndex("PluginData", Script::IDENTIFIER_FUNC_DELETE, Script::mf(&PluginData::__delete__), &plugin_data);
	Script::DeclareClassVirtualIndex("PluginData", "reset", Script::mf(&PluginData::reset), &plugin_data);


	ConfigPanel config_panel;
	Script::DeclareClassSize("ConfigPanel", sizeof(ConfigPanel));
	Script::LinkExternal("ConfigPanel." + Script::IDENTIFIER_FUNC_INIT, Script::mf(&ConfigPanel::__init__));
	Script::DeclareClassVirtualIndex("ConfigPanel", Script::IDENTIFIER_FUNC_DELETE, Script::mf(&ConfigPanel::__delete__), &config_panel);
	Script::DeclareClassVirtualIndex("ConfigPanel", "update", Script::mf(&ConfigPanel::update), &config_panel);
	Script::LinkExternal("ConfigPanel.notify", Script::mf(&ConfigPanel::notify));
	Script::DeclareClassOffset("ConfigPanel", "c", _offsetof(ConfigPanel, c));


	Effect effect;
	Script::DeclareClassSize("AudioEffect", sizeof(Effect));
	Script::DeclareClassOffset("AudioEffect", "name", _offsetof(Effect, name));
	Script::DeclareClassOffset("AudioEffect", "only_on_selection", _offsetof(Effect, only_on_selection));
	Script::DeclareClassOffset("AudioEffect", "range", _offsetof(Effect, range));
	Script::DeclareClassOffset("AudioEffect", "usable", _offsetof(Effect, usable));
	Script::DeclareClassOffset("AudioEffect", "song", _offsetof(Effect, song));
	Script::DeclareClassOffset("AudioEffect", "track", _offsetof(Effect, track));
	Script::DeclareClassOffset("AudioEffect", "range", _offsetof(Effect, range));
	Script::DeclareClassOffset("AudioEffect", "level", _offsetof(Effect, level));
	Script::LinkExternal("AudioEffect." + Script::IDENTIFIER_FUNC_INIT, Script::mf(&Effect::__init__));
	Script::DeclareClassVirtualIndex("AudioEffect", Script::IDENTIFIER_FUNC_DELETE, Script::mf(&Effect::__delete__), &effect);
	Script::DeclareClassVirtualIndex("AudioEffect", "process", Script::mf(&Effect::processTrack), &effect);
	Script::DeclareClassVirtualIndex("AudioEffect", "createPanel", Script::mf(&Effect::createPanel), &effect);
	Script::LinkExternal("AudioEffect.resetConfig", Script::mf(&Effect::resetConfig));
	Script::LinkExternal("AudioEffect.resetState", Script::mf(&Effect::resetState));
	//Script::DeclareClassVirtualIndex("AudioEffect", "updateDialog", Script::mf(&Effect::UpdateDialog), &effect);
	Script::LinkExternal("AudioEffect.notify", Script::mf(&Effect::notify));
	Script::DeclareClassVirtualIndex("AudioEffect", "onConfig", Script::mf(&Effect::onConfig), &effect);

	MidiEffect midieffect;
	Script::DeclareClassSize("MidiEffect", sizeof(MidiEffect));
	Script::DeclareClassOffset("MidiEffect", "name", _offsetof(MidiEffect, name));
	Script::DeclareClassOffset("MidiEffect", "only_on_selection", _offsetof(MidiEffect, only_on_selection));
	Script::DeclareClassOffset("MidiEffect", "range", _offsetof(MidiEffect, range));
	Script::DeclareClassOffset("MidiEffect", "usable", _offsetof(MidiEffect, usable));
	Script::DeclareClassOffset("MidiEffect", "song", _offsetof(MidiEffect, song));
	Script::LinkExternal("MidiEffect." + Script::IDENTIFIER_FUNC_INIT, Script::mf(&MidiEffect::__init__));
	Script::DeclareClassVirtualIndex("MidiEffect", Script::IDENTIFIER_FUNC_DELETE, Script::mf(&MidiEffect::__delete__), &midieffect);
	Script::DeclareClassVirtualIndex("MidiEffect", "process", Script::mf(&MidiEffect::process), &midieffect);
	Script::DeclareClassVirtualIndex("MidiEffect", "createPanel", Script::mf(&MidiEffect::createPanel), &midieffect);
	Script::LinkExternal("MidiEffect.resetConfig", Script::mf(&MidiEffect::resetConfig));
	Script::LinkExternal("MidiEffect.resetState", Script::mf(&MidiEffect::resetState));
	//Script::DeclareClassVirtualIndex("MidiEffect", "updateDialog", Script::mf(&MidiEffect::UpdateDialog), &midieffect);
	Script::LinkExternal("MidiEffect.notify", Script::mf(&MidiEffect::notify));
	Script::DeclareClassVirtualIndex("MidiEffect", "onConfig", Script::mf(&MidiEffect::onConfig), &midieffect);

	Script::DeclareClassSize("BufferBox", sizeof(BufferBox));
	Script::DeclareClassOffset("BufferBox", "offset", _offsetof(BufferBox, offset));
	Script::DeclareClassOffset("BufferBox", "length", _offsetof(BufferBox, length));
	Script::DeclareClassOffset("BufferBox", "channels", _offsetof(BufferBox, channels));
	Script::DeclareClassOffset("BufferBox", "r", _offsetof(BufferBox, c[0]));
	Script::DeclareClassOffset("BufferBox", "l", _offsetof(BufferBox, c[1]));
	Script::DeclareClassOffset("BufferBox", "peaks", _offsetof(BufferBox, peaks));
	Script::LinkExternal("BufferBox." + Script::IDENTIFIER_FUNC_INIT, Script::mf(&BufferBox::__init__));
	Script::LinkExternal("BufferBox." + Script::IDENTIFIER_FUNC_DELETE, Script::mf(&BufferBox::__delete__));
	Script::LinkExternal("BufferBox.clear", Script::mf(&BufferBox::clear));
	Script::LinkExternal("BufferBox." + Script::IDENTIFIER_FUNC_ASSIGN, Script::mf(&BufferBox::__assign__));
	Script::LinkExternal("BufferBox.range", Script::mf(&BufferBox::range));
	Script::LinkExternal("BufferBox.add", Script::mf(&BufferBox::add));
	Script::LinkExternal("BufferBox.set", Script::mf(&BufferBox::set));
	Script::LinkExternal("BufferBox.get_spectrum", Script::mf(&ExtendedBufferBox::get_spectrum));


	Script::DeclareClassSize("RingBuffer", sizeof(RingBuffer));
	//Script::LinkExternal("RingBuffer." + Script::IDENTIFIER_FUNC_INIT, Script::mf(&RingBuffer::__init__));
	Script::LinkExternal("RingBuffer.available", Script::mf(&RingBuffer::available));
	Script::LinkExternal("RingBuffer.read", Script::mf(&RingBuffer::read));
	Script::LinkExternal("RingBuffer.write", Script::mf(&RingBuffer::write));
	Script::LinkExternal("RingBuffer.readRef", Script::mf(&RingBuffer::readRef));
	Script::LinkExternal("RingBuffer.peekRef", Script::mf(&RingBuffer::peekRef));
	Script::LinkExternal("RingBuffer.writeRef", Script::mf(&RingBuffer::writeRef));
	Script::LinkExternal("RingBuffer.moveReadPos", Script::mf(&RingBuffer::moveReadPos));
	Script::LinkExternal("RingBuffer.moveWritePos", Script::mf(&RingBuffer::moveWritePos));
	Script::LinkExternal("RingBuffer.clear", Script::mf(&RingBuffer::clear));

	Script::DeclareClassSize("Sample", sizeof(Sample));
	Script::DeclareClassOffset("Sample", "name", _offsetof(Sample, name));
	Script::DeclareClassOffset("Sample", "type", _offsetof(Sample, type));
	Script::DeclareClassOffset("Sample", "buf", _offsetof(Sample, buf));
	Script::DeclareClassOffset("Sample", "midi", _offsetof(Sample, midi));
	Script::DeclareClassOffset("Sample", "volume", _offsetof(Sample, volume));
	Script::DeclareClassOffset("Sample", "uid", _offsetof(Sample, uid));
	Script::LinkExternal("Sample.createRef", Script::mf(&Sample::create_ref));

	Sample sample(0);
	sample.owner = tsunami->song;
	SampleRef sampleref(&sample);
	Script::DeclareClassSize("SampleRef", sizeof(SampleRef));
	Script::DeclareClassOffset("SampleRef", "buf", _offsetof(SampleRef, buf));
	Script::DeclareClassOffset("SampleRef", "midi", _offsetof(SampleRef, midi));
	Script::DeclareClassOffset("SampleRef", "origin", _offsetof(SampleRef, origin));
	Script::LinkExternal("SampleRef." + Script::IDENTIFIER_FUNC_INIT, Script::mf(&SampleRef::__init__));
	Script::DeclareClassVirtualIndex("SampleRef", Script::IDENTIFIER_FUNC_DELETE, Script::mf(&SampleRef::__delete__), &sampleref);

	MidiSource midi_source;
	Script::DeclareClassSize("MidiSource", sizeof(MidiSource));
	Script::LinkExternal("MidiSource." + Script::IDENTIFIER_FUNC_INIT, Script::mf(&MidiSource::__init__));
	Script::DeclareClassVirtualIndex("MidiSource", Script::IDENTIFIER_FUNC_DELETE, Script::mf(&MidiSource::__delete__), &midi_source);
	Script::DeclareClassVirtualIndex("MidiSource", "read", Script::mf(&MidiSource::read), &midi_source);

	Synthesizer synth;
	Script::DeclareClassSize("Synthesizer", sizeof(Synthesizer));
	Script::DeclareClassOffset("Synthesizer", "name", _offsetof(Synthesizer, name));
	Script::DeclareClassOffset("Synthesizer", "sample_rate", _offsetof(Synthesizer, sample_rate));
	Script::DeclareClassOffset("Synthesizer", "events", _offsetof(Synthesizer, events));
	Script::DeclareClassOffset("Synthesizer", "keep_notes", _offsetof(Synthesizer, keep_notes));
	Script::DeclareClassOffset("Synthesizer", "active_pitch", _offsetof(Synthesizer, active_pitch));
	Script::DeclareClassOffset("Synthesizer", "freq", _offsetof(Synthesizer, tuning.freq));
	Script::DeclareClassOffset("Synthesizer", "delta_phi", _offsetof(Synthesizer, delta_phi));
	Script::LinkExternal("Synthesizer." + Script::IDENTIFIER_FUNC_INIT, Script::mf(&Synthesizer::__init__));
	Script::DeclareClassVirtualIndex("Synthesizer", Script::IDENTIFIER_FUNC_DELETE, Script::mf(&Synthesizer::__delete__), &synth);
	Script::DeclareClassVirtualIndex("Synthesizer", "createPanel", Script::mf(&Synthesizer::createPanel), &synth);
	Script::LinkExternal("Synthesizer.resetConfig", Script::mf(&Synthesizer::resetConfig));
	Script::LinkExternal("Synthesizer.resetState", Script::mf(&Synthesizer::resetState));
	Script::LinkExternal("Synthesizer.enablePitch", Script::mf(&Synthesizer::enablePitch));
	Script::DeclareClassVirtualIndex("Synthesizer", "render", Script::mf(&Synthesizer::render), &synth);
	Script::DeclareClassVirtualIndex("Synthesizer", "onConfig", Script::mf(&Synthesizer::onConfig), &synth);
	Script::LinkExternal("Synthesizer.setSampleRate", Script::mf(&Synthesizer::setSampleRate));
	Script::LinkExternal("Synthesizer.notify", Script::mf(&Synthesizer::notify));

	MidiRenderer midiren(NULL, NULL);
	Script::DeclareClassSize("MidiRenderer", sizeof(MidiRenderer));
	Script::LinkExternal("MidiRenderer." + Script::IDENTIFIER_FUNC_INIT, Script::mf(&MidiRenderer::__init__));
	Script::DeclareClassVirtualIndex("MidiRenderer", Script::IDENTIFIER_FUNC_DELETE, Script::mf(&MidiRenderer::__delete__), &midiren);
	Script::DeclareClassVirtualIndex("MidiRenderer", "read", Script::mf(&MidiRenderer::read), &midiren);
	Script::DeclareClassVirtualIndex("MidiRenderer", "reset", Script::mf(&MidiRenderer::reset), &midiren);
	Script::DeclareClassVirtualIndex("MidiRenderer", "getSampleRate", Script::mf(&MidiRenderer::getSampleRate), &midiren);
	Script::LinkExternal("MidiRenderer.setSynthesizer", Script::mf(&MidiRenderer::setSynthesizer));


	DummySynthesizer dsynth;
	Script::DeclareClassSize("DummySynthesizer", sizeof(DummySynthesizer));
	Script::LinkExternal("DummySynthesizer." + Script::IDENTIFIER_FUNC_INIT, Script::mf(&DummySynthesizer::__init__));
	Script::DeclareClassVirtualIndex("DummySynthesizer", Script::IDENTIFIER_FUNC_DELETE, Script::mf(&DummySynthesizer::__delete__), &dsynth);
	Script::DeclareClassVirtualIndex("DummySynthesizer", "render", Script::mf(&DummySynthesizer::render), &dsynth);
	Script::DeclareClassVirtualIndex("DummySynthesizer", "onConfig", Script::mf(&DummySynthesizer::onConfig), &dsynth);

	Script::DeclareClassSize("EnvelopeADSR", sizeof(EnvelopeADSR));
	Script::LinkExternal("EnvelopeADSR.set", Script::mf(&EnvelopeADSR::set));
	Script::LinkExternal("EnvelopeADSR.set2", Script::mf(&EnvelopeADSR::set2));
	Script::LinkExternal("EnvelopeADSR.reset", Script::mf(&EnvelopeADSR::reset));
	Script::LinkExternal("EnvelopeADSR.start", Script::mf(&EnvelopeADSR::start));
	Script::LinkExternal("EnvelopeADSR.end", Script::mf(&EnvelopeADSR::end));
	Script::LinkExternal("EnvelopeADSR.get", Script::mf(&EnvelopeADSR::get));
	Script::DeclareClassOffset("EnvelopeADSR", "just_killed", _offsetof(EnvelopeADSR, just_killed));

	Script::DeclareClassSize("BarPattern", sizeof(BarPattern));
	Script::DeclareClassOffset("BarPattern", "num_beats", _offsetof(BarPattern, num_beats));
	Script::DeclareClassOffset("BarPattern", "length", _offsetof(BarPattern, length));
	Script::DeclareClassOffset("BarPattern", "type", _offsetof(BarPattern, type));
	//Script::DeclareClassOffset("BarPattern", "count", _offsetof(BarPattern, count));
	Script::DeclareClassOffset("BarPattern", "is_selected", _offsetof(BarPattern, is_selected));

	Script::DeclareClassSize("MidiNote", sizeof(MidiNote));
	Script::DeclareClassOffset("MidiNote", "range", _offsetof(MidiNote, range));
	Script::DeclareClassOffset("MidiNote", "pitch", _offsetof(MidiNote, pitch));
	Script::DeclareClassOffset("MidiNote", "volume", _offsetof(MidiNote, volume));
	Script::DeclareClassOffset("MidiNote", "stringno", _offsetof(MidiNote, stringno));
	Script::DeclareClassOffset("MidiNote", "clef_position", _offsetof(MidiNote, clef_position));
	Script::DeclareClassOffset("MidiNote", "modifier", _offsetof(MidiNote, modifier));

	Script::DeclareClassSize("MidiRawData", sizeof(MidiRawData));
	Script::DeclareClassOffset("MidiRawData", "samples", _offsetof(MidiRawData, samples));
	Script::LinkExternal("MidiRawData." + Script::IDENTIFIER_FUNC_INIT, Script::mf(&MidiRawData::__init__));
	Script::LinkExternal("MidiRawData.getEvents", Script::mf(&MidiRawData::getEvents));
	Script::LinkExternal("MidiRawData.getNotes", Script::mf(&MidiRawData::getNotes));
	Script::LinkExternal("MidiRawData.getRange", Script::mf(&MidiRawData::getRange));
	Script::LinkExternal("MidiRawData.addMetronomeClick", Script::mf(&MidiRawData::addMetronomeClick));

	Script::DeclareClassSize("MidiData", sizeof(MidiData));
	Script::DeclareClassOffset("MidiData", "samples", _offsetof(MidiData, samples));
	Script::LinkExternal("MidiData." + Script::IDENTIFIER_FUNC_INIT, Script::mf(&MidiData::__init__));
	Script::LinkExternal("MidiData.getEvents", Script::mf(&MidiData::getEvents));
	Script::LinkExternal("MidiData.getNotes", Script::mf(&MidiData::getNotes));
	Script::LinkExternal("MidiData.getRange", Script::mf(&MidiData::getRange));

	Script::DeclareClassSize("TrackMarker", sizeof(TrackMarker));
	Script::DeclareClassOffset("TrackMarker", "text", _offsetof(TrackMarker, pos));
	Script::DeclareClassOffset("TrackMarker", "pos", _offsetof(TrackMarker, text));

	Script::DeclareClassSize("TrackLevel", sizeof(TrackLevel));
	Script::DeclareClassOffset("TrackLevel", "buffers", _offsetof(TrackLevel, buffers));

	Script::DeclareClassSize("Track", sizeof(Track));
	Script::DeclareClassOffset("Track", "type", _offsetof(Track, type));
	Script::DeclareClassOffset("Track", "name", _offsetof(Track, name));
	Script::DeclareClassOffset("Track", "levels", _offsetof(Track, levels));
	Script::DeclareClassOffset("Track", "volume", _offsetof(Track, volume));
	Script::DeclareClassOffset("Track", "panning", _offsetof(Track, panning));
	Script::DeclareClassOffset("Track", "muted", _offsetof(Track, muted));
	Script::DeclareClassOffset("Track", "fx", _offsetof(Track, fx));
	Script::DeclareClassOffset("Track", "midi", _offsetof(Track, midi));
	Script::DeclareClassOffset("Track", "synth", _offsetof(Track, synth));
	Script::DeclareClassOffset("Track", "samples", _offsetof(Track, samples));
	Script::DeclareClassOffset("Track", "markers", _offsetof(Track, markers));
	Script::DeclareClassOffset("Track", "root", _offsetof(Track, song));
	//Script::DeclareClassOffset("Track", "is_selected", _offsetof(Track, is_selected));
	Script::LinkExternal("Track.getBuffers", Script::mf(&Track::getBuffers));
	Script::LinkExternal("Track.readBuffers", Script::mf(&Track::readBuffers));
	Script::LinkExternal("Track.setName", Script::mf(&Track::setName));
	Script::LinkExternal("Track.setMuted", Script::mf(&Track::setMuted));
	Script::LinkExternal("Track.setVolume", Script::mf(&Track::setVolume));
	Script::LinkExternal("Track.setPanning", Script::mf(&Track::setPanning));
	Script::LinkExternal("Track.insertMidiData", Script::mf(&Track::insertMidiData));
	Script::LinkExternal("Track.addEffect", Script::mf(&Track::addEffect));
	Script::LinkExternal("Track.deleteEffect", Script::mf(&Track::deleteEffect));
	Script::LinkExternal("Track.editEffect", Script::mf(&Track::editEffect));
	Script::LinkExternal("Track.enableEffect", Script::mf(&Track::enableEffect));
	Script::LinkExternal("Track.addSample", Script::mf(&Track::addSample));
	Script::LinkExternal("Track.deleteSample", Script::mf(&Track::deleteSample));
	Script::LinkExternal("Track.editSample", Script::mf(&Track::editSample));
	Script::LinkExternal("Track.addMidiNote", Script::mf(&Track::addMidiNote));
	Script::LinkExternal("Track.addMidiNotes", Script::mf(&Track::addMidiNotes));
	Script::LinkExternal("Track.deleteMidiNote", Script::mf(&Track::deleteMidiNote));
	Script::LinkExternal("Track.setSynthesizer", Script::mf(&Track::setSynthesizer));
	Script::LinkExternal("Track.addMarker", Script::mf(&Track::addMarker));
	Script::LinkExternal("Track.deleteMarker", Script::mf(&Track::deleteMarker));
	Script::LinkExternal("Track.moveMarker", Script::mf(&Track::moveMarker));

	Song af;
	Script::DeclareClassSize("Song", sizeof(Song));
	Script::DeclareClassOffset("Song", "filename", _offsetof(Song, filename));
	Script::DeclareClassOffset("Song", "tag", _offsetof(Song, tags));
	Script::DeclareClassOffset("Song", "sample_rate", _offsetof(Song, sample_rate));
	Script::DeclareClassOffset("Song", "volume", _offsetof(Song, volume));
	Script::DeclareClassOffset("Song", "fx", _offsetof(Song, fx));
	Script::DeclareClassOffset("Song", "tracks", _offsetof(Song, tracks));
	Script::DeclareClassOffset("Song", "samples", _offsetof(Song, samples));
	Script::DeclareClassOffset("Song", "level_names", _offsetof(Song, level_names));
	Script::DeclareClassOffset("Song", "bars", _offsetof(Song, bars));
	Script::LinkExternal("Song." + Script::IDENTIFIER_FUNC_INIT, Script::mf(&Song::__init__));
	Script::DeclareClassVirtualIndex("Song", Script::IDENTIFIER_FUNC_DELETE, Script::mf(&Song::__delete__), &af);
	Script::LinkExternal("Song.newEmpty", Script::mf(&Song::newEmpty));
	Script::LinkExternal("Song.addTrack", Script::mf(&Song::addTrack));
	Script::LinkExternal("Song.deleteTrack", Script::mf(&Song::deleteTrack));
	Script::LinkExternal("Song.getRange", Script::mf(&Song::getRange));
	Script::LinkExternal("Song.getNextBeat", Script::mf(&Song::getNextBeat));
	Script::LinkExternal("Song.addBar", Script::mf(&Song::addBar));
	Script::LinkExternal("Song.addPause", Script::mf(&Song::addPause));
	Script::LinkExternal("Song.editBar", Script::mf(&Song::editBar));
	Script::LinkExternal("Song.deleteBar", Script::mf(&Song::deleteBar));

	AudioRenderer ar;
	Script::DeclareClassSize("AudioRenderer", sizeof(AudioRenderer));
	//Script::DeclareClassOffset("AudioRenderer", "sample_rate", _offsetof(AudioRenderer, sample_rate));
	Script::LinkExternal("AudioRenderer." + Script::IDENTIFIER_FUNC_INIT, Script::mf(&AudioRenderer::__init__));
	Script::DeclareClassVirtualIndex("AudioRenderer", Script::IDENTIFIER_FUNC_DELETE, Script::mf(&AudioRenderer::__delete__), &ar);
	Script::DeclareClassVirtualIndex("AudioRenderer", "read", Script::mf(&AudioRenderer::read), &ar);
	Script::DeclareClassVirtualIndex("AudioRenderer", "reset", Script::mf(&AudioRenderer::reset), &ar);
	Script::DeclareClassVirtualIndex("AudioRenderer", "range", Script::mf(&AudioRenderer::range), &ar);
	Script::DeclareClassVirtualIndex("AudioRenderer", "getPos", Script::mf(&AudioRenderer::getPos), &ar);
	Script::DeclareClassVirtualIndex("AudioRenderer", "seek", Script::mf(&AudioRenderer::seek), &ar);
	Script::DeclareClassVirtualIndex("AudioRenderer", "getSampleRate", Script::mf(&AudioRenderer::getSampleRate), &ar);

	SongRenderer sr(&af, NULL);
	Script::DeclareClassSize("SongRenderer", sizeof(SongRenderer));
	Script::LinkExternal("SongRenderer.prepare", Script::mf(&SongRenderer::prepare));
	Script::LinkExternal("SongRenderer.render", Script::mf(&SongRenderer::render));
	Script::LinkExternal("SongRenderer." + Script::IDENTIFIER_FUNC_INIT, Script::mf(&SongRenderer::__init__));
	Script::DeclareClassVirtualIndex("SongRenderer", Script::IDENTIFIER_FUNC_DELETE, Script::mf(&SongRenderer::__delete__), &sr);
	Script::DeclareClassVirtualIndex("SongRenderer", "read", Script::mf(&SongRenderer::read), &sr);
	Script::DeclareClassVirtualIndex("SongRenderer", "reset", Script::mf(&SongRenderer::reset), &sr);
	Script::DeclareClassVirtualIndex("SongRenderer", "range", Script::mf(&SongRenderer::range), &sr);
	Script::DeclareClassVirtualIndex("SongRenderer", "getPos", Script::mf(&SongRenderer::getPos), &sr);
	Script::DeclareClassVirtualIndex("SongRenderer", "seek", Script::mf(&SongRenderer::seek), &sr);
	Script::DeclareClassVirtualIndex("SongRenderer", "getSampleRate", Script::mf(&SongRenderer::getSampleRate), &sr);

	{
	InputStreamAudio input(0);
	Script::DeclareClassSize("InputStreamAudio", sizeof(InputStreamAudio));
	Script::DeclareClassOffset("InputStreamAudio", "current_buffer", _offsetof(InputStreamAudio, current_buffer));
	Script::DeclareClassOffset("InputStreamAudio", "buffer", _offsetof(InputStreamAudio, buffer));
	Script::DeclareClassOffset("InputStreamAudio", "sample_rate", _offsetof(InputStreamAudio, sample_rate));
	Script::DeclareClassOffset("InputStreamAudio", "accumulating", _offsetof(InputStreamAudio, accumulating));
	Script::DeclareClassOffset("InputStreamAudio", "capturing", _offsetof(InputStreamAudio, capturing));
	Script::LinkExternal("InputStreamAudio." + Script::IDENTIFIER_FUNC_INIT, Script::mf(&InputStreamAudio::__init__));
	Script::DeclareClassVirtualIndex("InputStreamAudio", Script::IDENTIFIER_FUNC_DELETE, Script::mf(&InputStreamAudio::__delete__), &input);
	Script::LinkExternal("InputStreamAudio.start", Script::mf(&InputStreamAudio::start));
	//Script::LinkExternal("InputStreamAudio.resetSync", Script::mf(&AudioInputAudio::resetSync));
	Script::LinkExternal("InputStreamAudio.stop",	 Script::mf(&InputStreamAudio::stop));
	Script::LinkExternal("InputStreamAudio.isCapturing", Script::mf(&InputStreamAudio::isCapturing));
	Script::LinkExternal("InputStreamAudio.getSampleCount", Script::mf(&InputStreamAudio::getSampleCount));
	Script::LinkExternal("InputStreamAudio.accumulate", Script::mf(&InputStreamAudio::accumulate));
	Script::LinkExternal("InputStreamAudio.resetAccumulation", Script::mf(&InputStreamAudio::resetAccumulation));
	//Script::LinkExternal("InputStreamAudio.getDelay", Script::mf(&AudioInputAudio::getDelay));
	Script::LinkExternal("InputStreamAudio.addObserver", Script::mf(&InputStreamAudio::addWrappedObserver));
	Script::LinkExternal("InputStreamAudio.removeObserver", Script::mf(&InputStreamAudio::removeWrappedObserver));
	//Script::LinkExternal("Observable.addObserver", Script::mf(&Observable::AddWrappedObserver);
	Script::DeclareClassVirtualIndex("InputStreamAudio", "getSampleRate", Script::mf(&InputStreamAudio::getSampleRate), &input);
	Script::DeclareClassVirtualIndex("InputStreamAudio", "getSomeSamples", Script::mf(&InputStreamAudio::getSomeSamples), &input);
	Script::DeclareClassVirtualIndex("InputStreamAudio", "getState", Script::mf(&InputStreamAudio::getState), &input);
	}

	{
	OutputStream stream(NULL);
	Script::DeclareClassSize("OutputStream", sizeof(OutputStream));
	Script::LinkExternal("OutputStream." + Script::IDENTIFIER_FUNC_INIT, Script::mf(&OutputStream::__init__));
	Script::DeclareClassVirtualIndex("OutputStream", Script::IDENTIFIER_FUNC_DELETE, Script::mf(&OutputStream::__delete__), &stream);
	//Script::LinkExternal("OutputStream.setSource", Script::mf(&AudioStream::setSource));
	Script::LinkExternal("OutputStream.play", Script::mf(&OutputStream::play));
	Script::LinkExternal("OutputStream.stop", Script::mf(&OutputStream::stop));
	Script::LinkExternal("OutputStream.pause", Script::mf(&OutputStream::pause));
	Script::LinkExternal("OutputStream.isPlaying", Script::mf(&OutputStream::isPlaying));
	Script::LinkExternal("OutputStream.getPos", Script::mf(&OutputStream::getPos));
	Script::LinkExternal("OutputStream.getSampleRate", Script::mf(&OutputStream::getSampleRate));
	Script::LinkExternal("OutputStream.getVolume", Script::mf(&OutputStream::getVolume));
	Script::LinkExternal("OutputStream.setVolume", Script::mf(&OutputStream::setVolume));
	Script::LinkExternal("OutputStream.setBufferSize", Script::mf(&OutputStream::setBufferSize));
	}

	Script::DeclareClassSize("AudioView", sizeof(AudioView));
	Script::DeclareClassOffset("AudioView", "sel", _offsetof(AudioView, sel));
	Script::DeclareClassOffset("AudioView", "sel_raw", _offsetof(AudioView, sel_raw));
	Script::DeclareClassOffset("AudioView", "stream", _offsetof(AudioView, stream));
	Script::DeclareClassOffset("AudioView", "renderer", _offsetof(AudioView, renderer));

	Script::DeclareClassSize("ColorScheme", sizeof(ColorScheme));
	Script::DeclareClassOffset("ColorScheme", "background", _offsetof(ColorScheme, background));
	Script::DeclareClassOffset("ColorScheme", "background_track", _offsetof(ColorScheme, background_track));
	Script::DeclareClassOffset("ColorScheme", "background_track_selected", _offsetof(ColorScheme, background_track_selected));
	Script::DeclareClassOffset("ColorScheme", "text", _offsetof(ColorScheme, text));
	Script::DeclareClassOffset("ColorScheme", "text_soft1", _offsetof(ColorScheme, text_soft1));
	Script::DeclareClassOffset("ColorScheme", "text_soft2", _offsetof(ColorScheme, text_soft2));
	Script::DeclareClassOffset("ColorScheme", "text_soft3", _offsetof(ColorScheme, text_soft3));
	Script::DeclareClassOffset("ColorScheme", "grid", _offsetof(ColorScheme, grid));
	Script::DeclareClassOffset("ColorScheme", "selection", _offsetof(ColorScheme, selection));
	Script::DeclareClassOffset("ColorScheme", "hover", _offsetof(ColorScheme, hover));

	Script::LinkExternal("Log.error", Script::mf(&Log::error));
	Script::LinkExternal("Log.warn", Script::mf(&Log::warn));
	Script::LinkExternal("Log.info", Script::mf(&Log::info));

	Script::LinkExternal("Storage.load", Script::mf(&Storage::load));
	Script::LinkExternal("Storage.save", Script::mf(&Storage::save));
	Script::DeclareClassOffset("Storage", "current_directory", _offsetof(Storage, current_directory));


	Slider slider;
	Script::DeclareClassSize("Slider", sizeof(Slider));
	Script::LinkExternal("Slider." + Script::IDENTIFIER_FUNC_INIT, Script::mf(&Slider::__init_ext__));
	Script::DeclareClassVirtualIndex("Slider", Script::IDENTIFIER_FUNC_DELETE, Script::mf(&Slider::__delete__), &slider);
	Script::LinkExternal("Slider.get", Script::mf(&Slider::get));
	Script::LinkExternal("Slider.set", Script::mf(&Slider::set));

	SongPlugin song_plugin;
	Script::DeclareClassSize("SongPlugin", sizeof(SongPlugin));
	Script::DeclareClassOffset("SongPlugin", "win", _offsetof(SongPlugin, win));
	Script::DeclareClassOffset("SongPlugin", "view", _offsetof(SongPlugin, view));
	Script::LinkExternal("SongPlugin." + Script::IDENTIFIER_FUNC_INIT, Script::mf(&SongPlugin::__init__));
	Script::DeclareClassVirtualIndex("SongPlugin", Script::IDENTIFIER_FUNC_DELETE, Script::mf(&SongPlugin::__delete__), &song_plugin);
	Script::DeclareClassVirtualIndex("SongPlugin", "apply", Script::mf(&SongPlugin::apply), &song_plugin);

	TsunamiPlugin tsunami_plugin;
	Script::DeclareClassSize("TsunamiPlugin", sizeof(TsunamiPlugin));
	Script::DeclareClassOffset("TsunamiPlugin", "win", _offsetof(TsunamiPlugin, win));
	Script::DeclareClassOffset("TsunamiPlugin", "view", _offsetof(TsunamiPlugin, view));
	Script::LinkExternal("TsunamiPlugin." + Script::IDENTIFIER_FUNC_INIT, Script::mf(&TsunamiPlugin::__init__));
	Script::DeclareClassVirtualIndex("TsunamiPlugin", Script::IDENTIFIER_FUNC_DELETE, Script::mf(&TsunamiPlugin::__delete__), &tsunami_plugin);
	Script::DeclareClassVirtualIndex("TsunamiPlugin", "onStart", Script::mf(&TsunamiPlugin::onStart), &tsunami_plugin);
	Script::DeclareClassVirtualIndex("TsunamiPlugin", "onStop", Script::mf(&TsunamiPlugin::onStop), &tsunami_plugin);
	Script::LinkExternal("TsunamiPlugin.stop", Script::mf(&TsunamiPlugin::stop_request));

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
	Array<DirEntry> list = dir_search(HuiAppDirectoryStatic + "Plugins/" + dir, "*.kaba", false);
	for (DirEntry &e : list){
		PluginManager::PluginFile pf;
		pf.type = type;
		pf.name = e.name.replace(".kaba", "");
		pf.filename = HuiAppDirectoryStatic + "Plugins/" + dir + e.name;
		get_plugin_file_data(pf);
		pm->plugin_files.add(pf);
	}
}

void add_plugins_in_dir(const string &dir, PluginManager *pm, HuiMenu *m, const string &name_space, TsunamiWindow *win, void (TsunamiWindow::*function)())
{
	foreachi(PluginManager::PluginFile &f, pm->plugin_files, i){
		if (f.filename.find(dir) >= 0){
			string id = "execute-" + name_space + "--" + f.name;
            m->addItemImage(f.name, f.image, id);
            win->event(id, win, function);
		}
	}
}

void PluginManager::FindPlugins()
{
	msg_db_f("FindPlugins", 2);
	Script::Init();

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
	msg_db_f("AddPluginsToMenu", 2);

	HuiMenu *m = win->getMenu();

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


string PluginManager::SelectFavoriteName(HuiWindow *win, Configurable *c, bool save)
{
	return favorites->SelectName(win, c, save);
}

// always push the script... even if an error occurred
//   don't log error...
Plugin *PluginManager::LoadAndCompilePlugin(int type, const string &filename)
{
	msg_db_f("LoadAndCompilePlugin", 1);

	for (Plugin *p : plugins)
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
	msg_db_f("ExecutePlugin", 1);

	Plugin *p = LoadAndCompilePlugin(Plugin::TYPE_OTHER, filename);
	if (!p->usable){
		tsunami->log->error(p->getError());
		return;
	}

	Script::Script *s = p->s;

	main_void_func *f_main = (main_void_func*)s->MatchFunction("main", "void", 0);
	if (f_main){
		f_main();
	}else{
		tsunami->log->error(_("Plugin does not contain a function 'void main()'"));
	}
}


Plugin *PluginManager::GetPlugin(int type, const string &name)
{
	for (PluginFile &pf : plugin_files){
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
	Array<DirEntry> list = dir_search(HuiAppDirectoryStatic + "Plugins/Synthesizer/", "*.kaba", false);
	for (DirEntry &e : list)
		names.add(e.name.replace(".kaba", ""));
	return names;
}

Synthesizer *PluginManager::LoadSynthesizer(const string &name, Song *song)
{
	string filename = HuiAppDirectoryStatic + "Plugins/Synthesizer/" + name + ".kaba";
	if (!file_test_existence(filename))
		return NULL;
	Script::Script *s;
	try{
		s = Script::Load(filename);
	}catch(Script::Exception &e){
		tsunami->log->error(e.message);
		return NULL;
	}
	for (auto *t : s->syntax->types){
		if (t->GetRoot()->name != "Synthesizer")
			continue;
		Synthesizer *synth = (Synthesizer*)t->CreateInstance();
		synth->song = song;
		synth->setSampleRate(song->sample_rate);
		return synth;
	}
	return NULL;
}

Effect* PluginManager::ChooseEffect(HuiPanel *parent, Song *song)
{
	ConfigurableSelectorDialog *dlg = new ConfigurableSelectorDialog(parent->win, Configurable::TYPE_EFFECT, song);
	dlg->run();
	return (Effect*)ConfigurableSelectorDialog::_return;
}

MidiEffect* PluginManager::ChooseMidiEffect(HuiPanel *parent, Song *song)
{
	ConfigurableSelectorDialog *dlg = new ConfigurableSelectorDialog(parent->win, Configurable::TYPE_MIDI_EFFECT, song);
	dlg->run();
	return (MidiEffect*)ConfigurableSelectorDialog::_return;
}

/*Synthesizer* PluginManager::ChooseSynthesizer(HuiPanel *parent)
{
	string name = ChooseConfigurable(parent, Configurable::TYPE_SYNTHESIZER);
	if (name == "")
		return NULL;
	return CreateSynthesizer(name);
}*/


