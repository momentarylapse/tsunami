/*
 * Tsunami.cpp
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#include "lib/hui/hui.h"
#include "Tsunami.h"
#include "Data/AudioFile.h"
#include "View/Dialog/NewDialog.h"
#include "View/Dialog/CaptureDialog.h"
#include "View/Dialog/SettingsDialog.h"
#include "View/BottomBar/BottomBar.h"
#include "View/SideBar/SideBar.h"
#include "View/Helper/Slider.h"
#include "View/Helper/Progress.h"
#include "View/Helper/PeakMeter.h"
#include "View/AudioView.h"
#include "Plugins/PluginManager.h"
#include "Storage/Storage.h"
#include "Stuff/Log.h"
#include "Stuff/Clipboard.h"
#include "Audio/AudioOutput.h"
#include "Audio/AudioInput.h"
#include "Audio/AudioRenderer.h"

#include "Plugins/FastFourierTransform.h"

Tsunami *tsunami = NULL;
extern string AppName;
extern string AppVersion;

HuiTimer debug_timer;

Tsunami::Tsunami(Array<string> arg) :
	Observer("Tsunami"),
	HuiWindow(AppName, -1, -1, 800, 600, NULL, false, HuiWinModeResizable | HuiWinModeControls)
{
	tsunami = this;

	progress = new Progress;
	log = new Log;

	clipboard = new Clipboard;

	output = new AudioOutput;
	input = new AudioInput;
	renderer = new AudioRenderer;


	int width = HuiConfig.getInt("Window.Width", 800);
	int height = HuiConfig.getInt("Window.Height", 600);
	bool maximized = HuiConfig.getBool("Window.Maximized", true);

	//HuiAddKeyCode("insert_added", KEY_RETURN);
	//HuiAddKeyCode("remove_added", KEY_BACKSPACE);

	HuiAddCommandM("new", "hui:new", KEY_N + KEY_CONTROL, this, &Tsunami::OnNew);
	HuiAddCommandM("open", "hui:open", KEY_O + KEY_CONTROL, this, &Tsunami::OnOpen);
	HuiAddCommandM("save", "hui:save", KEY_S + KEY_CONTROL, this, &Tsunami::OnSave);
	HuiAddCommandM("save_as", "hui:save-as", KEY_S + KEY_CONTROL + KEY_SHIFT, this, &Tsunami::OnSaveAs);
	HuiAddCommandM("copy", "hui:copy", KEY_C + KEY_CONTROL, this, &Tsunami::OnCopy);
	HuiAddCommandM("paste", "hui:paste", KEY_V + KEY_CONTROL, this, &Tsunami::OnPaste);
	HuiAddCommandM("delete", "hui:delete", -1, this, &Tsunami::OnDelete);
	HuiAddCommandM("export_selection", "", KEY_X + KEY_CONTROL, this, &Tsunami::OnExport);
	HuiAddCommandM("undo", "hui:undo", KEY_Z + KEY_CONTROL, this, &Tsunami::OnUndo);
	HuiAddCommandM("redo", "hui:redo", KEY_Y + KEY_CONTROL, this, &Tsunami::OnRedo);
	HuiAddCommandM("add_track", "hui:add", -1, this, &Tsunami::OnAddTrack);
	HuiAddCommandM("add_time_track", "hui:add", -1, this, &Tsunami::OnAddTimeTrack);
	HuiAddCommandM("add_midi_track", "hui:add", -1, this, &Tsunami::OnAddMidiTrack);
	HuiAddCommandM("delete_track", "hui:delete", -1, this, &Tsunami::OnDeleteTrack);
	HuiAddCommandM("level_add", "hui:add", -1, this, &Tsunami::OnAddLevel);
	HuiAddCommandM("level_delete", "hui:delete", -1, this, &Tsunami::OnDeleteLevel);
	HuiAddCommandM("level_up", "hui:up", -1, this, &Tsunami::OnCurLevelUp);
	HuiAddCommandM("level_down", "hui:down", -1, this, &Tsunami::OnCurLevelDown);
	HuiAddCommandM("sample_manager", "", -1, this, &Tsunami::OnSampleManager);
	HuiAddCommandM("show_mixing_console", "", -1, this, &Tsunami::OnMixingConsole);
	HuiAddCommandM("show_fx_console", "", -1, this, &Tsunami::OnFxConsole);
	HuiAddCommandM("sample_from_selection", "hui:cut", -1, this, &Tsunami::OnSubFromSelection);
	HuiAddCommandM("insert_sample", "", KEY_I + KEY_CONTROL, this, &Tsunami::OnInsertAdded);
	HuiAddCommandM("remove_sample", "", -1, this, &Tsunami::OnRemoveAdded);
	HuiAddCommandM("track_import", "", -1, this, &Tsunami::OnTrackImport);
	HuiAddCommandM("sub_import", "", -1, this, &Tsunami::OnSubImport);
	HuiAddCommandM("audio_file_properties", "", KEY_F4, this, &Tsunami::OnAudioProperties);
	HuiAddCommandM("track_properties", "", -1, this, &Tsunami::OnTrackProperties);
	HuiAddCommandM("sample_properties", "", -1, this, &Tsunami::OnSubProperties);
	HuiAddCommandM("settings", "", -1, this, &Tsunami::OnSettings);
	HuiAddCommandM("close_file", "hui:close", KEY_W + KEY_CONTROL, this, &Tsunami::OnCloseFile);
	HuiAddCommandM("play", "hui:media-play", -1, this, &Tsunami::OnPlay);
	HuiAddCommandM("play_loop", "", -1, this, &Tsunami::OnPlayLoop);
	HuiAddCommandM("pause", "hui:media-pause", -1, this, &Tsunami::OnPause);
	HuiAddCommandM("stop", "hui:media-stop", -1, this, &Tsunami::OnStop);
	HuiAddCommandM("record", "hui:media-record", -1, this, &Tsunami::OnRecord);
	HuiAddCommandM("show_log", "", -1, this, &Tsunami::OnShowLog);
	HuiAddCommandM("about", "", -1, this, &Tsunami::OnAbout);
	HuiAddCommandM("run_plugin", "hui:execute", KEY_RETURN + KEY_SHIFT, this, &Tsunami::OnFindAndExecutePlugin);
	HuiAddCommandM("exit", "hui:quit", KEY_Q + KEY_CONTROL, this, &Tsunami::OnExit);

	HuiAddCommandM("select_all", "", KEY_A + KEY_CONTROL, this, &Tsunami::OnSelectAll);
	HuiAddCommandM("select_nothing", "", -1, this, &Tsunami::OnSelectNone);
	HuiAddCommandM("view_mono", "", -1, this, &Tsunami::OnViewMono);
	HuiAddCommandM("view_stereo", "", -1, this, &Tsunami::OnViewStereo);
	HuiAddCommandM("view_peaks_max", "", -1, this, &Tsunami::OnViewPeaksMax);
	HuiAddCommandM("view_peaks_mean", "", -1, this, &Tsunami::OnViewPeaksMean);
	HuiAddCommandM("view_optimal", "", -1, this, &Tsunami::OnViewOptimal);
	HuiAddCommandM("zoom_in", "", -1, this, &Tsunami::OnZoomIn);
	HuiAddCommandM("zoom_out", "", -1, this, &Tsunami::OnZoomOut);

	// table structure
	SetSize(width, height);
	SetBorderWidth(0);
	AddControlTable("", 0, 0, 1, 2, "root_table");
	SetTarget("root_table", 0);
	AddControlTable("", 0, 0, 2, 1, "main_table");

	// main table
	SetTarget("main_table", 0);
	AddDrawingArea("!grabfocus", 0, 0, 0, 0, "area");


	toolbar[0]->SetByID("toolbar");
	//ToolbarConfigure(false, true);

	SetMenu(HuiCreateResourceMenu("menu"));
	//ToolBarConfigure(true, true);
	SetMaximized(maximized);

	// events
	EventM("hui:close", this, &Tsunami::OnExit);
	for (int i=0;i<256;i++)
		EventM(format("jump_to_level_%d", i), this, &Tsunami::OnCurLevel);


	audio = new AudioFile;

	storage = new Storage;

	view = new AudioView(this, audio, output, input, renderer);

	// side bar
	side_bar = new SideBar(view, audio);
	Embed(side_bar, "main_table", 1, 0);
	side_bar->Hide();

	// bottom bar
	bottom_bar = new BottomBar(view, audio, output, log);
	Embed(bottom_bar, "root_table", 0, 1);
	bottom_bar->Hide();

	// create (link) PluginManager after all other components are ready
	plugin_manager = new PluginManager;
	plugin_manager->AddPluginsToMenu();
	plugin_manager->LinkAppScriptData();

	Subscribe(view);
	Subscribe(audio);
	Subscribe(output, output->MESSAGE_STATE_CHANGE);
	Subscribe(clipboard);
	Subscribe(bottom_bar);

	UpdateMenu();

	log->Info(AppName + " " + AppVersion);
	log->Info(_("  ...keine Sorge, das wird schon!"));

	audio->NewWithOneTrack(Track::TYPE_AUDIO, DEFAULT_SAMPLE_RATE);

	HandleArguments(arg);

	Show();

	HuiRunLaterM(0.01f, this, &Tsunami::OnViewOptimal);
}

Tsunami::~Tsunami()
{
	Unsubscribe(view);
	Unsubscribe(audio);
	Unsubscribe(output);
	Unsubscribe(clipboard);
	Unsubscribe(bottom_bar);

	int w, h;
	GetSizeDesired(w, h);
	HuiConfig.setInt("Window.Width", w);
	HuiConfig.setInt("Window.Height", h);
	HuiConfig.setBool("Window.Maximized", IsMaximized());

	delete(side_bar);
	delete(bottom_bar);
	delete(storage);
	delete(view);
	delete(output);
	delete(input);
	delete(audio);
	delete(renderer);
	delete(plugin_manager);
	HuiEnd();
}


int Tsunami::Run()
{
	return HuiRun();
}



void Tsunami::OnAbout()
{
	HuiAboutBox(this);
}



void Tsunami::OnAddTrack()
{
	audio->AddTrack(Track::TYPE_AUDIO);
}

void Tsunami::OnAddTimeTrack()
{
	audio->action_manager->BeginActionGroup();
	Track *t = audio->AddTrack(Track::TYPE_TIME);
	if (t)
		t->AddBars(-1, 90, 4, 10);
	audio->action_manager->EndActionGroup();
}

void Tsunami::OnAddMidiTrack()
{
	audio->AddTrack(Track::TYPE_MIDI);
}

void Tsunami::OnDeleteTrack()
{
	if (audio->used){
		if (audio->track.num < 2){
			log->Error(_("Es muss mindestens eine Spur existieren"));
			return;
		}

		if (view->cur_track)
			audio->DeleteTrack(get_track_index(view->cur_track));
		else
			log->Error(_("Keine Spur ausgew&ahlt"));
	}
}

void Tsunami::OnCloseFile()
{
	audio->Reset();
}

void Tsunami::LoadKeyCodes()
{
}

void Tsunami::OnAudioProperties()
{
	side_bar->Choose(SideBar::AUDIO_FILE_DIALOG);
}

void Tsunami::OnTrackProperties()
{
	if (view->cur_track)
		side_bar->Open(SideBar::TRACK_DIALOG);
	else
		log->Error(_("Keine Spur ausgew&ahlt"));
}

void Tsunami::OnSubProperties()
{
	if (view->cur_sample)
		side_bar->Open(SideBar::SUB_DIALOG);
	else
		log->Error(_("Kein Sample ausgew&ahlt"));
}

void Tsunami::OnShowLog()
{
	bottom_bar->Choose(BottomBar::LOG_CONSOLE);
}

void Tsunami::OnUndo()
{
	audio->action_manager->Undo();
}

void Tsunami::OnRedo()
{
	audio->action_manager->Redo();
}

void Tsunami::OnSendBugReport()
{
}


string title_filename(const string &filename)
{
	if (filename.num > 0)
		return filename.basename();// + " (" + filename.dirname() + ")";
	return _("Unbenannt");
}

bool Tsunami::AllowTermination()
{
	if (!audio->action_manager->IsSave()){
		string answer = HuiQuestionBox(this, _("Frage"), format(_("'%s'\nDatei speichern?"), title_filename(audio->filename).c_str()), true);
		if (answer == "hui:yes"){
			/*if (!OnSave())
				return false;*/
			OnSave();
		}else if (answer == "hui:cancel")
			return false;
	}
	return true;
}

