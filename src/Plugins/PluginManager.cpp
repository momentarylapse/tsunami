/*
 * PluginManager.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "PluginManager.h"
#include "../Tsunami.h"
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
#include "Plugin.h"
#include "Effect.h"


void PluginManager::PluginContext::set(Track *t, int l, const Range &r)
{
	track = t;
	level = l;
	range = r;
	track_no = get_track_index(t);
}

PluginManager::PluginManager()
{
}

PluginManager::~PluginManager()
{
}


BufferBox AudioFileRender(AudioFile *a, const Range &r)
{	return tsunami->renderer->RenderAudioFile(a, r);	}

void GlobalPutFavoriteBarFixed(HuiWindow *win, int x, int y, int w)
{	tsunami->plugin_manager->PutFavoriteBarFixed(win, x, y, w);	}

void GlobalPutFavoriteBarSizable(HuiWindow *win, const string &root_id, int x, int y)
{	tsunami->plugin_manager->PutFavoriteBarSizable(win, root_id, x, y);	}

void GlobalPutCommandBarFixed(HuiWindow *win, int x, int y, int w)
{	tsunami->plugin_manager->PutCommandBarFixed(win, x, y, w);	}

void GlobalPutCommandBarSizable(HuiWindow *win, const string &root_id, int x, int y)
{	tsunami->plugin_manager->PutCommandBarSizable(win, root_id, x, y);	}

Array<Slider*> global_slider;

void GlobalAddSlider(HuiWindow *win, const string &id_slider, const string &id_edit, float v_min, float v_max, float factor, hui_callback *func, float value)
{	global_slider.add(new Slider(win, id_slider, id_edit, v_min, v_max, factor, func, value));	}

void GlobalSliderSet(HuiWindow *win, const string &id, float value)
{
	foreach(Slider *s, global_slider)
		if (s->Match(id))
				s->Set(value);
}

float GlobalSliderGet(HuiWindow *win, const string &id)
{
	foreach(Slider *s, global_slider)
		if (s->Match(id))
				return s->Get();
	return 0;
}

void GlobalRemoveSliders(HuiWindow *win)
{
	foreach(Slider *s, global_slider)
		delete(s);
	global_slider.clear();
}

HuiWindow *GlobalMainWin = NULL;

void PluginManager::LinkAppScriptData()
{
	msg_db_f("LinkAppScriptData", 2);
	Script::config.Directory = "";

	// api definition
	GlobalMainWin = dynamic_cast<HuiWindow*>(tsunami);
	Script::LinkExternal("MainWin", &GlobalMainWin);
	Script::LinkExternal("audio", &tsunami->audio);
	Script::LinkExternal("input", &tsunami->input);
	Script::LinkExternal("output", &tsunami->output);
	Script::LinkExternal("renderer", &tsunami->renderer);
	Script::LinkExternal("storage", &tsunami->storage);
	Script::LinkExternal("logging", &tsunami->log);
	Script::LinkExternal("fft_c2c", (void*)&FastFourierTransform::fft_c2c);
	Script::LinkExternal("fft_r2c", (void*)&FastFourierTransform::fft_r2c);
	Script::LinkExternal("fft_c2r_inv", (void*)&FastFourierTransform::fft_c2r_inv);
	/*Script::LinkExternal("ProgressStart", (void*)&ProgressStart);
	Script::LinkExternal("ProgressEnd", (void*)&ProgressEnd);
	Script::LinkExternal("Progress", (void*)&ProgressStatus);*/
	Script::LinkExternal("PutFavoriteBarFixed", (void*)&GlobalPutFavoriteBarFixed);
	Script::LinkExternal("PutFavoriteBarSizable", (void*)&GlobalPutFavoriteBarSizable);
	Script::LinkExternal("PutCommandBarFixed", (void*)&GlobalPutCommandBarFixed);
	Script::LinkExternal("PutCommandBarSizable", (void*)&GlobalPutCommandBarSizable);
	Script::LinkExternal("AddSlider", (void*)&GlobalAddSlider);
	Script::LinkExternal("SliderSet", (void*)&GlobalSliderSet);
	Script::LinkExternal("SliderGet", (void*)&GlobalSliderGet);
	Script::LinkExternal("RemoveSliders", (void*)&GlobalRemoveSliders);
	Script::LinkExternal("AudioFileRender",	 (void*)&AudioFileRender);
	Script::LinkExternal("CreateSynthesizer", (void*)&CreateSynthesizer);

	Script::DeclareClassSize("Range", sizeof(Range));
	Script::DeclareClassOffset("Range", "offset", offsetof(Range, offset));
	Script::DeclareClassOffset("Range", "length", offsetof(Range, num));

	Script::DeclareClassSize("EffectParam", sizeof(EffectParam));
	Script::DeclareClassOffset("EffectParam", "name", offsetof(EffectParam, name));
	Script::DeclareClassOffset("EffectParam", "type", offsetof(EffectParam, type));
	Script::DeclareClassOffset("EffectParam", "value", offsetof(EffectParam, value));

	Script::DeclareClassSize("AudioEffect", sizeof(Effect));
	Script::DeclareClassOffset("AudioEffect", "name", offsetof(Effect, name));
	Script::DeclareClassOffset("AudioEffect", "param", offsetof(Effect, param));
	Script::DeclareClassOffset("AudioEffect", "only_on_selection", offsetof(Effect, only_on_selection));
	Script::DeclareClassOffset("AudioEffect", "range", offsetof(Effect, range));
	Script::DeclareClassOffset("AudioEffect", "plugin", offsetof(Effect, plugin));
	Script::DeclareClassOffset("AudioEffect", "state", offsetof(Effect, state));
	Script::DeclareClassOffset("AudioEffect", "usable", offsetof(Effect, usable));

	Script::DeclareClassSize("BufferBox", sizeof(BufferBox));
	Script::DeclareClassOffset("BufferBox", "offset", offsetof(BufferBox, offset));
	Script::DeclareClassOffset("BufferBox", "num", offsetof(BufferBox, num));
	Script::DeclareClassOffset("BufferBox", "r", offsetof(BufferBox, r));
	Script::DeclareClassOffset("BufferBox", "l", offsetof(BufferBox, l));
	Script::DeclareClassOffset("BufferBox", "peak", offsetof(BufferBox, peak));
	Script::LinkExternal("BufferBox.clear", Script::mf(&BufferBox::clear));
	Script::LinkExternal("BufferBox.__assign__", Script::mf(&BufferBox::__assign__));
	Script::LinkExternal("BufferBox.range", Script::mf(&BufferBox::range));
	Script::LinkExternal("BufferBox.get_spectrum", Script::mf(&ExtendedBufferBox::get_spectrum));

	Synthesizer synth;
	Script::DeclareClassSize("Synthesizer", sizeof(Synthesizer));
	Script::DeclareClassOffset("Synthesizer", "name", offsetof(Synthesizer, name));
	Script::DeclareClassOffset("Synthesizer", "sample_rate", offsetof(Synthesizer, sample_rate));
	Script::DeclareClassOffset("Synthesizer", "keep_notes", offsetof(Synthesizer, keep_notes));
	Script::LinkExternal("Synthesizer.__init__", Script::mf(&Synthesizer::__init__));
	Script::DeclareClassVirtualIndex("Synthesizer", "__delete__", Script::mf(&Synthesizer::__delete__), &synth);
	Script::DeclareClassVirtualIndex("Synthesizer", "RenderNote", Script::mf(&Synthesizer::RenderNote), &synth);
	Script::DeclareClassVirtualIndex("Synthesizer", "read", Script::mf(&Synthesizer::read), &synth);
	Script::DeclareClassVirtualIndex("Synthesizer", "OnConfigure", Script::mf(&Synthesizer::OnConfigure), &synth);
	Script::LinkExternal("Synthesizer.set", Script::mf(&Synthesizer::set));
	Script::LinkExternal("Synthesizer.RenderMetronomeClick", Script::mf(&Synthesizer::RenderMetronomeClick));


	DummySynthesizer dsynth;
	Script::DeclareClassSize("DummySynthesizer", sizeof(DummySynthesizer));
	Script::LinkExternal("DummySynthesizer.__init__", Script::mf(&DummySynthesizer::__init__));
	Script::DeclareClassVirtualIndex("DummySynthesizer", "__delete__", Script::mf(&DummySynthesizer::__delete__), &dsynth);
	Script::DeclareClassVirtualIndex("DummySynthesizer", "RenderNote", Script::mf(&DummySynthesizer::RenderNote), &dsynth);
	//Script::DeclareClassVirtualIndex("DummySynthesizer", "read", Script::mf(&DummySynthesizer::read), &dsynth);
	//Script::DeclareClassVirtualIndex("DummySynthesizer", "OnConfigure", Script::mf(&DummySynthesizer::OnConfigure), &dsynth);
	//Script::LinkExternal("DummySynthesizer.set", Script::mf(&Synthesizer::set));
	//Script::LinkExternal("DummySynthesizer.RenderMetronomeClick", Script::mf(&Synthesizer::RenderMetronomeClick));

	Script::DeclareClassSize("BarPattern", sizeof(BarPattern));
	Script::DeclareClassOffset("BarPattern", "num_beats", offsetof(BarPattern, num_beats));
	Script::DeclareClassOffset("BarPattern", "length", offsetof(BarPattern, length));
	Script::DeclareClassOffset("BarPattern", "type", offsetof(BarPattern, type));
	Script::DeclareClassOffset("BarPattern", "count", offsetof(BarPattern, count));
	Script::DeclareClassOffset("BarPattern", "is_selected", offsetof(BarPattern, is_selected));

	Script::DeclareClassSize("MidiNote", sizeof(MidiNote));
	Script::DeclareClassOffset("MidiNote", "range", offsetof(MidiNote, range));
	Script::DeclareClassOffset("MidiNote", "pitch", offsetof(MidiNote, pitch));
	Script::DeclareClassOffset("MidiNote", "volume", offsetof(MidiNote, volume));

	Script::DeclareClassSize("MidiData", sizeof(MidiData));
	Script::DeclareClassOffset("MidiData", "note", 0); //offsetof(MidiData, note));
	Script::DeclareClassOffset("MidiData", "synthesizer", offsetof(MidiData, synthesizer));
	Script::DeclareClassOffset("MidiData", "instrument", offsetof(MidiData, instrument));
	Script::DeclareClassOffset("MidiData", "options", offsetof(MidiData, options));

	Script::DeclareClassSize("TrackLevel", sizeof(TrackLevel));
	Script::DeclareClassOffset("TrackLevel", "buffer", offsetof(TrackLevel, buffer));

	Script::DeclareClassSize("Track", sizeof(Track));
	Script::DeclareClassOffset("Track", "type", offsetof(Track, type));
	Script::DeclareClassOffset("Track", "name", offsetof(Track, name));
	Script::DeclareClassOffset("Track", "level", offsetof(Track, level));
