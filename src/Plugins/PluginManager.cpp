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
#include <typeinfo>


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
	cur_synth = NULL;
}

PluginManager::~PluginManager()
{
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
	Script::LinkExternal("view", &tsunami->view);
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

	Script::DeclareClassSize("Range", sizeof(Range));
	Script::DeclareClassOffset("Range", "offset", offsetof(Range, offset));
	Script::DeclareClassOffset("Range", "length", offsetof(Range, num));


	PluginData plugin_data;
	Script::DeclareClassSize("PluginData", sizeof(PluginData));
	Script::LinkExternal("PluginData.__init__", Script::mf(&PluginData::__init__));
	Script::DeclareClassVirtualIndex("PluginData", "__delete__", Script::mf(&PluginData::__delete__), &plugin_data);
	Script::DeclareClassVirtualIndex("PluginData", "reset", Script::mf(&PluginData::reset), &plugin_data);


	Effect effect;
	Script::DeclareClassSize("AudioEffect", sizeof(Effect));
	Script::DeclareClassOffset("AudioEffect", "name", offsetof(Effect, name));
	Script::DeclareClassOffset("AudioEffect", "only_on_selection", offsetof(Effect, only_on_selection));
	Script::DeclareClassOffset("AudioEffect", "range", offsetof(Effect, range));
	Script::DeclareClassOffset("AudioEffect", "usable", offsetof(Effect, usable));
	Script::LinkExternal("AudioEffect.__init__", Script::mf(&Effect::__init__));
	Script::DeclareClassVirtualIndex("AudioEffect", "__delete__", Script::mf(&Effect::__delete__), &effect);
	Script::DeclareClassVirtualIndex("AudioEffect", "processTrack", Script::mf(&Effect::ProcessTrack), &effect);
	Script::DeclareClassVirtualIndex("AudioEffect", "createPanel", Script::mf(&Effect::CreatePanel), &effect);
	Script::DeclareClassVirtualIndex("AudioEffect", "resetConfig", Script::mf(&Effect::ResetConfig), &effect);
	Script::DeclareClassVirtualIndex("AudioEffect", "resetState", Script::mf(&Effect::ResetState), &effect);
	Script::DeclareClassVirtualIndex("AudioEffect", "updateDialog", Script::mf(&Effect::UpdateDialog), &effect);
	Script::LinkExternal("AudioEffect.notify", Script::mf(&Effect::notify));

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
	Script::DeclareClassVirtualIndex("Synthesizer", "renderNote", Script::mf(&Synthesizer::RenderNote), &synth);
	Script::DeclareClassVirtualIndex("Synthesizer", "read", Script::mf(&Synthesizer::read), &synth);
	Script::DeclareClassVirtualIndex("Synthesizer", "createPanel", Script::mf(&Synthesizer::CreatePanel), &synth);
	Script::DeclareClassVirtualIndex("Synthesizer", "updateDialog", Script::mf(&Synthesizer::UpdateDialog), &synth);
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
	Script::LinkExternal("Track.getBuffers", Script::mf(&Track::GetBuffers));
	Script::LinkExternal("Track.readBuffers", Script::mf(&Track::ReadBuffers));
	Script::LinkExternal("Track.insertMidiData", Script::mf(&Track::InsertMidiData));

	Script::DeclareClassSize("AudioFile", sizeof(AudioFile));
	Script::DeclareClassOffset("AudioFile", "used", offsetof(AudioFile, used));
	Script::DeclareClassOffset("AudioFile", "filename", offsetof(AudioFile, filename));
	Script::DeclareClassOffset("AudioFile", "tag", offsetof(AudioFile, tag));
	Script::DeclareClassOffset("AudioFile", "sample_rate", offsetof(AudioFile, sample_rate));
	Script::DeclareClassOffset("AudioFile", "volume", offsetof(AudioFile, volume));
	Script::DeclareClassOffset("AudioFile", "fx", offsetof(AudioFile, fx));
	Script::DeclareClassOffset("AudioFile", "track", offsetof(AudioFile, track));
	Script::DeclareClassOffset("AudioFile", "area", offsetof(AudioFile, area));
	Script::DeclareClassOffset("AudioFile", "level_name", offsetof(AudioFile, level_name));
	Script::LinkExternal("AudioFile.getRange", Script::mf(&AudioFile::GetRange));
	Script::LinkExternal("AudioFile.getNextBeat", Script::mf(&AudioFile::GetNextBeat));

	Script::LinkExternal("AudioRenderer.prepare", Script::mf(&AudioRenderer::Prepare));
	//Script::LinkExternal("AudioRenderer.read", Script::mf(&AudioRenderer::read));
	Script::LinkExternal("AudioRenderer.renderAudioFile", Script::mf(&AudioRenderer::RenderAudioFile));

	Script::DeclareClassSize("AudioInput", sizeof(AudioInput));
	Script::DeclareClassOffset("AudioInput", "cur_buf", offsetof(AudioInput, current_buffer));
	Script::DeclareClassOffset("AudioInput", "buf", offsetof(AudioInput, buffer));
	Script::DeclareClassOffset("AudioInput", "midi", offsetof(AudioInput, midi));
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
	Script::DeclareClassOffset("AudioView", "sel_range", offsetof(AudioView, sel_range));
	Script::DeclareClassOffset("AudioView", "sel_raw", offsetof(AudioView, sel_raw));

	Script::LinkExternal("Log.error", Script::mf(&Log::Error));
	Script::LinkExternal("Log.warning", Script::mf(&Log::Warning));
	Script::LinkExternal("Log.info", Script::mf(&Log::Info));

	Script::LinkExternal("Storage.load", Script::mf(&Storage::Load));
	Script::LinkExternal("Storage.save", Script::mf(&Storage::Save));

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
		tsunami->EventM(format("execute_plugin_%d", i), this, &PluginManager::OnMenuExecutePlugin);
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
	if (n == 0){
		get_configurable()->ResetConfig();
		HuiCurWindow->SetString("favorite_name", "");
		HuiCurWindow->Enable("favorite_save", false);
		HuiCurWindow->Enable("favorite_delete", false);
	}else{
		ApplyFavorite(get_configurable(), PluginFavoriteName[n - 1]);
		HuiCurWindow->SetString("favorite_name", PluginFavoriteName[n - 1]);
		HuiCurWindow->Enable("favorite_delete", true);
	}
	get_configurable()->UpdateDialog();
}