void Tsunami::OnCopy()
{
	if (audio->used)
		clipboard->Copy(audio);
}

void Tsunami::OnPaste()
{
	clipboard->Paste(audio);
}

void Tsunami::OnFindAndExecutePlugin()
{
	plugin_manager->FindAndExecutePlugin();
}

void Tsunami::OnDelete()
{
	if (audio->used)
		audio->DeleteSelection(view->cur_level, view->sel_range, false);
}

void Tsunami::OnSampleManager()
{
	bottom_bar->Choose(BottomBar::SAMPLE_CONSOLE);
}

void Tsunami::OnMixingConsole()
{
	bottom_bar->Choose(BottomBar::MIXING_CONSOLE);
}

void Tsunami::OnFxConsole()
{
	bottom_bar->Choose(BottomBar::FX_CONSOLE);
}

void Tsunami::OnSubImport()
{
}

void Tsunami::OnCommand(const string & id)
{
}

void Tsunami::OnSettings()
{
	SettingsDialog *dlg = new SettingsDialog(this, false);
	dlg->Run();
}

void Tsunami::OnTrackImport()
{
	if (!audio->used)
		return;
	if (storage->AskOpenImport(this)){
		Track *t = audio->AddTrack(Track::TYPE_AUDIO);
		storage->LoadTrack(t, HuiFilename, view->sel_range.start(), view->cur_level);
	}
}