//	Script::DeclareClassOffset("Track", "length", offsetof(Track, length));
//	Script::DeclareClassOffset("Track", "pos", offsetof(Track, pos));
	Script::DeclareClassOffset("Track", "volume", offsetof(Track, volume));
	Script::DeclareClassOffset("Track", "panning", offsetof(Track, panning));
	Script::DeclareClassOffset("Track", "muted", offsetof(Track, muted));
//	Script::DeclareClassOffset("Track", "rep_num", offsetof(Track, rep_num));
//	Script::DeclareClassOffset("Track", "rep_delay", offsetof(Track, rep_delay));
	Script::DeclareClassOffset("Track", "fx", offsetof(Track, fx));
//	Script::DeclareClassOffset("Track", "sub", offsetof(Track, sub));
	Script::DeclareClassOffset("Track", "midi", offsetof(Track, midi));
	Script::DeclareClassOffset("Track", "synth", offsetof(Track, synth));
	Script::DeclareClassOffset("Track", "area", offsetof(Track, area));
//	Script::DeclareClassOffset("Track", "parent", offsetof(Track, parent));
	Script::DeclareClassOffset("Track", "root", offsetof(Track, root));
	Script::DeclareClassOffset("Track", "is_selected", offsetof(Track, is_selected));
	Script::LinkExternal("Track.GetBuffers", Script::mf(&Track::GetBuffers));
	Script::LinkExternal("Track.ReadBuffers", Script::mf(&Track::ReadBuffers));
	Script::LinkExternal("Track.InsertMidiData", Script::mf(&Track::InsertMidiData));

	Script::DeclareClassSize("AudioFile", sizeof(AudioFile));
	Script::DeclareClassOffset("AudioFile", "used", offsetof(AudioFile, used));
	Script::DeclareClassOffset("AudioFile", "filename", offsetof(AudioFile, filename));
	Script::DeclareClassOffset("AudioFile", "tag", offsetof(AudioFile, tag));
	Script::DeclareClassOffset("AudioFile", "sample_rate", offsetof(AudioFile, sample_rate));
	Script::DeclareClassOffset("AudioFile", "volume", offsetof(AudioFile, volume));
	Script::DeclareClassOffset("AudioFile", "fx", offsetof(AudioFile, fx));
	Script::DeclareClassOffset("AudioFile", "track", offsetof(AudioFile, track));
	Script::DeclareClassOffset("AudioFile", "area", offsetof(AudioFile, area));
	Script::DeclareClassOffset("AudioFile", "selection", offsetof(AudioFile, selection));
	Script::DeclareClassOffset("AudioFile", "sel_raw", offsetof(AudioFile, sel_raw));
	Script::DeclareClassOffset("AudioFile", "level_name", offsetof(AudioFile, level_name));
	Script::LinkExternal("AudioFile.GetRange", Script::mf(&AudioFile::GetRange));
	Script::LinkExternal("AudioFile.GetNextBeat", Script::mf(&AudioFile::GetNextBeat));

	Script::LinkExternal("AudioRenderer.Prepare", Script::mf(&AudioRenderer::Prepare));
	Script::LinkExternal("AudioRenderer.CleanUp", Script::mf(&AudioRenderer::CleanUp));
	//Script::LinkExternal("AudioRenderer.read", Script::mf(&AudioRenderer::read));
	Script::LinkExternal("AudioRenderer.RenderAudioFile", Script::mf(&AudioRenderer::RenderAudioFile));
	Script::LinkExternal("AudioRenderer.TranslateOutputPos", Script::mf(&AudioRenderer::TranslateOutputPos));

	Script::DeclareClassSize("AudioInput", sizeof(AudioInput));
	Script::DeclareClassOffset("AudioInput", "cur_buf", offsetof(AudioInput, current_buffer));
	Script::DeclareClassOffset("AudioInput", "buf", offsetof(AudioInput, buffer));
	Script::DeclareClassOffset("AudioInput", "midi", offsetof(AudioInput, midi));
	Script::LinkExternal("AudioInput.Start", Script::mf(&AudioInput::Start));
	Script::LinkExternal("AudioInput.ResetSync", Script::mf(&AudioInput::ResetSync));
	Script::LinkExternal("AudioInput.Stop",	 Script::mf(&AudioInput::Stop));
	Script::LinkExternal("AudioInput.IsCapturing", Script::mf(&AudioInput::IsCapturing));
	Script::LinkExternal("AudioInput.GetSampleCount", Script::mf(&AudioInput::GetSampleCount));
	Script::LinkExternal("AudioInput.Accumulate", Script::mf(&AudioInput::Accumulate));
	Script::LinkExternal("AudioInput.ResetAccumulation", Script::mf(&AudioInput::ResetAccumulation));
	Script::LinkExternal("AudioInput.GetDelay", Script::mf(&AudioInput::GetDelay));
	Script::LinkExternal("AudioInput.AddObserver", Script::mf(&AudioInput::AddWrappedObserver));
	Script::LinkExternal("AudioInput.RemoveObserver", Script::mf(&AudioInput::RemoveWrappedObserver));
	//Script::LinkExternal("Observable.AddObserver", Script::mf(&Observable::AddWrappedObserver);

	Script::LinkExternal("AudioOutput.Play", Script::mf(&AudioOutput::Play));
	Script::LinkExternal("AudioOutput.PlayGenerated", Script::mf(&AudioOutput::PlayGenerated));
	Script::LinkExternal("AudioOutput.Stop", Script::mf(&AudioOutput::Stop));
	Script::LinkExternal("AudioOutput.IsPlaying", Script::mf(&AudioOutput::IsPlaying));
	Script::LinkExternal("AudioOutput.GetPos", Script::mf(&AudioOutput::GetPos));
	Script::LinkExternal("AudioOutput.GetSampleRate", Script::mf(&AudioOutput::GetSampleRate));
	Script::LinkExternal("AudioOutput.GetVolume", Script::mf(&AudioOutput::GetVolume));
	Script::LinkExternal("AudioOutput.SetVolume", Script::mf(&AudioOutput::SetVolume));
	Script::LinkExternal("AudioOutput.SetBufferSize", Script::mf(&AudioOutput::SetBufferSize));

	Script::LinkExternal("Log.Error", Script::mf(&Log::Error));
	Script::LinkExternal("Log.Warning", Script::mf(&Log::Warning));
	Script::LinkExternal("Log.Info", Script::mf(&Log::Info));

	Script::LinkExternal("Storage.Load", Script::mf(&Storage::Load));
	Script::LinkExternal("Storage.Save", Script::mf(&Storage::Save));

	Script::DeclareClassSize("PluginContext", sizeof(PluginManager::PluginContext));
	Script::DeclareClassOffset("PluginContext", "track", offsetof(PluginManager::PluginContext, track));
	Script::DeclareClassOffset("PluginContext", "track_no", offsetof(PluginManager::PluginContext, track_no));
	Script::DeclareClassOffset("PluginContext", "range", offsetof(PluginManager::PluginContext, range));
	Script::DeclareClassOffset("PluginContext", "level", offsetof(PluginManager::PluginContext, level));
	Script::LinkExternal("plugin_context",	(void*)&tsunami->plugin_manager->context);
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