void PluginManager::OnFavoriteSave()
{
	string name = HuiCurWindow->GetString("favorite_name");
	SaveFavorite(get_configurable(), name);
	PluginFavoriteName.add(name);
	HuiCurWindow->AddString("favorite_list", name);
	HuiCurWindow->SetInt("favorite_list", PluginFavoriteName.num);
}

void PluginManager::OnFavoriteDelete()
{}

void PluginManager::InitFavorites(HuiPanel *panel)
{
	msg_db_f("InitFavorites", 1);
	PluginFavoriteName = GetFavoriteList(get_configurable());


	panel->Enable("favorite_save", false);
	panel->Enable("favorite_delete", false);
	foreach(string &n, PluginFavoriteName)
		panel->AddString("favorite_list", n);

	panel->EventM("favorite_name", this, &PluginManager::OnFavoriteName);
	panel->EventM("favorite_save", this, &PluginManager::OnFavoriteSave);
	panel->EventM("favorite_delete", this, &PluginManager::OnFavoriteDelete);
	panel->EventM("favorite_list", this, &PluginManager::OnFavoriteList);
}

Configurable *PluginManager::get_configurable()
{
	return cur_synth ? (Configurable*)cur_synth : (Configurable*)cur_effect;
}

string get_fav_dir(Configurable *c)
{
	dir_create(HuiAppDirectory + "Favorites");
	if (c->configurable_type == CONFIGURABLE_SYNTHESIZER){
		dir_create(HuiAppDirectory + "Favorites/Synthesizer");
		return HuiAppDirectory + "Favorites/Synthesizer";
	}
	dir_create(HuiAppDirectory + "Favorites/Effect");
	return HuiAppDirectory + "Favorites/Effect";
}

Array<string> PluginManager::GetFavoriteList(Configurable *c)
{
	Array<string> names;

	string init = c->name + "___";

	string dir = get_fav_dir(c);
	Array<DirEntry> list = dir_search(dir, "*", false);
	foreach(DirEntry &e, list){
		if (e.name.find(init) < 0)
			continue;
		names.add(e.name.substr(init.num, -1));
	}
	return names;
}

void PluginManager::ApplyFavorite(Configurable *c, const string &name)
{
	c->ResetConfig();
	if (name == ":def:")
		return;
	msg_db_f("ApplyFavorite", 1);
	string dir = get_fav_dir(c);
	CFile *f = FileOpen(dir + "/" + c->name + "___" + name);
	if (!f)
		return;
	c->ConfigFromString(f->ReadStr());
	delete(f);
}

