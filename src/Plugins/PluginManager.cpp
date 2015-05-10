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
#include "../Audio/AudioRenderer.h"
#include "../Audio/AudioInput.h"
#include "../Audio/AudioOutput.h"
#include "../Audio/AudioStream.h"
#include "../Audio/Synth/Synthesizer.h"
#include "../Audio/Synth/DummySynthesizer.h"
#include "../Audio/Synth/SynthesizerRenderer.h"
#include "../View/Helper/Progress.h"
#include "../Storage/Storage.h"
#include "../Stuff/Log.h"
#include "../View/AudioView.h"
#include "../View/Dialog/ConfigurableSelectorDialog.h"
#include "../View/BottomBar/SampleManager.h"
#include "Plugin.h"
#include "Effect.h"
#include "ConfigPanel.h"
#include "MidiEffect.h"
#include "FavoriteManager.h"

#define _offsetof(CLASS, ELEMENT) (int)( (char*)&((CLASS*)1)->ELEMENT - (char*)((CLASS*)1) )


void PluginManager::PluginContext::set(Track *t, int l, const Range &r)
{
	track = t;
	level = l;
	range = r;
	track_no = get_track_index(t);
}

PluginManager::PluginManager() :
	Observer("PluginManager")
{
	cur_plugin = NULL;
	cur_effect = NULL;

	favorites = new FavoriteManager;

	ErrorApplyingEffect = false;

	FindPlugins();
	LinkAppScriptData();
}

PluginManager::~PluginManager()
{
	delete(favorites);
}


Array<Slider*> global_slider;

void GlobalCreateSlider(HuiPanel *panel, const string &id_slider, const string &id_edit, float v_min, float v_max, float factor, hui_callback *func, float value)
{	global_slider.add(new Slider(panel, id_slider, id_edit, v_min, v_max, factor, func, value));	}

void GlobalCreateSliderM(HuiPanel *panel, const string &id_slider, const string &id_edit, float v_min, float v_max, float factor, hui_kaba_callback *func, float value)
{	global_slider.add(new Slider(panel, id_slider, id_edit, v_min, v_max, factor, func, value));	}

void GlobalSliderSet(HuiPanel *panel, const string &id, float value)
{
	foreach(Slider *s, global_slider)
		if (s->match(panel, id))
			s->set(value);
}

float GlobalSliderGet(HuiPanel *panel, const string &id)
{
	foreach(Slider *s, global_slider)
		if (s->match(panel, id))
			return s->get();
	return 0;
}

void GlobalRemoveSliders(HuiPanel *panel)
{
	foreach(Slider *s, global_slider)
		delete(s);
	global_slider.clear();
}

bool GlobalAllowTermination()
{
	return tsunami->AllowTermination();
}