void find_plugins_in_dir(const string &dir, PluginManager *pm, HuiMenu *m)
{
	Array<DirEntry> list = dir_search(HuiAppDirectoryStatic + "Plugins/" + dir, "*.kaba", false);
	foreach(DirEntry &e, list){
		PluginManager::PluginFile pf;
		pf.name = e.name.replace(".kaba", "");
		pf.filename = HuiAppDirectoryStatic + "Plugins/" + dir + e.name;
		get_plugin_file_data(pf);
		m->AddItemImage(pf.name, pf.image, format("execute_plugin_%d", pm->plugin_file.num));
		pm->plugin_file.add(pf);
	}
}

void PluginManager::AddPluginsToMenu()
{
	msg_db_f("AddPluginsToMenu", 2);
	Script::Init();

	HuiMenu *m = tsunami->GetMenu();

	// "Buffer"
	find_plugins_in_dir("Buffer/Channels/", this, m->GetSubMenuByID("menu_plugins_channels"));
	find_plugins_in_dir("Buffer/Dynamics/", this, m->GetSubMenuByID("menu_plugins_dynamics"));
	find_plugins_in_dir("Buffer/Echo/", this, m->GetSubMenuByID("menu_plugins_echo"));
	find_plugins_in_dir("Buffer/Pitch/", this, m->GetSubMenuByID("menu_plugins_pitch"));
	find_plugins_in_dir("Buffer/Sound/", this, m->GetSubMenuByID("menu_plugins_sound"));
	find_plugins_in_dir("Buffer/Synthesizer/", this, m->GetSubMenuByID("menu_plugins_synthesizer"));

	// "All"
	find_plugins_in_dir("All/", this, m->GetSubMenuByID("menu_plugins_on_audio"));

	// rest
	find_plugins_in_dir("Independent/", this, m->GetSubMenuByID("menu_plugins_other"));

	// Events
	for (int i=0;i<plugin_file.num;i++)
		tsunami->EventM(format("execute_plugin_%d", i), this, (void(HuiEventHandler::*)())&PluginManager::OnMenuExecutePlugin);
}

