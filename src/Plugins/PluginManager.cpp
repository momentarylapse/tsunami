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
#include "../Audio/Synth/Synthesizer.h"
#include "../Audio/Synth/DummySynthesizer.h"
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
		if (s->Match(panel, id))
			s->Set(value);
}

float GlobalSliderGet(HuiPanel *panel, const string &id)
{
	foreach(Slider *s, global_slider)
		if (s->Match(panel, id))
			return s->Get();
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
	Script::config.Directory = "";

	// api definition
	Script::LinkExternal("MainWin", &tsunami->_win);
	Script::LinkExternal("audio", &tsunami->audio);
	Script::LinkExternal("input", &tsunami->input);
	Script::LinkExternal("output", &tsunami->output);
	Script::LinkExternal("renderer", &tsunami->renderer);
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
	Script::LinkExternal("SelectSample", (void*)&SampleManager::Select);

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
	Script::DeclareClassVirtualIndex("AudioEffect", "process", Script::mf(&Effect::ProcessTrack), &effect);
	Script::DeclareClassVirtualIndex("AudioEffect", "createPanel", Script::mf(&Effect::CreatePanel), &effect);
	Script::DeclareClassVirtualIndex("AudioEffect", "resetConfig", Script::mf(&Effect::ResetConfig), &effect);
	Script::DeclareClassVirtualIndex("AudioEffect", "resetState", Script::mf(&Effect::ResetState), &effect);
	//Script::DeclareClassVirtualIndex("AudioEffect", "updateDialog", Script::mf(&Effect::UpdateDialog), &effect);
	Script::LinkExternal("AudioEffect.notify", Script::mf(&Effect::notify));

	MidiEffect midieffect;
	Script::DeclareClassSize("MidiEffect", sizeof(MidiEffect));
	Script::DeclareClassOffset("MidiEffect", "name", _offsetof(MidiEffect, name));
	Script::DeclareClassOffset("MidiEffect", "only_on_selection", _offsetof(MidiEffect, only_on_selection));
	Script::DeclareClassOffset("MidiEffect", "range", _offsetof(MidiEffect, range));
	Script::DeclareClassOffset("MidiEffect", "usable", _offsetof(MidiEffect, usable));
	Script::LinkExternal("MidiEffect.__init__", Script::mf(&MidiEffect::__init__));
	Script::DeclareClassVirtualIndex("MidiEffect", "__delete__", Script::mf(&MidiEffect::__delete__), &midieffect);
	Script::DeclareClassVirtualIndex("MidiEffect", "process", Script::mf(&MidiEffect::process), &midieffect);
	Script::DeclareClassVirtualIndex("MidiEffect", "createPanel", Script::mf(&MidiEffect::CreatePanel), &midieffect);
	Script::DeclareClassVirtualIndex("MidiEffect", "resetConfig", Script::mf(&MidiEffect::ResetConfig), &midieffect);
	Script::DeclareClassVirtualIndex("MidiEffect", "resetState", Script::mf(&MidiEffect::ResetState), &midieffect);
	//Script::DeclareClassVirtualIndex("MidiEffect", "updateDialog", Script::mf(&MidiEffect::UpdateDialog), &midieffect);
	Script::LinkExternal("MidiEffect.notify", Script::mf(&MidiEffect::notify));

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
	Script::DeclareClassOffset("Sample", "buf", _offsetof(Sample, buf));
	Script::DeclareClassOffset("Sample", "volume", _offsetof(Sample, volume));
	Script::DeclareClassOffset("Sample", "uid", _offsetof(Sample, uid));
	Script::LinkExternal("Sample.createRef", Script::mf(&Sample::create_ref));

	Sample sample;
	sample.owner = tsunami->audio;
	SampleRef sampleref(&sample);
	Script::DeclareClassSize("SampleRef", sizeof(SampleRef));
	Script::DeclareClassOffset("SampleRef", "buf", _offsetof(SampleRef, buf));
	Script::DeclareClassOffset("SampleRef", "origin", _offsetof(SampleRef, origin));
	Script::LinkExternal("SampleRef.__init__", Script::mf(&SampleRef::__init__));
	Script::DeclareClassVirtualIndex("SampleRef", "__delete__", Script::mf(&SampleRef::__delete__), &sampleref);

	Synthesizer synth;
	Script::DeclareClassSize("Synthesizer", sizeof(Synthesizer));
	Script::DeclareClassOffset("Synthesizer", "name", _offsetof(Synthesizer, name));
	Script::DeclareClassOffset("Synthesizer", "sample_rate", _offsetof(Synthesizer, sample_rate));
	Script::DeclareClassOffset("Synthesizer", "keep_notes", _offsetof(Synthesizer, keep_notes));
	Script::LinkExternal("Synthesizer.__init__", Script::mf(&Synthesizer::__init__));
	Script::DeclareClassVirtualIndex("Synthesizer", "__delete__", Script::mf(&Synthesizer::__delete__), &synth);
	Script::DeclareClassVirtualIndex("Synthesizer", "renderNote", Script::mf(&Synthesizer::RenderNote), &synth);
	Script::DeclareClassVirtualIndex("Synthesizer", "read", Script::mf(&Synthesizer::read), &synth);
	Script::DeclareClassVirtualIndex("Synthesizer", "createPanel", Script::mf(&Synthesizer::CreatePanel), &synth);
	//Script::DeclareClassVirtualIndex("Synthesizer", "updateDialog", Script::mf(&Synthesizer::UpdateDialog), &synth);
	Script::DeclareClassVirtualIndex("Synthesizer", "reset", Script::mf(&Synthesizer::Reset), &synth);
	Script::DeclareClassVirtualIndex("Synthesizer", "resetConfig", Script::mf(&Synthesizer::ResetConfig), &synth);
	Script::LinkExternal("Synthesizer.set", Script::mf(&Synthesizer::set));
	Script::LinkExternal("Synthesizer.renderMetronomeClick", Script::mf(&Synthesizer::RenderMetronomeClick));
	Script::LinkExternal("Synthesizer._reset", Script::mf(&Synthesizer::reset));
	Script::LinkExternal("Synthesizer.notify", Script::mf(&Synthesizer::notify));


	DummySynthesizer dsynth;
	Script::DeclareClassSize("DummySynthesizer", sizeof(DummySynthesizer));
	Script::LinkExternal("DummySynthesizer.__init__", Script::mf(&DummySynthesizer::__init__));
	Script::DeclareClassVirtualIndex("DummySynthesizer", "__delete__", Script::mf(&DummySynthesizer::__delete__), &dsynth);
	Script::DeclareClassVirtualIndex("DummySynthesizer", "renderNote", Script::mf(&DummySynthesizer::RenderNote), &dsynth);
	//Script::DeclareClassVirtualIndex("DummySynthesizer", "read", Script::mf(&DummySynthesizer::read), &dsynth);
	//Script::DeclareClassVirtualIndex("DummySynthesizer", "onConfigure", Script::mf(&DummySynthesizer::OnConfigure), &dsynth);
	//Script::LinkExternal("DummySynthesizer.set", Script::mf(&Synthesizer::set));
	//Script::LinkExternal("DummySynthesizer.renderMetronomeClick", Script::mf(&Synthesizer::RenderMetronomeClick));

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
	Script::DeclareClassOffset("MidiData", "note", 0); //_offsetof(MidiData, note));

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
	Script::DeclareClassOffset("Track", "area", _offsetof(Track, area));
//	Script::DeclareClassOffset("Track", "parent", _offsetof(Track, parent));
	Script::DeclareClassOffset("Track", "root", _offsetof(Track, root));
	Script::DeclareClassOffset("Track", "is_selected", _offsetof(Track, is_selected));
	Script::LinkExternal("Track.getBuffers", Script::mf(&Track::GetBuffers));
	Script::LinkExternal("Track.readBuffers", Script::mf(&Track::ReadBuffers));
	Script::LinkExternal("Track.setName", Script::mf(&Track::SetName));
	Script::LinkExternal("Track.setMuted", Script::mf(&Track::SetMuted));
	Script::LinkExternal("Track.setVolume", Script::mf(&Track::SetVolume));
	Script::LinkExternal("Track.setPanning", Script::mf(&Track::SetPanning));
	Script::LinkExternal("Track.insertMidiData", Script::mf(&Track::InsertMidiData));
	Script::LinkExternal("Track.addEffect", Script::mf(&Track::AddEffect));
	Script::LinkExternal("Track.deleteEffect", Script::mf(&Track::DeleteEffect));
	Script::LinkExternal("Track.editEffect", Script::mf(&Track::EditEffect));
	Script::LinkExternal("Track.enableEffect", Script::mf(&Track::EnableEffect));
	Script::LinkExternal("Track.addSample", Script::mf(&Track::AddSample));
	Script::LinkExternal("Track.deleteSample", Script::mf(&Track::DeleteSample));
	Script::LinkExternal("Track.editSample", Script::mf(&Track::EditSample));
	Script::LinkExternal("Track.addMidiNote", Script::mf(&Track::AddMidiNote));
	Script::LinkExternal("Track.addMidiNotes", Script::mf(&Track::AddMidiNotes));
	Script::LinkExternal("Track.deleteMidiNote", Script::mf(&Track::DeleteMidiNote));
	Script::LinkExternal("Track.setSynthesizer", Script::mf(&Track::SetSynthesizer));
	Script::LinkExternal("Track.addBars", Script::mf(&Track::AddBars));
	Script::LinkExternal("Track.addPause", Script::mf(&Track::AddPause));
	Script::LinkExternal("Track.editBar", Script::mf(&Track::EditBar));
	Script::LinkExternal("Track.deleteBar", Script::mf(&Track::DeleteBar));

	Script::DeclareClassSize("AudioFile", sizeof(AudioFile));
	Script::DeclareClassOffset("AudioFile", "used", _offsetof(AudioFile, used));
	Script::DeclareClassOffset("AudioFile", "filename", _offsetof(AudioFile, filename));
	Script::DeclareClassOffset("AudioFile", "tag", _offsetof(AudioFile, tag));
	Script::DeclareClassOffset("AudioFile", "sample_rate", _offsetof(AudioFile, sample_rate));
	Script::DeclareClassOffset("AudioFile", "volume", _offsetof(AudioFile, volume));
	Script::DeclareClassOffset("AudioFile", "fx", _offsetof(AudioFile, fx));
	Script::DeclareClassOffset("AudioFile", "track", _offsetof(AudioFile, track));
	Script::DeclareClassOffset("AudioFile", "sample", _offsetof(AudioFile, sample));
	Script::DeclareClassOffset("AudioFile", "area", _offsetof(AudioFile, area));
	Script::DeclareClassOffset("AudioFile", "level_name", _offsetof(AudioFile, level_name));
	Script::LinkExternal("AudioFile.newEmpty", Script::mf(&AudioFile::NewEmpty));
	Script::LinkExternal("AudioFile.addTrack", Script::mf(&AudioFile::AddTrack));
	Script::LinkExternal("AudioFile.deleteTrack", Script::mf(&AudioFile::DeleteTrack));
	Script::LinkExternal("AudioFile.getRange", Script::mf(&AudioFile::GetRange));
	Script::LinkExternal("AudioFile.getNextBeat", Script::mf(&AudioFile::GetNextBeat));

	Script::LinkExternal("AudioRenderer.prepare", Script::mf(&AudioRenderer::Prepare));
	//Script::LinkExternal("AudioRenderer.read", Script::mf(&AudioRenderer::read));
	Script::LinkExternal("AudioRenderer.renderAudioFile", Script::mf(&AudioRenderer::RenderAudioFile));

	Script::DeclareClassSize("AudioInput", sizeof(AudioInput));
	Script::DeclareClassOffset("AudioInput", "cur_buf", _offsetof(AudioInput, current_buffer));
	Script::DeclareClassOffset("AudioInput", "buf", _offsetof(AudioInput, buffer));
	Script::DeclareClassOffset("AudioInput", "midi", _offsetof(AudioInput, midi));
	Script::LinkExternal("AudioInput.start", Script::mf(&AudioInput::Start));
	Script::LinkExternal("AudioInput.resetSync", Script::mf(&AudioInput::ResetSync));
	Script::LinkExternal("AudioInput.stop",	 Script::mf(&AudioInput::Stop));
	Script::LinkExternal("AudioInput.isCapturing", Script::mf(&AudioInput::IsCapturing));
	Script::LinkExternal("AudioInput.getSampleCount", Script::mf(&AudioInput::GetSampleCount));
	Script::LinkExternal("AudioInput.accumulate", Script::mf(&AudioInput::Accumulate));
	Script::LinkExternal("AudioInput.resetAccumulation", Script::mf(&AudioInput::ResetAccumulation));
	Script::LinkExternal("AudioInput.getDelay", Script::mf(&AudioInput::GetDelay));
	Script::LinkExternal("AudioInput.addObserver", Script::mf(&AudioInput::AddWrappedObserver));
	Script::LinkExternal("AudioInput.removeObserver", Script::mf(&AudioInput::RemoveWrappedObserver));
	//Script::LinkExternal("Observable.addObserver", Script::mf(&Observable::AddWrappedObserver);

	Script::LinkExternal("AudioOutput.play", Script::mf(&AudioOutput::Play));
	Script::LinkExternal("AudioOutput.playGenerated", Script::mf(&AudioOutput::PlayGenerated));
	Script::LinkExternal("AudioOutput.stop", Script::mf(&AudioOutput::Stop));
	Script::LinkExternal("AudioOutput.isPlaying", Script::mf(&AudioOutput::IsPlaying));
	Script::LinkExternal("AudioOutput.getPos", Script::mf(&AudioOutput::GetPos));
	Script::LinkExternal("AudioOutput.getSampleRate", Script::mf(&AudioOutput::GetSampleRate));
	Script::LinkExternal("AudioOutput.getVolume", Script::mf(&AudioOutput::GetVolume));
	Script::LinkExternal("AudioOutput.setVolume", Script::mf(&AudioOutput::SetVolume));
	Script::LinkExternal("AudioOutput.setBufferSize", Script::mf(&AudioOutput::SetBufferSize));

	Script::DeclareClassSize("AudioView", sizeof(AudioView));
	Script::DeclareClassOffset("AudioView", "sel_range", _offsetof(AudioView, sel_range));
	Script::DeclareClassOffset("AudioView", "sel_raw", _offsetof(AudioView, sel_raw));

	Script::LinkExternal("Log.error", Script::mf(&Log::Error));
	Script::LinkExternal("Log.warning", Script::mf(&Log::Warning));
	Script::LinkExternal("Log.info", Script::mf(&Log::Info));

	Script::LinkExternal("Storage.load", Script::mf(&Storage::Load));
	Script::LinkExternal("Storage.save", Script::mf(&Storage::Save));

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
            m->AddItemImage(f.name, f.image, format("execute_plugin_%d", i));
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

	HuiMenu *m = win->GetMenu();

	// "Buffer"
	add_plugins_in_dir("Buffer/Channels/", this, m->GetSubMenuByID("menu_plugins_channels"));
	add_plugins_in_dir("Buffer/Dynamics/", this, m->GetSubMenuByID("menu_plugins_dynamics"));
	add_plugins_in_dir("Buffer/Echo/", this, m->GetSubMenuByID("menu_plugins_echo"));
	add_plugins_in_dir("Buffer/Pitch/", this, m->GetSubMenuByID("menu_plugins_pitch"));
	add_plugins_in_dir("Buffer/Sound/", this, m->GetSubMenuByID("menu_plugins_sound"));
	add_plugins_in_dir("Buffer/Synthesizer/", this, m->GetSubMenuByID("menu_plugins_synthesizer"));

	// "Midi"
	add_plugins_in_dir("Midi/", this, m->GetSubMenuByID("menu_plugins_on_midi"));

	// "All"
	add_plugins_in_dir("All/", this, m->GetSubMenuByID("menu_plugins_on_audio"));

	// rest
	add_plugins_in_dir("Independent/", this, m->GetSubMenuByID("menu_plugins_other"));

	// Events
	for (int i=0;i<plugin_file.num;i++)
		win->EventM(format("execute_plugin_%d", i), this, &PluginManager::OnMenuExecutePlugin);
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

void PluginManager::OnUpdate(Observable *o, const string &message)
{
	if (o == tsunami->progress){
		if (message == tsunami->progress->MESSAGE_CANCEL)
			PreviewEnd();
	}else if (o == tsunami->output){
		int pos = tsunami->output->GetPos();
		Range r = tsunami->win->view->sel_range;
		tsunami->progress->Set(_("Vorschau"), (float)(pos - r.offset) / r.length());
		if (!tsunami->output->IsPlaying())
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
		foreach(Script::Type *t, s->syntax->Types){
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
			fx->ResetConfig();
			if (fx->Configure()){
				main_audiofile_func *f_audio = (main_audiofile_func*)s->MatchFunction("main", "void", 1, "AudioFile*");
			//	main_void_func *f_void = (main_void_func*)s->MatchFunction("main", "void", 0);
				if (a->used){
					Range range = tsunami->win->view->GetPlaybackSelection();
					a->action_manager->BeginActionGroup();
					foreach(Track *t, a->track)
						if ((t->is_selected) && (t->type == t->TYPE_AUDIO)){
							fx->ResetState();
							fx->DoProcessTrack(t, tsunami->win->view->cur_level, range);
						}
					a->action_manager->EndActionGroup();
				}else{
					tsunami->log->Error(_("Plugin kann nicht f&ur eine leere Audiodatei ausgef&uhrt werden"));
				}
			}
			delete(fx);
		}else if (mfx){
			tsunami->plugin_manager->cur_effect = NULL;//fx;
			mfx->ResetConfig();
			if (mfx->Configure()){
				if (a->used){
					Range range = tsunami->win->view->GetPlaybackSelection();
					a->action_manager->BeginActionGroup();
					foreach(Track *t, a->track)
						if ((t->is_selected) && (t->type == t->TYPE_MIDI)){
							mfx->ResetState();
							mfx->DoProcessTrack(t, range);
						}
					a->action_manager->EndActionGroup();
				}else{
					tsunami->log->Error(_("Plugin kann nicht f&ur eine leere Audiodatei ausgef&uhrt werden"));
				}
			}
			delete(mfx);
		}/*else if (f_audio){
			if (a->used)
				f_audio(a);
			else
				tsunami->log->Error(_("Plugin kann nicht f&ur eine leere Audiodatei ausgef&uhrt werden"));
		}*/else if (f_main){
			f_main();
		}else{
			tsunami->log->Error(_("Plugin ist kein Effekt/MidiEffekt und enth&alt keine Funktion 'void main()'"));
		}
	}else{
		tsunami->log->Error(cur_plugin->GetError());
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
		fx->ConfigToString();
	tsunami->renderer->effect = fx;


	tsunami->progress->StartCancelable(_("Vorschau"), 0);
	Subscribe(tsunami->progress);
	Subscribe(tsunami->output);
	tsunami->renderer->Prepare(tsunami->audio, tsunami->win->view->sel_range, false);
	tsunami->output->Play(tsunami->renderer);
}

void PluginManager::PreviewEnd()
{
	tsunami->output->Stop();
	Unsubscribe(tsunami->output);
	Unsubscribe(tsunami->progress);
	tsunami->progress->End();


	tsunami->renderer->effect = NULL;
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
		tsunami->log->Error(format(_("Kann Effekt nicht laden: %s"), name.c_str()));
		return NULL;
	}

	Script::Script *s = cur_plugin->s;
	foreach(Script::Type *t, s->syntax->Types){
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
		tsunami->log->Error(format(_("Kann MidiEffekt nicht laden: %s"), name.c_str()));
		return NULL;
	}

	Script::Script *s = cur_plugin->s;
	foreach(Script::Type *t, s->syntax->Types){
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
		tsunami->log->Error(e.message);
		return NULL;
	}
	foreach(Script::Type *t, s->syntax->Types){
		if (t->GetRoot()->name != "Synthesizer")
			continue;
		return (Synthesizer*)t->CreateInstance();
	}
	return NULL;
}

Effect* PluginManager::ChooseEffect(HuiPanel *parent)
{
	ConfigurableSelectorDialog *dlg = new ConfigurableSelectorDialog(parent->win, Configurable::TYPE_EFFECT);
	dlg->Run();
	return (Effect*)ConfigurableSelectorDialog::_return;
}

MidiEffect* PluginManager::ChooseMidiEffect(HuiPanel *parent)
{
	ConfigurableSelectorDialog *dlg = new ConfigurableSelectorDialog(parent->win, Configurable::TYPE_MIDI_EFFECT);
	dlg->Run();
	return (MidiEffect*)ConfigurableSelectorDialog::_return;
}

/*Synthesizer* PluginManager::ChooseSynthesizer(HuiPanel *parent)
{
	string name = ChooseConfigurable(parent, Configurable::TYPE_SYNTHESIZER);
	if (name == "")
		return NULL;
	return CreateSynthesizer(name);
}*/