void PluginManager::LinkAppScriptData()
{
	msg_db_f("LinkAppScriptData", 2);
	Script::config.directory = "";

	// api definition
	Script::LinkExternal("MainWin", &tsunami->_win);
	Script::LinkExternal("audio", &tsunami->audio);
	Script::LinkExternal("input", &tsunami->input);
	Script::LinkExternal("output", &tsunami->output);
	Script::LinkExternal("storage", &tsunami->storage);
	Script::LinkExternal("logging", &tsunami->log);
	Script::LinkExternal("view", &tsunami->_view);
	Script::LinkExternal("fft_c2c", (void*)&FastFourierTransform::fft_c2c);
	Script::LinkExternal("fft_r2c", (void*)&FastFourierTransform::fft_r2c);
	Script::LinkExternal("fft_c2r_inv", (void*)&FastFourierTransform::fft_c2r_inv);
	/*Script::LinkExternal("ProgressStart", (void*)&ProgressStart);
	Script::LinkExternal("ProgressEnd", (void*)&ProgressEnd);
	Script::LinkExternal("Progress", (void*)&ProgressStatus);*/
	Script::LinkExternal("CreateSlider", (void*)&GlobalCreateSlider);
	Script::LinkExternal("CreateSliderM", (void*)&GlobalCreateSliderM);
	Script::LinkExternal("SliderSet", (void*)&GlobalSliderSet);
	Script::LinkExternal("SliderGet", (void*)&GlobalSliderGet);
	Script::LinkExternal("RemoveSliders", (void*)&GlobalRemoveSliders);
	Script::LinkExternal("CreateSynthesizer", (void*)&CreateSynthesizer);
	Script::LinkExternal("AllowTermination", (void*)&GlobalAllowTermination);
	Script::LinkExternal("SelectSample", (void*)&SampleManager::select);

	Script::DeclareClassSize("Range", sizeof(Range));
	Script::DeclareClassOffset("Range", "offset", _offsetof(Range, offset));
	Script::DeclareClassOffset("Range", "length", _offsetof(Range, num));


	PluginData plugin_data;
	Script::DeclareClassSize("PluginData", sizeof(PluginData));
	Script::LinkExternal("PluginData.__init__", Script::mf(&PluginData::__init__));
	Script::DeclareClassVirtualIndex("PluginData", "__delete__", Script::mf(&PluginData::__delete__), &plugin_data);
	Script::DeclareClassVirtualIndex("PluginData", "reset", Script::mf(&PluginData::reset), &plugin_data);


	ConfigPanel config_panel;
	Script::DeclareClassSize("ConfigPanel", sizeof(ConfigPanel));
	Script::LinkExternal("ConfigPanel.__init__", Script::mf(&ConfigPanel::__init__));
	Script::DeclareClassVirtualIndex("ConfigPanel", "__delete__", Script::mf(&ConfigPanel::__delete__), &config_panel);
	Script::DeclareClassVirtualIndex("ConfigPanel", "update", Script::mf(&ConfigPanel::update), &config_panel);
	Script::LinkExternal("ConfigPanel.notify", Script::mf(&ConfigPanel::notify));
	Script::DeclareClassOffset("ConfigPanel", "c", _offsetof(ConfigPanel, c));


	Effect effect;
	Script::DeclareClassSize("AudioEffect", sizeof(Effect));
	Script::DeclareClassOffset("AudioEffect", "name", _offsetof(Effect, name));
	Script::DeclareClassOffset("AudioEffect", "only_on_selection", _offsetof(Effect, only_on_selection));
	Script::DeclareClassOffset("AudioEffect", "range", _offsetof(Effect, range));
	Script::DeclareClassOffset("AudioEffect", "usable", _offsetof(Effect, usable));
	Script::LinkExternal("AudioEffect.__init__", Script::mf(&Effect::__init__));
	Script::DeclareClassVirtualIndex("AudioEffect", "__delete__", Script::mf(&Effect::__delete__), &effect);
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
	Script::LinkExternal("MidiEffect.__init__", Script::mf(&MidiEffect::__init__));
	Script::DeclareClassVirtualIndex("MidiEffect", "__delete__", Script::mf(&MidiEffect::__delete__), &midieffect);
	Script::DeclareClassVirtualIndex("MidiEffect", "process", Script::mf(&MidiEffect::process), &midieffect);
	Script::DeclareClassVirtualIndex("MidiEffect", "createPanel", Script::mf(&MidiEffect::createPanel), &midieffect);
	Script::LinkExternal("MidiEffect.resetConfig", Script::mf(&MidiEffect::resetConfig));
	Script::LinkExternal("MidiEffect.resetState", Script::mf(&MidiEffect::resetState));
	//Script::DeclareClassVirtualIndex("MidiEffect", "updateDialog", Script::mf(&MidiEffect::UpdateDialog), &midieffect);
	Script::LinkExternal("MidiEffect.notify", Script::mf(&MidiEffect::notify));
	Script::DeclareClassVirtualIndex("MidiEffect", "onConfig", Script::mf(&MidiEffect::onConfig), &midieffect);

	Script::DeclareClassSize("BufferBox", sizeof(BufferBox));
	Script::DeclareClassOffset("BufferBox", "offset", _offsetof(BufferBox, offset));
	Script::DeclareClassOffset("BufferBox", "num", _offsetof(BufferBox, num));
	Script::DeclareClassOffset("BufferBox", "r", _offsetof(BufferBox, r));
	Script::DeclareClassOffset("BufferBox", "l", _offsetof(BufferBox, l));
	Script::DeclareClassOffset("BufferBox", "peak", _offsetof(BufferBox, peak));
	Script::LinkExternal("BufferBox.clear", Script::mf(&BufferBox::clear));
	Script::LinkExternal("BufferBox.__assign__", Script::mf(&BufferBox::__assign__));
	Script::LinkExternal("BufferBox.range", Script::mf(&BufferBox::range));
	Script::LinkExternal("BufferBox.add", Script::mf(&BufferBox::add));
	Script::LinkExternal("BufferBox.set", Script::mf(&BufferBox::set));
	Script::LinkExternal("BufferBox.get_spectrum", Script::mf(&ExtendedBufferBox::get_spectrum));

	Script::DeclareClassSize("Sample", sizeof(Sample));
	Script::DeclareClassOffset("Sample", "name", _offsetof(Sample, name));
	Script::DeclareClassOffset("Sample", "type", _offsetof(Sample, type));
	Script::DeclareClassOffset("Sample", "buf", _offsetof(Sample, buf));
	Script::DeclareClassOffset("Sample", "midi", _offsetof(Sample, midi));
	Script::DeclareClassOffset("Sample", "volume", _offsetof(Sample, volume));
	Script::DeclareClassOffset("Sample", "uid", _offsetof(Sample, uid));
	Script::LinkExternal("Sample.createRef", Script::mf(&Sample::create_ref));

	Sample sample(0);
	sample.owner = tsunami->audio;
	SampleRef sampleref(&sample);
	Script::DeclareClassSize("SampleRef", sizeof(SampleRef));
	Script::DeclareClassOffset("SampleRef", "buf", _offsetof(SampleRef, buf));
	Script::DeclareClassOffset("SampleRef", "midi", _offsetof(SampleRef, midi));
	Script::DeclareClassOffset("SampleRef", "origin", _offsetof(SampleRef, origin));
	Script::LinkExternal("SampleRef.__init__", Script::mf(&SampleRef::__init__));
	Script::DeclareClassVirtualIndex("SampleRef", "__delete__", Script::mf(&SampleRef::__delete__), &sampleref);

	Synthesizer synth;
	Script::DeclareClassSize("Synthesizer", sizeof(Synthesizer));
	Script::DeclareClassOffset("Synthesizer", "name", _offsetof(Synthesizer, name));
	Script::DeclareClassOffset("Synthesizer", "sample_rate", _offsetof(Synthesizer, sample_rate));
	Script::DeclareClassOffset("Synthesizer", "events", _offsetof(Synthesizer, events));
	Script::DeclareClassOffset("Synthesizer", "keep_notes", _offsetof(Synthesizer, keep_notes));
	Script::DeclareClassOffset("Synthesizer", "active_pitch", _offsetof(Synthesizer, active_pitch));
	Script::DeclareClassOffset("Synthesizer", "delta_phi", _offsetof(Synthesizer, delta_phi));
	Script::LinkExternal("Synthesizer.__init__", Script::mf(&Synthesizer::__init__));
	Script::DeclareClassVirtualIndex("Synthesizer", "__delete__", Script::mf(&Synthesizer::__delete__), &synth);
	Script::DeclareClassVirtualIndex("Synthesizer", "createPanel", Script::mf(&Synthesizer::createPanel), &synth);
	Script::LinkExternal("Synthesizer.resetConfig", Script::mf(&Synthesizer::resetConfig));
	Script::LinkExternal("Synthesizer.resetState", Script::mf(&Synthesizer::resetState));
	Script::LinkExternal("Synthesizer.enablePitch", Script::mf(&Synthesizer::enablePitch));
	Script::DeclareClassVirtualIndex("Synthesizer", "read", Script::mf(&Synthesizer::read), &synth);
	Script::DeclareClassVirtualIndex("Synthesizer", "render", Script::mf(&Synthesizer::render), &synth);
	Script::DeclareClassVirtualIndex("Synthesizer", "onConfig", Script::mf(&Synthesizer::onConfig), &synth);
	Script::LinkExternal("Synthesizer.addMetronomeClick", Script::mf(&Synthesizer::addMetronomeClick));
	Script::LinkExternal("Synthesizer.add", Script::mf(&Synthesizer::add));
	Script::LinkExternal("Synthesizer.feed", Script::mf(&Synthesizer::feed));
	Script::LinkExternal("Synthesizer.setSampleRate", Script::mf(&Synthesizer::setSampleRate));
	Script::LinkExternal("Synthesizer.notify", Script::mf(&Synthesizer::notify));

	SynthesizerRenderer synthren(NULL);
	Script::DeclareClassSize("SynthesizerRenderer", sizeof(SynthesizerRenderer));
	Script::DeclareClassVirtualIndex("SynthesizerRenderer", "read", Script::mf(&SynthesizerRenderer::read), &synthren);
	Script::DeclareClassVirtualIndex("SynthesizerRenderer", "reset", Script::mf(&SynthesizerRenderer::reset), &synthren);
	Script::LinkExternal("SynthesizerRenderer.add", Script::mf(&SynthesizerRenderer::add));
	Script::LinkExternal("SynthesizerRenderer.resetMidiData", Script::mf(&SynthesizerRenderer::resetMidiData));


	DummySynthesizer dsynth;
	Script::DeclareClassSize("DummySynthesizer", sizeof(DummySynthesizer));
	Script::LinkExternal("DummySynthesizer.__init__", Script::mf(&DummySynthesizer::__init__));
	Script::DeclareClassVirtualIndex("DummySynthesizer", "__delete__", Script::mf(&DummySynthesizer::__delete__), &dsynth);
	Script::DeclareClassVirtualIndex("DummySynthesizer", "render", Script::mf(&DummySynthesizer::render), &dsynth);

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
	Script::DeclareClassOffset("BarPattern", "count", _offsetof(BarPattern, count));
	Script::DeclareClassOffset("BarPattern", "is_selected", _offsetof(BarPattern, is_selected));

	Script::DeclareClassSize("MidiNote", sizeof(MidiNote));
	Script::DeclareClassOffset("MidiNote", "range", _offsetof(MidiNote, range));
	Script::DeclareClassOffset("MidiNote", "pitch", _offsetof(MidiNote, pitch));
	Script::DeclareClassOffset("MidiNote", "volume", _offsetof(MidiNote, volume));

	Script::DeclareClassSize("MidiData", sizeof(MidiData));
	Script::DeclareClassOffset("MidiData", "samples", _offsetof(MidiData, samples));
	Script::LinkExternal("MidiData.__init__", Script::mf(&MidiData::__init__));

	Script::DeclareClassSize("TrackLevel", sizeof(TrackLevel));
	Script::DeclareClassOffset("TrackLevel", "buffer", _offsetof(TrackLevel, buffer));

	Script::DeclareClassSize("Track", sizeof(Track));
	Script::DeclareClassOffset("Track", "type", _offsetof(Track, type));
	Script::DeclareClassOffset("Track", "name", _offsetof(Track, name));
	Script::DeclareClassOffset("Track", "level", _offsetof(Track, level));
//	Script::DeclareClassOffset("Track", "length", _offsetof(Track, length));
//	Script::DeclareClassOffset("Track", "pos", _offsetof(Track, pos));
	Script::DeclareClassOffset("Track", "volume", _offsetof(Track, volume));
	Script::DeclareClassOffset("Track", "panning", _offsetof(Track, panning));
	Script::DeclareClassOffset("Track", "muted", _offsetof(Track, muted));
//	Script::DeclareClassOffset("Track", "rep_num", _offsetof(Track, rep_num));
//	Script::DeclareClassOffset("Track", "rep_delay", _offsetof(Track, rep_delay));
	Script::DeclareClassOffset("Track", "fx", _offsetof(Track, fx));
//	Script::DeclareClassOffset("Track", "sub", _offsetof(Track, sub));
	Script::DeclareClassOffset("Track", "midi", _offsetof(Track, midi));
	Script::DeclareClassOffset("Track", "synth", _offsetof(Track, synth));
//	Script::DeclareClassOffset("Track", "parent", _offsetof(Track, parent));
	Script::DeclareClassOffset("Track", "root", _offsetof(Track, root));
	Script::DeclareClassOffset("Track", "is_selected", _offsetof(Track, is_selected));
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
	Script::LinkExternal("Track.addMidiEvent", Script::mf(&Track::addMidiEvent));
	Script::LinkExternal("Track.addMidiEvents", Script::mf(&Track::addMidiEvents));
	Script::LinkExternal("Track.deleteMidiEvent", Script::mf(&Track::deleteMidiEvent));
	Script::LinkExternal("Track.setSynthesizer", Script::mf(&Track::setSynthesizer));
	Script::LinkExternal("Track.addBars", Script::mf(&Track::addBars));
	Script::LinkExternal("Track.addPause", Script::mf(&Track::addPause));
	Script::LinkExternal("Track.editBar", Script::mf(&Track::editBar));
	Script::LinkExternal("Track.deleteBar", Script::mf(&Track::deleteBar));

	AudioFile af;
	Script::DeclareClassSize("AudioFile", sizeof(AudioFile));
	Script::DeclareClassOffset("AudioFile", "filename", _offsetof(AudioFile, filename));
	Script::DeclareClassOffset("AudioFile", "tag", _offsetof(AudioFile, tag));
	Script::DeclareClassOffset("AudioFile", "sample_rate", _offsetof(AudioFile, sample_rate));
	Script::DeclareClassOffset("AudioFile", "volume", _offsetof(AudioFile, volume));
	Script::DeclareClassOffset("AudioFile", "fx", _offsetof(AudioFile, fx));
	Script::DeclareClassOffset("AudioFile", "track", _offsetof(AudioFile, track));
	Script::DeclareClassOffset("AudioFile", "sample", _offsetof(AudioFile, sample));
	Script::DeclareClassOffset("AudioFile", "level_name", _offsetof(AudioFile, level_name));
	Script::LinkExternal("AudioFile.__init__", Script::mf(&AudioFile::__init__));
	Script::DeclareClassVirtualIndex("AudioFile", "__delete__", Script::mf(&AudioFile::__delete__), &af);
	Script::LinkExternal("AudioFile.newEmpty", Script::mf(&AudioFile::newEmpty));
	Script::LinkExternal("AudioFile.addTrack", Script::mf(&AudioFile::addTrack));
	Script::LinkExternal("AudioFile.deleteTrack", Script::mf(&AudioFile::deleteTrack));
	Script::LinkExternal("AudioFile.getRange", Script::mf(&AudioFile::getRange));
	Script::LinkExternal("AudioFile.getNextBeat", Script::mf(&AudioFile::getNextBeat));

	AudioRendererInterface ari;
	Script::DeclareClassSize("AudioRendererInterface", sizeof(AudioRendererInterface));
	Script::DeclareClassOffset("AudioRendererInterface", "sample_rate", _offsetof(AudioRendererInterface, sample_rate));
	Script::LinkExternal("AudioRendererInterface.__init__", Script::mf(&AudioRendererInterface::__init__));
	Script::DeclareClassVirtualIndex("AudioRendererInterface", "__delete__", Script::mf(&AudioRendererInterface::__delete__), &ari);
	Script::DeclareClassVirtualIndex("AudioRendererInterface", "read", Script::mf(&AudioRendererInterface::read), &ari);
	Script::DeclareClassVirtualIndex("AudioRendererInterface", "reset", Script::mf(&AudioRendererInterface::reset), &ari);
	Script::DeclareClassVirtualIndex("AudioRendererInterface", "range", Script::mf(&AudioRendererInterface::range), &ari);
	Script::DeclareClassVirtualIndex("AudioRendererInterface", "offset", Script::mf(&AudioRendererInterface::offset), &ari);

	AudioRenderer ar;
	Script::DeclareClassSize("AudioRenderer", sizeof(AudioRenderer));
	Script::LinkExternal("AudioRenderer.prepare", Script::mf(&AudioRenderer::prepare));
	Script::LinkExternal("AudioRenderer.renderAudioFile", Script::mf(&AudioRenderer::renderAudioFile));
	Script::LinkExternal("AudioRenderer.__init__", Script::mf(&AudioRenderer::__init__));
	Script::DeclareClassVirtualIndex("AudioRenderer", "__delete__", Script::mf(&AudioRenderer::__delete__), &ar);
	Script::DeclareClassVirtualIndex("AudioRenderer", "read", Script::mf(&AudioRenderer::read), &ari);
	Script::DeclareClassVirtualIndex("AudioRenderer", "reset", Script::mf(&AudioRenderer::reset), &ari);
	Script::DeclareClassVirtualIndex("AudioRenderer", "range", Script::mf(&AudioRenderer::range), &ari);
	Script::DeclareClassVirtualIndex("AudioRenderer", "offset", Script::mf(&AudioRenderer::offset), &ari);

	Script::DeclareClassSize("AudioInput", sizeof(AudioInput));
	Script::DeclareClassOffset("AudioInput", "cur_buf", _offsetof(AudioInput, current_buffer));
	Script::DeclareClassOffset("AudioInput", "buf", _offsetof(AudioInput, buffer));
	Script::DeclareClassOffset("AudioInput", "midi", _offsetof(AudioInput, midi));
	Script::LinkExternal("AudioInput.start", Script::mf(&AudioInput::start));
	Script::LinkExternal("AudioInput.resetSync", Script::mf(&AudioInput::resetSync));
	Script::LinkExternal("AudioInput.stop",	 Script::mf(&AudioInput::stop));
	Script::LinkExternal("AudioInput.isCapturing", Script::mf(&AudioInput::isCapturing));
	Script::LinkExternal("AudioInput.getSampleCount", Script::mf(&AudioInput::getSampleCount));
	Script::LinkExternal("AudioInput.accumulate", Script::mf(&AudioInput::accumulate));
	Script::LinkExternal("AudioInput.resetAccumulation", Script::mf(&AudioInput::resetAccumulation));
	Script::LinkExternal("AudioInput.getDelay", Script::mf(&AudioInput::getDelay));
	Script::LinkExternal("AudioInput.addObserver", Script::mf(&AudioInput::addWrappedObserver));
	Script::LinkExternal("AudioInput.removeObserver", Script::mf(&AudioInput::removeWrappedObserver));
	//Script::LinkExternal("Observable.addObserver", Script::mf(&Observable::AddWrappedObserver);

	AudioStream::JUST_FAKING_IT = true;
	{
	AudioStream stream(NULL);
	Script::DeclareClassSize("AudioStream", sizeof(AudioStream));
	Script::LinkExternal("AudioStream.__init__", Script::mf(&AudioStream::__init__));
	Script::DeclareClassVirtualIndex("AudioStream", "__delete__", Script::mf(&AudioStream::__delete__), &stream);
	//Script::LinkExternal("AudioStream.setSource", Script::mf(&AudioStream::setSource));
	Script::LinkExternal("AudioStream.play", Script::mf(&AudioStream::play));
	Script::LinkExternal("AudioStream.stop", Script::mf(&AudioStream::stop));
	Script::LinkExternal("AudioStream.pause", Script::mf(&AudioStream::pause));
	Script::LinkExternal("AudioStream.isPlaying", Script::mf(&AudioStream::isPlaying));
	Script::LinkExternal("AudioStream.getPos", Script::mf(&AudioStream::getPos));
	Script::LinkExternal("AudioStream.getSampleRate", Script::mf(&AudioStream::getSampleRate));
	Script::LinkExternal("AudioStream.getVolume", Script::mf(&AudioStream::getVolume));
	Script::LinkExternal("AudioStream.setVolume", Script::mf(&AudioStream::setVolume));
	Script::LinkExternal("AudioStream.setBufferSize", Script::mf(&AudioStream::setBufferSize));
	}
	AudioStream::JUST_FAKING_IT = false;

	Script::DeclareClassSize("AudioView", sizeof(AudioView));
	Script::DeclareClassOffset("AudioView", "sel_range", _offsetof(AudioView, sel_range));
	Script::DeclareClassOffset("AudioView", "sel_raw", _offsetof(AudioView, sel_raw));
	Script::DeclareClassOffset("AudioView", "stream", _offsetof(AudioView, stream));
	Script::DeclareClassOffset("AudioView", "renderer", _offsetof(AudioView, renderer));

	Script::LinkExternal("Log.error", Script::mf(&Log::error));
	Script::LinkExternal("Log.warning", Script::mf(&Log::warning));
	Script::LinkExternal("Log.info", Script::mf(&Log::info));

	Script::LinkExternal("Storage.load", Script::mf(&Storage::load));
	Script::LinkExternal("Storage.save", Script::mf(&Storage::save));

	Script::DeclareClassSize("PluginContext", sizeof(PluginManager::PluginContext));
	Script::DeclareClassOffset("PluginContext", "track", _offsetof(PluginManager::PluginContext, track));
	Script::DeclareClassOffset("PluginContext", "track_no", _offsetof(PluginManager::PluginContext, track_no));
	Script::DeclareClassOffset("PluginContext", "range", _offsetof(PluginManager::PluginContext, range));
	Script::DeclareClassOffset("PluginContext", "level", _offsetof(PluginManager::PluginContext, level));
	Script::LinkExternal("plugin_context",	(void*)&context);
}