void PluginManager::InitPluginData()
{
	msg_db_f("InitPluginData", 2);
}

void PluginManager::FinishPluginData()
{
	msg_db_f("FinishPluginData", 2);
	//tsunami->view->ForceRedraw();
}

void PluginManager::OnFavoriteName()
{
	bool enabled = HuiCurWindow->GetString("").num > 0;
	HuiCurWindow->Enable("favorite_save", enabled);
	HuiCurWindow->Enable("favorite_delete", enabled);
}

void PluginManager::OnFavoriteList()
{
	int n = HuiCurWindow->GetInt("");
	cur_plugin->ResetData();
	if (n == 0){
		HuiCurWindow->SetString("favorite_name", "");
		HuiCurWindow->Enable("favorite_save", false);
		HuiCurWindow->Enable("favorite_delete", false);
	}else{
		cur_plugin->LoadDataFromFile(PluginFavoriteName[n - 1]);
		HuiCurWindow->SetString("favorite_name", PluginFavoriteName[n - 1]);
		HuiCurWindow->Enable("favorite_delete", true);
	}
	cur_plugin->DataToDialog();
}

void PluginManager::OnFavoriteSave()
{
	string name = HuiCurWindow->GetString("favorite_name");
	cur_plugin->WriteDataToFile(name);
	PluginFavoriteName.add(name);
	HuiCurWindow->AddString("favorite_list", name);
	HuiCurWindow->SetInt("favorite_list", PluginFavoriteName.num);
}