void PluginManager::SaveFavorite(Configurable *c, const string &name)
{
	msg_db_f("SaveFavorite", 1);
	string dir = get_fav_dir(c);
	CFile *f = FileCreate(dir + "/" + c->name + "___" + name);
	f->WriteStr(c->ConfigToString());
	delete(f);
}

static string FavoriteSelectionDialogReturn;

class FavoriteSelectionDialog : public HuiDialog
{
public:
	FavoriteSelectionDialog(HuiWindow *win, const Array<string> &_names, bool _save) :
		HuiDialog(_(""), 300, 200, win, false)
	{
		save = _save;
		FavoriteSelectionDialogReturn = "";
		names = _names;
		AddControlTable("", 0, 0, 1, 2, "grid");
		SetTarget("grid", 0);
		AddListView("Name", 0, 0, 0, 0, "list");
		AddControlTable("", 0, 1, 2, 1, "grid2");
		SetTarget("grid2", 0);
		AddEdit("", 0, 0, 0, 0, "name");
		AddDefButton("Ok", 1, 0, 0, 0, "ok");
		if (!save)
			AddString("list", _("-Standard Parameter-"));
		foreach(string &n, names)
			AddString("list", n);
		if (!save)
			names.insert(":def:", 0);
		HideControl("grid2", !save);
		EventM("list", this, &FavoriteSelectionDialog::OnList);
		EventMX("list", "hui:select", this, &FavoriteSelectionDialog::OnListSelect);
		EventM("ok", this, &FavoriteSelectionDialog::OnOk);
	}
	void OnList()
	{
		int n = GetInt("list");
		FavoriteSelectionDialogReturn = "";
		if (n >= 0){
			FavoriteSelectionDialogReturn = names[n];
			SetString("name", names[n]);
		}
		delete(this);
	}
	void OnListSelect()
	{
		int n = GetInt("list");
		if (n >= 0)
			SetString("name", names[n]);
	}
	void OnOk()
	{
		FavoriteSelectionDialogReturn = GetString("name");
		delete(this);
	}

	bool save;
	Array<string> names;
};

string PluginManager::SelectFavoriteName(HuiWindow *win, Configurable *c, bool save)
{
	FavoriteSelectionDialog *dlg = new FavoriteSelectionDialog(win, GetFavoriteList(c), save);
	dlg->Run();
	return FavoriteSelectionDialogReturn;
}

void PluginManager::PutFavoriteBarSizable(HuiPanel *panel, const string &root_id, int x, int y)
{
	msg_db_f("PutFavoriteBarSizable", 1);
	panel->SetTarget(root_id, 0);
	panel->AddControlTable("", x, y, 4, 1, "favorite_table");
	panel->SetTarget("favorite_table", 0);
	panel->AddComboBox("", 0, 0, 0, 0, "favorite_list");
	panel->AddEdit("!expandx", 1, 0, 0, 0, "favorite_name");
	panel->AddButton("", 2, 0, 0, 0, "favorite_save");
	panel->SetImage("favorite_save", "hui:save");
	panel->AddButton("", 3, 0, 0, 0, "favorite_delete");
	panel->SetImage("favorite_delete", "hui:delete");

	InitFavorites(panel);
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
	if (n == 0){
		get_configurable()->ResetConfig();
		win->SetString("favorite_name", "");
		win->Enable("favorite_save", false);
		win->Enable("favorite_delete", false);
	}else{
		ApplyFavorite(get_configurable(), PluginFavoriteName[n - 1]);
		win->SetString("favorite_name", PluginFavoriteName[n - 1]);
		win->Enable("favorite_delete", true);
	}
	get_configurable()->UpdateDialog();
}

void PluginManager::OnPluginFavoriteSave()
{
	HuiWindow *win = HuiGetEvent()->win;
	SaveFavorite(get_configurable(), win->GetString("favorite_name"));
	PluginFavoriteName.add(win->GetString("favorite_name"));
	win->AddString("favorite_list", win->GetString("favorite_name"));
	win->SetInt("favorite_list", PluginFavoriteName.num);
}

void PluginManager::OnPluginOk()
{
	PluginCancelled = false;
	cur_effect = NULL;
	cur_plugin = NULL;
	cur_synth = NULL;
	delete(HuiCurWindow);
}