bool Tsunami::HandleArguments(Array<string> arg)
{
	if (arg.num > 1)
		return storage->Load(audio, arg[1]);
	return false;
}

void Tsunami::OnRemoveAdded()
{
	audio->DeleteSelectedSamples();
}

void Tsunami::OnPlayLoop()
{
	renderer->loop_if_allowed = !renderer->loop_if_allowed;
	UpdateMenu();
}

void Tsunami::OnPlay()
{
	renderer->Prepare(audio, view->GetPlaybackSelection(), true);
	output->Play(renderer);
}

void Tsunami::OnPause()
{
	output->Pause();
}

void Tsunami::OnStop()
{
	output->Stop();
}

void Tsunami::OnInsertAdded()
{
	if (audio->used)
		audio->InsertSelectedSamples(view->cur_level);
}

void Tsunami::OnRecord()
{
	CaptureDialog *dlg = new CaptureDialog(this, false, audio);
	dlg->Run();
}

void Tsunami::OnAddLevel()
{
	if (audio->used)
		audio->AddLevel();
}

void Tsunami::OnDeleteLevel()
{
	audio->DeleteLevel(view->cur_level, true);
}

void Tsunami::OnCurLevel()
{
	view->SetCurLevel(HuiGetEvent()->id.substr(14, -1)._int());
}