void PluginManager::OnFavoriteDelete()
{}

void PluginManager::InitFavorites(HuiWindow *win)
{
	msg_db_f("InitFavorites", 1);
	PluginFavoriteName.clear();


	win->Enable("favorite_save", false);
	win->Enable("favorite_delete", false);

	string init = cur_plugin->s->Filename.basename() + "___";

	dir_create(HuiAppDirectory + "Plugins/Favorites");
	Array<DirEntry> list = dir_search(HuiAppDirectory + "Plugins/Favorites", "*", false);
	foreach(DirEntry &e, list){
		if (e.name.find(init) < 0)
			continue;
		PluginFavoriteName.add(e.name.substr(init.num, -1));
		win->AddString("favorite_list", PluginFavoriteName.back());
	}

	win->EventM("favorite_name", this, (void(HuiEventHandler::*)())&PluginManager::OnFavoriteName);
	win->EventM("favorite_save", this, (void(HuiEventHandler::*)())&PluginManager::OnFavoriteSave);
	win->EventM("favorite_delete", this, (void(HuiEventHandler::*)())&PluginManager::OnFavoriteDelete);
	win->EventM("favorite_list", this, (void(HuiEventHandler::*)())&PluginManager::OnFavoriteList);
}

void PluginManager::PutFavoriteBarFixed(HuiWindow *win, int x, int y, int w)
{
	msg_db_f("PutFavoriteBarFixed", 1);
	w -= 10;
	win->AddComboBox("", x, y, w / 2 - 35, 25, "favorite_list");
	win->AddEdit("", x + w / 2 - 30, y, w / 2 - 30, 25, "favorite_name");
	win->AddButton("", x + w - 55, y, 25, 25, "favorite_save");
	win->SetImage("favorite_save", "hui:save");
	win->AddButton("", x + w - 25, y, 25, 25, "favorite_delete");
	win->SetImage("favorite_delete", "hui:delete");

	InitFavorites(win);
}