void PluginManager::OnPluginClose()
{
	PluginCancelled = true;
	cur_effect = NULL;
	cur_plugin = NULL;
	cur_synth = NULL;
	delete(HuiCurWindow);
}

void PluginManager::PutCommandBarSizable(HuiPanel *panel, const string &root_id, int x, int y)
{
	msg_db_f("PutCommandBarSizable", 1);
	panel->SetTarget(root_id, 0);
	panel->AddControlTable("!buttonbar", x, y, 4, 1, "command_table");
	panel->SetTarget("command_table", 0);
	if (PluginAddPreview){
		if (cur_effect){
			panel->AddButton(_("Vorschau"), 0, 0, 0, 0, "preview");
			panel->SetImage("preview", "hui:media-play");
		}
	}else if (cur_synth){
		panel->AddButton(_("Vorschau"), 0, 0, 0, 0, "preview");
		panel->SetImage("preview", "hui:media-play");
	}
	panel->AddText("!width=30", 1, 0, 0, 0, "");
	panel->AddButton(_("Abbrechen"), 2, 0, 0, 0, "cancel");
	panel->SetImage("cancel", "hui:cancel");
	panel->AddDefButton(_("OK"), 3, 0, 0, 0, "ok");
	panel->SetImage("ok", "hui:ok");
	panel->EventM("ok", this, &PluginManager::OnPluginOk);
	panel->EventM("preview", this, &PluginManager::OnPluginPreview);
	panel->EventM("cancel", this, &PluginManager::OnPluginClose);
	panel->EventM("hui:close", this, &PluginManager::OnPluginClose);
}

void PluginManager::OnPluginPreview()
{
	PreviewStart(cur_effect);
}

bool PluginManager::ConfigureSynthesizer(Synthesizer *s)
{
	string params_old = s->ConfigToString();

	PluginAddPreview = false;
	cur_plugin = NULL;
	cur_synth = s;
	s->Configure();
	if (PluginCancelled)
		s->ConfigFromString(params_old);
	return !PluginCancelled;
}

void PluginManager::OnUpdate(Observable *o, const string &message)
{
	if (o == tsunami->progress){
		if (message == "Cancel")
			PreviewEnd();
	}else if (o == tsunami->output){
		int pos = tsunami->output->GetPos();
		Range r = tsunami->view->sel_range;
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
		foreach(Script::Type *t, s->syntax->Types){
			Script::Type *r = t;
			while (r->parent)
				r = r->parent;
			if (r->name != "AudioEffect")
				continue;
			fx = (Effect*)t->CreateInstance();
			fx->name = p->filename.basename();
			fx->name = fx->name.head(fx->name.num - 5);
			break;
		}
		main_void_func *f_main = (main_void_func*)s->MatchFunction("main", "void", 0);

		// run
		if (fx){
			fx->ResetConfig();
			if (fx->DoConfigure(true)){
				main_audiofile_func *f_audio = (main_audiofile_func*)s->MatchFunction("main", "void", 1, "AudioFile*");
				main_void_func *f_void = (main_void_func*)s->MatchFunction("main", "void", 0);
				if (a->used){
					Range range = tsunami->view->GetPlaybackSelection();
					a->action_manager->BeginActionGroup();
					foreach(Track *t, a->track)
						if ((t->is_selected) && (t->type == t->TYPE_AUDIO)){
							fx->ResetState();
							fx->DoProcessTrack(t, tsunami->view->cur_level, range);
						}
					a->action_manager->EndActionGroup();
				}else{
					tsunami->log->Error(_("Plugin kann nicht f&ur eine leere Audiodatei ausgef&uhrt werden"));
				}
			}
			delete(fx);
		}/*else if (f_audio){
			if (a->used)
				f_audio(a);
			else
				tsunami->log->Error(_("Plugin kann nicht f&ur eine leere Audiodatei ausgef&uhrt werden"));
		}*/else if (f_main){
			f_main();
		}else{
			tsunami->log->Error(_("Plugin ist kein Effekt und enth&alt keine Funktion 'void main()'"));
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


void PluginManager::PreviewStart(Effect *fx)
{
	if (fx)
		fx->ConfigToString();
	tsunami->renderer->effect = fx;


	tsunami->progress->StartCancelable(_("Vorschau"), 0);
	Subscribe(tsunami->progress);
	Subscribe(tsunami->output);
	tsunami->renderer->Prepare(tsunami->audio, tsunami->view->sel_range, false);
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