void Tsunami::OnCurLevelUp()
{
	view->SetCurLevel(view->cur_level + 1);
}

void Tsunami::OnCurLevelDown()
{
	view->SetCurLevel(view->cur_level - 1);
}

void Tsunami::OnSubFromSelection()
{
	if (audio->used)
		audio->CreateSamplesFromSelection(view->cur_level, view->sel_range);
}

void Tsunami::OnViewOptimal()
{
	view->OptimizeView();
}

void Tsunami::OnSelectNone()
{
	view->SelectNone();
}

void Tsunami::OnSelectAll()
{
	view->SelectAll();
}

void Tsunami::OnViewPeaksMax()
{
	view->SetPeaksMode(BufferBox::PEAK_MODE_MAXIMUM);
}

void Tsunami::OnViewPeaksMean()
{
	view->SetPeaksMode(BufferBox::PEAK_MODE_SQUAREMEAN);
}

void Tsunami::OnViewMono()
{
	view->SetShowMono(true);
}

void Tsunami::OnViewStereo()
{
	view->SetShowMono(false);
}

void Tsunami::OnZoomIn()
{
	view->ZoomIn();
}

void Tsunami::OnZoomOut()
{
	view->ZoomOut();
}

void Tsunami::UpdateMenu()
{
	msg_db_f("UpdateMenu", 1);
	bool selected = !view->sel_range.empty();
// menu / toolbar
	// edit
	Enable("select_all", audio->used);
	Enable("select_nothing", audio->used);
	Enable("undo", audio->action_manager->Undoable());
	Enable("redo", audio->action_manager->Redoable());
	Enable("copy", clipboard->CanCopy(audio));
	Enable("paste", clipboard->HasData());
	Enable("delete", selected || (audio->GetNumSelectedSamples() > 0));
	// file
	Enable("save", audio->used);
	Enable("save", audio->used);
	Enable("save_as", audio->used);
	Enable("close_file", audio->used);
	Enable("export_selection", audio->used);
	Enable("wave_properties", audio->used);
	// track
	Enable("track_import", audio->used);
	Enable("add_track", audio->used);
	Enable("add_time_track", audio->used);
	Enable("delete_track", view->cur_track);
	Enable("track_properties", view->cur_track);
	// level
	Enable("level_add", audio->used);
	Enable("level_delete", audio->used && (audio->level_name.num > 1));
	Enable("level_up", audio->used && (view->cur_level < audio->level_name.num -1));
	Enable("level_down", audio->used && (view->cur_level > 0));
	// sub
	Enable("sample_from_selection", selected);
	Enable("insert_sample", audio->GetNumSelectedSamples() > 0);
	Enable("remove_sample", audio->GetNumSelectedSamples() > 0);
	Enable("sample_properties", view->cur_sample);
	// sound
	Enable("play", audio->used);
	Enable("stop", output->IsPlaying());
	Enable("pause", output->IsPlaying());
	Check("play_loop", renderer->loop_if_allowed);
	// view
	Check("show_mixing_console", bottom_bar->IsActive(BottomBar::MIXING_CONSOLE));
	Check("show_fx_console", bottom_bar->IsActive(BottomBar::FX_CONSOLE));
	Check("sample_manager", bottom_bar->IsActive(BottomBar::SAMPLE_CONSOLE));

	HuiMenu *m = GetMenu()->GetSubMenuByID("menu_level_target");
	if (m){
		m->Clear();
		for (int i=0; i<audio->level_name.num; i++)
			m->AddItemCheckable(audio->GetNiceLevelName(i), format("jump_to_level_%d", i));
		Check(format("jump_to_level_%d", view->cur_level), true);
	}

	if (audio->used){
		string title = title_filename(audio->filename) + " - " + AppName;
		if (!audio->action_manager->IsSave())
			title = "*" + title;
		SetTitle(title);
	}else
		SetTitle(AppName);
}


void Tsunami::OnUpdate(Observable *o, const string &message)
{
	if (o == output){
		view->ForceRedraw();
		UpdateMenu();
	}else // "Clipboard", "AudioFile" or "AudioView"
		UpdateMenu();
}


void Tsunami::OnExit()
{
	if (AllowTermination())
		delete(this);
}


void Tsunami::OnNew()
{
	NewDialog *d = new NewDialog(tsunami, false, audio);
	d->Run();
}


void Tsunami::OnOpen()
{
	if (storage->AskOpen(this))
		storage->Load(audio, HuiFilename);
}


void Tsunami::OnSave()
{
	if (!audio->used)
		return;
	if (audio->filename == "")
		OnSaveAs();
	else
		storage->Save(audio, audio->filename);
}


void Tsunami::OnSaveAs()
{
	if (!audio->used)
		return;
	if (storage->AskSave(this))
		storage->Save(audio, HuiFilename);
}

void Tsunami::OnExport()
{
	if (!audio->used)
		return;
	if (storage->AskSaveExport(this))
		storage->Export(audio, view->GetPlaybackSelection(), HuiFilename);
}