void PluginManager::PutFavoriteBarSizable(HuiWindow *win, const string &root_id, int x, int y)
{
	msg_db_f("PutFavoriteBarSizable", 1);
	win->SetTarget(root_id, 0);
	win->AddControlTable("!noexpandy", x, y, 4, 1, "favorite_table");
	win->SetTarget("favorite_table", 0);
	win->AddComboBox("", 0, 0, 0, 0, "favorite_list");
	win->AddEdit("", 1, 0, 0, 0, "favorite_name");
	win->AddButton("", 2, 0, 0, 0, "favorite_save");
	win->SetImage("favorite_save", "hui:save");
	win->AddButton("", 3, 0, 0, 0, "favorite_delete");
	win->SetImage("favorite_delete", "hui:delete");

	InitFavorites(win);
}

void PluginManager::OnPluginFavoriteName()
{
	HuiWindow *win = HuiGetEvent()->win;
	win->Enable("favorite_save", win->GetString("favorite_name").num > 0);
	win->Enable("favorite_delete", win->GetString("favorite_name").num > 0);
}

void PluginManager::OnPluginFavoriteList()
{
	HuiWindow *win = HuiGetEvent()->win;
	int n = win->GetInt("");
	cur_plugin->ResetData();
	if (n == 0){
		win->SetString("favorite_name", "");
		win->Enable("favorite_save", false);
		win->Enable("favorite_delete", false);
	}else{
		cur_plugin->LoadDataFromFile(PluginFavoriteName[n - 1]);
		win->SetString("favorite_name", PluginFavoriteName[n - 1]);
		win->Enable("favorite_delete", true);
	}
	cur_plugin->DataToDialog();
}