void PluginManager::OnMenuExecutePlugin()
{
	int n = s2i(HuiGetEvent()->id.substr(strlen("execute_plugin_"), -1));

	if ((n >= 0) && (n < plugin_file.num))
		ExecutePlugin(plugin_file[n].filename);
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

void find_plugins_in_dir(const string &dir, PluginManager *pm)
{
	Array<DirEntry> list = dir_search(HuiAppDirectoryStatic + "Plugins/" + dir, "*.kaba", false);
	foreach(DirEntry &e, list){
		PluginManager::PluginFile pf;
		pf.name = e.name.replace(".kaba", "");
		pf.filename = HuiAppDirectoryStatic + "Plugins/" + dir + e.name;
		get_plugin_file_data(pf);
		pm->plugin_file.add(pf);
	}
}

void add_plugins_in_dir(const string &dir, PluginManager *pm, HuiMenu *m)
{
	foreachi(PluginManager::PluginFile &f, pm->plugin_file, i){
		if (f.filename.find(dir) >= 0){
            m->addItemImage(f.name, f.image, format("execute_plugin_%d", i));
		}
	}
}

void PluginManager::FindPlugins()
{
	msg_db_f("FindPlugins", 2);
	Script::Init();

	// "Buffer"
	find_plugins_in_dir("Buffer/Channels/", this);
	find_plugins_in_dir("Buffer/Dynamics/", this);
	find_plugins_in_dir("Buffer/Echo/", this);
	find_plugins_in_dir("Buffer/Pitch/", this);
	find_plugins_in_dir("Buffer/Repair/", this);
	find_plugins_in_dir("Buffer/Sound/", this);
	find_plugins_in_dir("Buffer/Synthesizer/", this);

	// "Midi"
	find_plugins_in_dir("Midi/", this);

	// "All"
	find_plugins_in_dir("All/", this);

	// rest
	find_plugins_in_dir("Independent/", this);
}

void PluginManager::AddPluginsToMenu(HuiWindow *win)
{
	msg_db_f("AddPluginsToMenu", 2);

	HuiMenu *m = win->getMenu();

	// "Buffer"
	add_plugins_in_dir("Buffer/Channels/", this, m->getSubMenuByID("menu_plugins_channels"));
	add_plugins_in_dir("Buffer/Dynamics/", this, m->getSubMenuByID("menu_plugins_dynamics"));
	add_plugins_in_dir("Buffer/Echo/", this, m->getSubMenuByID("menu_plugins_echo"));
	add_plugins_in_dir("Buffer/Pitch/", this, m->getSubMenuByID("menu_plugins_pitch"));
	add_plugins_in_dir("Buffer/Repair/", this, m->getSubMenuByID("menu_plugins_repair"));
	add_plugins_in_dir("Buffer/Sound/", this, m->getSubMenuByID("menu_plugins_sound"));
	add_plugins_in_dir("Buffer/Synthesizer/", this, m->getSubMenuByID("menu_plugins_synthesizer"));

	// "Midi"
	add_plugins_in_dir("Midi/", this, m->getSubMenuByID("menu_plugins_on_midi"));

	// "All"
	add_plugins_in_dir("All/", this, m->getSubMenuByID("menu_plugins_on_audio"));

	// rest
	add_plugins_in_dir("Independent/", this, m->getSubMenuByID("menu_plugins_other"));

	// Events
	for (int i=0;i<plugin_file.num;i++)
		win->event(format("execute_plugin_%d", i), this, &PluginManager::OnMenuExecutePlugin);
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

void PluginManager::onUpdate(Observable *o, const string &message)
{
	if (o == tsunami->progress){
		if (message == tsunami->progress->MESSAGE_CANCEL)
			PreviewEnd();
	}else if (o == tsunami->win->view->stream){
		int pos = tsunami->win->view->stream->getPos();
		Range r = tsunami->win->view->sel_range;
		tsunami->progress->set(_("Vorschau"), (float)(pos - r.offset) / r.length());
		if (!tsunami->win->view->stream->isPlaying())
			PreviewEnd();
	}
}

// always push the script... even if an error occurred
bool PluginManager::LoadAndCompilePlugin(const string &filename)
{
	msg_db_f("LoadAndCompilePlugin", 1);

	//msg_write(filename);

	foreach(Plugin *p, plugin){
		if (filename == p->filename){
			cur_plugin = p;
			return p->usable;
		}
	}

	//InitPluginData();

	Plugin *p = new Plugin(filename);
	p->index = plugin.num;

	plugin.add(p);
	cur_plugin = p;

	return p->usable;
}
typedef void main_audiofile_func(AudioFile*);
typedef void main_void_func();

void PluginManager::ExecutePlugin(const string &filename)
{
	msg_db_f("ExecutePlugin", 1);

	if (LoadAndCompilePlugin(filename)){
		Plugin *p = cur_plugin;
		Script::Script *s = p->s;

		AudioFile *a = tsunami->audio;

		Effect *fx = NULL;
		MidiEffect *mfx = NULL;
		foreach(Script::Type *t, s->syntax->types){
			Script::Type *r = t;
			while (r->parent)
				r = r->parent;
			if (r->name == "AudioEffect"){
				fx = (Effect*)t->CreateInstance();
				fx->name = p->filename.basename();
				fx->name = fx->name.head(fx->name.num - 5);
			}else if (r->name == "MidiEffect"){
				mfx = (MidiEffect*)t->CreateInstance();
				mfx->name = p->filename.basename();
				mfx->name = mfx->name.head(mfx->name.num - 5);
			}else
				continue;
			break;
		}
		main_void_func *f_main = (main_void_func*)s->MatchFunction("main", "void", 0);

		// run
		if (fx){
			tsunami->plugin_manager->cur_effect = fx;
			fx->resetConfig();
			if (fx->configure()){
				main_audiofile_func *f_audio = (main_audiofile_func*)s->MatchFunction("main", "void", 1, "AudioFile*");
			//	main_void_func *f_void = (main_void_func*)s->MatchFunction("main", "void", 0);
				Range range = tsunami->win->view->getPlaybackSelection();
				a->action_manager->beginActionGroup();
				foreach(Track *t, a->track)
					if ((t->is_selected) && (t->type == t->TYPE_AUDIO)){
						fx->resetState();
						fx->doProcessTrack(t, tsunami->win->view->cur_level, range);
					}
				a->action_manager->endActionGroup();
			}
			delete(fx);
		}else if (mfx){
			tsunami->plugin_manager->cur_effect = NULL;//fx;
			mfx->resetConfig();
			if (mfx->configure()){
				Range range = tsunami->win->view->getPlaybackSelection();
				a->action_manager->beginActionGroup();
				foreach(Track *t, a->track)
					if ((t->is_selected) && (t->type == t->TYPE_MIDI)){
						mfx->resetState();
						mfx->DoProcessTrack(t, range);
					}
				a->action_manager->endActionGroup();
			}
			delete(mfx);
		}/*else if (f_audio){
			if (a->used)
				f_audio(a);
			else
				tsunami->log->error(_("Plugin kann nicht f&ur eine leere Audiodatei ausgef&uhrt werden"));
		}*/else if (f_main){
			f_main();
		}else{
			tsunami->log->error(_("Plugin ist kein Effekt/MidiEffekt und enth&alt keine Funktion 'void main()'"));
		}
	}else{
		tsunami->log->error(cur_plugin->GetError());
	}
}


void PluginManager::FindAndExecutePlugin()
{
	msg_db_f("ExecutePlugin", 1);


	if (HuiFileDialogOpen(tsunami->win, _("Plugin-Script w&ahlen"), HuiAppDirectoryStatic + "Plugins/", _("Script (*.kaba)"), "*.kaba")){
		ExecutePlugin(HuiFilename);
	}
}


Plugin *PluginManager::GetPlugin(const string &name)
{
	foreach(PluginFile &pf, plugin_file)
		if (name == pf.name){
			LoadAndCompilePlugin(pf.filename);
			return cur_plugin;
		}
	return NULL;
}


void PluginManager::PreviewStart(Effect *fx)
{
	if (fx)
		fx->configToString();
	tsunami->win->view->renderer->effect = fx;


	tsunami->progress->startCancelable(_("Vorschau"), 0);
	subscribe(tsunami->progress);
	subscribe(tsunami->win->view->stream);
	tsunami->win->view->renderer->prepare(tsunami->audio, tsunami->win->view->sel_range, false);
	tsunami->win->view->stream->play();
}

void PluginManager::PreviewEnd()
{
	tsunami->win->view->stream->stop();
	unsubscribe(tsunami->win->view->stream);
	unsubscribe(tsunami->progress);
	tsunami->progress->end();


	tsunami->win->view->renderer->effect = NULL;
}

Effect *PluginManager::LoadEffect(const string &name)
{
	bool found = false;
	foreach(PluginFile &pf, plugin_file){
		if ((pf.name == name) && (pf.filename.find("/Buffer/") >= 0)){
			found = true;
			if (!LoadAndCompilePlugin(pf.filename))
				return NULL;
		}
	}
	if (!found){
		tsunami->log->error(format(_("Kann Effekt nicht laden: %s"), name.c_str()));
		return NULL;
	}

	Script::Script *s = cur_plugin->s;
	foreach(Script::Type *t, s->syntax->types){
		if (t->GetRoot()->name != "AudioEffect")
			continue;
		return (Effect*)t->CreateInstance();
	}
	return NULL;
}

MidiEffect *PluginManager::LoadMidiEffect(const string &name)
{
	bool found = false;
	foreach(PluginFile &pf, plugin_file){
		if ((pf.name == name) && (pf.filename.find("/Midi/") >= 0)){
			found = true;
			if (!LoadAndCompilePlugin(pf.filename))
				return NULL;
		}
	}
	if (!found){
		tsunami->log->error(format(_("Kann MidiEffekt nicht laden: %s"), name.c_str()));
		return NULL;
	}

	Script::Script *s = cur_plugin->s;
	foreach(Script::Type *t, s->syntax->types){
		if (t->GetRoot()->name != "MidiEffect")
			continue;
		return (MidiEffect*)t->CreateInstance();
	}
	return NULL;
}


Array<string> PluginManager::FindSynthesizers()
{
	Array<string> names;
	Array<DirEntry> list = dir_search(HuiAppDirectoryStatic + "Plugins/Synthesizer/", "*.kaba", false);
	foreach(DirEntry &e, list)
		names.add(e.name.replace(".kaba", ""));
	return names;
}

Synthesizer *PluginManager::LoadSynthesizer(const string &name)
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
	foreach(Script::Type *t, s->syntax->types){
		if (t->GetRoot()->name != "Synthesizer")
			continue;
		return (Synthesizer*)t->CreateInstance();
	}
	return NULL;
}

Effect* PluginManager::ChooseEffect(HuiPanel *parent)
{
	ConfigurableSelectorDialog *dlg = new ConfigurableSelectorDialog(parent->win, Configurable::TYPE_EFFECT);
	dlg->run();
	return (Effect*)ConfigurableSelectorDialog::_return;
}

MidiEffect* PluginManager::ChooseMidiEffect(HuiPanel *parent)
{
	ConfigurableSelectorDialog *dlg = new ConfigurableSelectorDialog(parent->win, Configurable::TYPE_MIDI_EFFECT);
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