void PluginManager::OnPluginFavoriteSave()
{
	HuiWindow *win = HuiGetEvent()->win;
	cur_plugin->WriteDataToFile(win->GetString("favorite_name"));
	PluginFavoriteName.add(win->GetString("favorite_name"));
	win->AddString("favorite_list", win->GetString("favorite_name"));
	win->SetInt("favorite_list", PluginFavoriteName.num);
}

void PluginManager::OnPluginOk()
{
	PluginCancelled = false;
	delete(HuiCurWindow);
}

void PluginManager::OnPluginClose()
{
	PluginCancelled = true;
	delete(HuiCurWindow);
}

void PluginManager::PutCommandBarFixed(HuiWindow *win, int x, int y, int w)
{
	msg_db_f("PutCommandBarFixed", 1);
	w -= 10;
	int ww = (w - 30) / 3;
	if (ww > 120)
		ww = 120;

	win->AddDefButton(_("OK"),w - ww,y,ww,25,"ok");
	//win->SetImage("ok", "hui:ok");
	win->AddButton(_("Abbrechen"),w - ww*2 - 10,y,ww,25,"cancel");
	//win->SetImage("cancel", "hui:cancel");

	if (PluginAddPreview){
		if (cur_plugin->type == Plugin::TYPE_EFFECT){
			win->AddButton(_("Vorschau"),w - ww * 3 - 20,y,ww,25,"preview");
			win->SetImage("preview", "hui:media-play");
		}
	}
	win->EventM("ok", this, (void(HuiEventHandler::*)())&PluginManager::OnPluginOk);
	win->EventM("preview", this, (void(HuiEventHandler::*)())&PluginManager::OnPluginPreview);
	win->EventM("cancel", this, (void(HuiEventHandler::*)())&PluginManager::OnPluginClose);
	win->EventM("hui:close", this, (void(HuiEventHandler::*)())&PluginManager::OnPluginClose);
}

void PluginManager::PutCommandBarSizable(HuiWindow *win, const string &root_id, int x, int y)
{
	msg_db_f("PutCommandBarSizable", 1);
	win->SetTarget(root_id, 0);
	win->AddControlTable("!noexpandy", x, y, 4, 1, "command_table");
	win->SetTarget("command_table", 0);
	win->AddDefButton(_("OK"), 3, 0, 0, 0, "ok");
	win->SetImage("ok", "hui:ok");
	win->AddButton(_("Abbrechen"), 2, 0, 0, 0, "cancel");
	win->SetImage("cancel", "hui:cancel");
	win->AddText("", 1, 0, 0, 0, "");
	if (PluginAddPreview){
		if (cur_plugin->type == Plugin::TYPE_EFFECT){
			win->AddButton(_("Vorschau"), 0, 0, 0, 0, "preview");
			win->SetImage("preview", "hui:media-play");
		}
	}
	win->EventM("ok", this, (void(HuiEventHandler::*)())&PluginManager::OnPluginOk);
	win->EventM("preview", this, (void(HuiEventHandler::*)())&PluginManager::OnPluginPreview);
	win->EventM("cancel", this, (void(HuiEventHandler::*)())&PluginManager::OnPluginClose);
	win->EventM("hui:close", this, (void(HuiEventHandler::*)())&PluginManager::OnPluginClose);
}

void PluginManager::OnPluginPreview()
{
	Effect fx(cur_plugin);
	Preview(fx);
}

void PluginManager::OnUpdate(Observable *o)
{
	if (o == tsunami->progress){
		if (o->GetMessage() == "Cancel")
			tsunami->output->Stop();
	}/*else if (o == tsunami->output){
		int pos = tsunami->output->GetPos();
		tsunami->progress->Set(_("Vorschau"), (float)(pos - tsunami->output->GetRange().start()) / tsunami->output->GetRange().length());
	}*/ // TODO
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
		Script::Script *s = cur_plugin->s;

		AudioFile *a = tsunami->audio;

		// run
		cur_plugin->ResetData();
		if (cur_plugin->Configure(true)){
			main_audiofile_func *f_audio = (main_audiofile_func*)s->MatchFunction("main", "void", 1, "AudioFile*");
			main_void_func *f_void = (main_void_func*)s->MatchFunction("main", "void", 0);
			if (cur_plugin->type == Plugin::TYPE_EFFECT){
				if (a->used){
					cur_plugin->ResetState();
					Range range = a->selection;
					if (range.empty())
						range = a->GetRange();
					a->action_manager->BeginActionGroup();
					foreach(Track *t, a->track)
						if ((t->is_selected) && (t->type == t->TYPE_AUDIO)){
							cur_plugin->ProcessTrack(t, tsunami->view->cur_level, range);
						}
					a->action_manager->EndActionGroup();
				}else{
					tsunami->log->Error(_("Plugin kann nicht f&ur eine leere Audiodatei ausgef&uhrt werden"));
				}
			}else if (f_audio){
				if (a->used)
					f_audio(a);
				else
					tsunami->log->Error(_("Plugin kann nicht f&ur eine leere Audiodatei ausgef&uhrt werden"));
			}else if (f_void){

				f_void();
			}
		}

		// data changed?
		FinishPluginData();
	}else{
		tsunami->log->Error(cur_plugin->GetError());
	}
}


void PluginManager::FindAndExecutePlugin()
{
	msg_db_f("ExecutePlugin", 1);


	if (HuiFileDialogOpen(tsunami, _("Plugin-Script w&ahlen"), HuiAppDirectoryStatic + "Plugins/", _("Script (*.kaba)"), "*.kaba")){
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


void PluginManager::Preview(Effect &fx)
{
	fx.ExportData();
	tsunami->renderer->effect = &fx;


	tsunami->progress->StartCancelable(_("Vorschau"), 0);
	Subscribe(tsunami->progress);
	Subscribe(tsunami->output);
	tsunami->renderer->Prepare(tsunami->audio, tsunami->audio->selection, false);
	tsunami->output->Play(tsunami->renderer);

	while(tsunami->output->IsPlaying()){
		HuiSleep(10);
		HuiDoSingleMainLoop();
	}
	Unsubscribe(tsunami->output);
	Unsubscribe(tsunami->progress);
	tsunami->progress->End();


	tsunami->renderer->effect = NULL;
	fx.ImportData();
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
		Script::Type *r = t;
		while (r->parent)
			r = r->parent;
		if (r->name != "Synthesizer")
			continue;
		return (Synthesizer*)t->CreateInstance();
	}
	return NULL;
}

