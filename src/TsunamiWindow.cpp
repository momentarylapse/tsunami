/*
 * Tsunami.cpp
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#include "lib/hui/hui.h"
#include "TsunamiWindow.h"
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

extern string AppName;

HuiTimer debug_timer;

TsunamiWindow::TsunamiWindow() :
	Observer("Tsunami"),
	HuiWindow(AppName, -1, -1, 800, 600, NULL, false, HuiWinModeResizable | HuiWinModeControls)
{

	tsunami->win = this;

	int width = HuiConfig.getInt("Window.Width", 800);
	int height = HuiConfig.getInt("Window.Height", 600);
	bool maximized = HuiConfig.getBool("Window.Maximized", true);

	//HuiAddKeyCode("insert_added", KEY_RETURN);
	//HuiAddKeyCode("remove_added", KEY_BACKSPACE);

	HuiAddCommandM("new", "hui:new", KEY_N + KEY_CONTROL, this, &TsunamiWindow::OnNew);
	HuiAddCommandM("open", "hui:open", KEY_O + KEY_CONTROL, this, &TsunamiWindow::OnOpen);
	HuiAddCommandM("save", "hui:save", KEY_S + KEY_CONTROL, this, &TsunamiWindow::OnSave);
	HuiAddCommandM("save_as", "hui:save-as", KEY_S + KEY_CONTROL + KEY_SHIFT, this, &TsunamiWindow::OnSaveAs);
	HuiAddCommandM("copy", "hui:copy", KEY_C + KEY_CONTROL, this, &TsunamiWindow::OnCopy);
	HuiAddCommandM("paste", "hui:paste", KEY_V + KEY_CONTROL, this, &TsunamiWindow::OnPaste);
	HuiAddCommandM("delete", "hui:delete", -1, this, &TsunamiWindow::OnDelete);
	HuiAddCommandM("export_selection", "", KEY_X + KEY_CONTROL, this, &TsunamiWindow::OnExport);
	HuiAddCommandM("undo", "hui:undo", KEY_Z + KEY_CONTROL, this, &TsunamiWindow::OnUndo);
	HuiAddCommandM("redo", "hui:redo", KEY_Y + KEY_CONTROL, this, &TsunamiWindow::OnRedo);
	HuiAddCommandM("add_track", "hui:add", -1, this, &TsunamiWindow::OnAddTrack);
	HuiAddCommandM("add_time_track", "hui:add", -1, this, &TsunamiWindow::OnAddTimeTrack);
	HuiAddCommandM("add_midi_track", "hui:add", -1, this, &TsunamiWindow::OnAddMidiTrack);
	HuiAddCommandM("delete_track", "hui:delete", -1, this, &TsunamiWindow::OnDeleteTrack);
	HuiAddCommandM("track_edit_midi", "hui:edit", -1, this, &TsunamiWindow::OnTrackEditMidi);
	HuiAddCommandM("track_edit_fx", "hui:edit", -1, this, &TsunamiWindow::OnTrackEditFX);
	HuiAddCommandM("level_add", "hui:add", -1, this, &TsunamiWindow::OnAddLevel);
	HuiAddCommandM("level_delete", "hui:delete", -1, this, &TsunamiWindow::OnDeleteLevel);
	HuiAddCommandM("level_up", "hui:up", -1, this, &TsunamiWindow::OnCurLevelUp);
	HuiAddCommandM("level_down", "hui:down", -1, this, &TsunamiWindow::OnCurLevelDown);
	HuiAddCommandM("sample_manager", "", -1, this, &TsunamiWindow::OnSampleManager);
	HuiAddCommandM("show_mixing_console", "", -1, this, &TsunamiWindow::OnMixingConsole);
	HuiAddCommandM("show_fx_console", "", -1, this, &TsunamiWindow::OnFxConsole);
	HuiAddCommandM("sample_from_selection", "hui:cut", -1, this, &TsunamiWindow::OnSubFromSelection);
	HuiAddCommandM("insert_sample", "", KEY_I + KEY_CONTROL, this, &TsunamiWindow::OnInsertAdded);
	HuiAddCommandM("remove_sample", "", -1, this, &TsunamiWindow::OnRemoveAdded);
	HuiAddCommandM("track_import", "", -1, this, &TsunamiWindow::OnTrackImport);
	HuiAddCommandM("sub_import", "", -1, this, &TsunamiWindow::OnSubImport);
	HuiAddCommandM("audio_file_properties", "", KEY_F4, this, &TsunamiWindow::OnAudioProperties);
	HuiAddCommandM("track_properties", "", -1, this, &TsunamiWindow::OnTrackProperties);
	HuiAddCommandM("sample_properties", "", -1, this, &TsunamiWindow::OnSubProperties);
	HuiAddCommandM("settings", "", -1, this, &TsunamiWindow::OnSettings);
	HuiAddCommandM("close_file", "hui:close", KEY_W + KEY_CONTROL, this, &TsunamiWindow::OnCloseFile);
	HuiAddCommandM("play", "hui:media-play", -1, this, &TsunamiWindow::OnPlay);
	HuiAddCommandM("play_loop", "", -1, this, &TsunamiWindow::OnPlayLoop);
	HuiAddCommandM("pause", "hui:media-pause", -1, this, &TsunamiWindow::OnPause);
	HuiAddCommandM("stop", "hui:media-stop", -1, this, &TsunamiWindow::OnStop);
	HuiAddCommandM("record", "hui:media-record", -1, this, &TsunamiWindow::OnRecord);
	HuiAddCommandM("show_log", "", -1, this, &TsunamiWindow::OnShowLog);
	HuiAddCommandM("about", "", -1, this, &TsunamiWindow::OnAbout);
	HuiAddCommandM("run_plugin", "hui:execute", KEY_RETURN + KEY_SHIFT, this, &TsunamiWindow::OnFindAndExecutePlugin);
	HuiAddCommandM("exit", "hui:quit", KEY_Q + KEY_CONTROL, this, &TsunamiWindow::OnExit);

	HuiAddCommandM("select_all", "", KEY_A + KEY_CONTROL, this, &TsunamiWindow::OnSelectAll);
	HuiAddCommandM("select_nothing", "", -1, this, &TsunamiWindow::OnSelectNone);
	HuiAddCommandM("view_mono", "", -1, this, &TsunamiWindow::OnViewMono);
	HuiAddCommandM("view_stereo", "", -1, this, &TsunamiWindow::OnViewStereo);
	HuiAddCommandM("view_peaks_max", "", -1, this, &TsunamiWindow::OnViewPeaksMax);
	HuiAddCommandM("view_peaks_mean", "", -1, this, &TsunamiWindow::OnViewPeaksMean);
	HuiAddCommandM("view_optimal", "", -1, this, &TsunamiWindow::OnViewOptimal);
	HuiAddCommandM("zoom_in", "", -1, this, &TsunamiWindow::OnZoomIn);
	HuiAddCommandM("zoom_out", "", -1, this, &TsunamiWindow::OnZoomOut);

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
	EventM("hui:close", this, &TsunamiWindow::OnExit);
	for (int i=0;i<256;i++)
		EventM(format("jump_to_level_%d", i), this, &TsunamiWindow::OnCurLevel);

	audio = tsunami->audio;


	view = new AudioView(this, audio, tsunami->output, tsunami->input, tsunami->renderer);

	// side bar
	side_bar = new SideBar(view, audio);
	Embed(side_bar, "main_table", 1, 0);
	side_bar->Hide();

	// bottom bar
	bottom_bar = new BottomBar(view, audio, tsunami->output, tsunami->log);
	Embed(bottom_bar, "root_table", 0, 1);
	bottom_bar->Hide();

	Subscribe(view);
	Subscribe(audio);
	Subscribe(tsunami->output, tsunami->output->MESSAGE_STATE_CHANGE);
	Subscribe(tsunami->clipboard);
	Subscribe(bottom_bar);


	if (audio->track.num > 0)
		view->SetCurTrack(audio->track[0]);

	UpdateMenu();
}

TsunamiWindow::~TsunamiWindow()
{
	Unsubscribe(view);
	Unsubscribe(audio);
	Unsubscribe(tsunami->output);
	Unsubscribe(tsunami->clipboard);
	Unsubscribe(bottom_bar);

	int w, h;
	GetSizeDesired(w, h);
	HuiConfig.setInt("Window.Width", w);
	HuiConfig.setInt("Window.Height", h);
	HuiConfig.setBool("Window.Maximized", IsMaximized());

	delete(side_bar);
	delete(bottom_bar);
	delete(view);
	HuiEnd();
}


void TsunamiWindow::OnAbout()
{
	HuiAboutBox(this);
}



void TsunamiWindow::OnAddTrack()
{
	audio->AddTrack(Track::TYPE_AUDIO);
}

void TsunamiWindow::OnAddTimeTrack()
{
	audio->action_manager->BeginActionGroup();
	Track *t = audio->AddTrack(Track::TYPE_TIME);
	if (t)
		t->AddBars(-1, 90, 4, 10);
	audio->action_manager->EndActionGroup();
}

void TsunamiWindow::OnAddMidiTrack()
{
	audio->AddTrack(Track::TYPE_MIDI);
}

void TsunamiWindow::OnDeleteTrack()
{
	if (audio->used){
		if (audio->track.num < 2){
			tsunami->log->Error(_("Es muss mindestens eine Spur existieren"));
			return;
		}

		if (view->cur_track)
			audio->DeleteTrack(get_track_index(view->cur_track));
		else
			tsunami->log->Error(_("Keine Spur ausgew&ahlt"));
	}
}

void TsunamiWindow::OnTrackEditMidi()
{
	if (view->cur_track)
		bottom_bar->Choose(BottomBar::MIDI_EDITOR);
	else
		tsunami->log->Error(_("Keine Spur ausgew&ahlt"));
}

void TsunamiWindow::OnTrackEditFX()
{
	if (view->cur_track)
		bottom_bar->Choose(BottomBar::FX_CONSOLE);
	else
		tsunami->log->Error(_("Keine Spur ausgew&ahlt"));
}

void TsunamiWindow::OnCloseFile()
{
	audio->Reset();
}

void TsunamiWindow::OnAudioProperties()
{
	side_bar->Choose(SideBar::AUDIO_FILE_DIALOG);
}

void TsunamiWindow::OnTrackProperties()
{
	if (view->cur_track)
		side_bar->Open(SideBar::TRACK_DIALOG);
	else
		tsunami->log->Error(_("Keine Spur ausgew&ahlt"));
}

void TsunamiWindow::OnSubProperties()
{
	if (view->cur_sample)
		side_bar->Open(SideBar::SUB_DIALOG);
	else
		tsunami->log->Error(_("Kein Sample ausgew&ahlt"));
}

void TsunamiWindow::OnShowLog()
{
	bottom_bar->Choose(BottomBar::LOG_CONSOLE);
}

void TsunamiWindow::OnUndo()
{
	audio->action_manager->Undo();
}

void TsunamiWindow::OnRedo()
{
	audio->action_manager->Redo();
}

void TsunamiWindow::OnSendBugReport()
{
}


string title_filename(const string &filename)
{
	if (filename.num > 0)
		return filename.basename();// + " (" + filename.dirname() + ")";
	return _("Unbenannt");
}

bool TsunamiWindow::AllowTermination()
{
	if (audio->action_manager->IsSave())
		return true;
	string answer = HuiQuestionBox(this, _("Frage"), format(_("'%s'\nDatei speichern?"), title_filename(audio->filename).c_str()), true);
	if (answer == "hui:yes"){
		/*if (!OnSave())
			return false;*/
		OnSave();
		return true;
	}else if (answer == "hui:no")
		return true;

	// cancel
	return false;
}

void TsunamiWindow::OnCopy()
{
	if (audio->used)
		tsunami->clipboard->Copy(audio);
}

void TsunamiWindow::OnPaste()
{
	tsunami->clipboard->Paste(audio);
}

void TsunamiWindow::OnFindAndExecutePlugin()
{
	tsunami->plugin_manager->FindAndExecutePlugin();
}

void TsunamiWindow::OnDelete()
{
	if (audio->used)
		audio->DeleteSelection(view->cur_level, view->sel_range, false);
}

void TsunamiWindow::OnSampleManager()
{
	bottom_bar->Choose(BottomBar::SAMPLE_CONSOLE);
}

void TsunamiWindow::OnMixingConsole()
{
	bottom_bar->Choose(BottomBar::MIXING_CONSOLE);
}

void TsunamiWindow::OnFxConsole()
{
	bottom_bar->Choose(BottomBar::FX_CONSOLE);
}

void TsunamiWindow::OnSubImport()
{
}

void TsunamiWindow::OnCommand(const string & id)
{
}

void TsunamiWindow::OnSettings()
{
	SettingsDialog *dlg = new SettingsDialog(this, false);
	dlg->Run();
}

void TsunamiWindow::OnTrackImport()
{
	if (!audio->used)
		return;
	if (tsunami->storage->AskOpenImport(this)){
		Track *t = audio->AddTrack(Track::TYPE_AUDIO);
		tsunami->storage->LoadTrack(t, HuiFilename, view->sel_range.start(), view->cur_level);
	}
}

void TsunamiWindow::OnRemoveAdded()
{
	audio->DeleteSelectedSamples();
}

void TsunamiWindow::OnPlayLoop()
{
	tsunami->renderer->loop_if_allowed = !tsunami->renderer->loop_if_allowed;
	UpdateMenu();
}

void TsunamiWindow::OnPlay()
{
	tsunami->renderer->Prepare(audio, view->GetPlaybackSelection(), true);
	tsunami->output->Play(tsunami->renderer);
}

void TsunamiWindow::OnPause()
{
	tsunami->output->Pause();
}

void TsunamiWindow::OnStop()
{
	tsunami->output->Stop();
}

void TsunamiWindow::OnInsertAdded()
{
	if (audio->used)
		audio->InsertSelectedSamples(view->cur_level);
}

void TsunamiWindow::OnRecord()
{
	CaptureDialog *dlg = new CaptureDialog(this, false, audio);
	dlg->Run();
}

void TsunamiWindow::OnAddLevel()
{
	if (audio->used)
		audio->AddLevel("");
}

void TsunamiWindow::OnDeleteLevel()
{
	audio->DeleteLevel(view->cur_level, true);
}

void TsunamiWindow::OnCurLevel()
{
	view->SetCurLevel(HuiGetEvent()->id.substr(14, -1)._int());
}

void TsunamiWindow::OnCurLevelUp()
{
	view->SetCurLevel(view->cur_level + 1);
}

void TsunamiWindow::OnCurLevelDown()
{
	view->SetCurLevel(view->cur_level - 1);
}

void TsunamiWindow::OnSubFromSelection()
{
	if (audio->used)
		audio->CreateSamplesFromSelection(view->cur_level, view->sel_range);
}

void TsunamiWindow::OnViewOptimal()
{
	view->OptimizeView();
}

void TsunamiWindow::OnSelectNone()
{
	view->SelectNone();
}

void TsunamiWindow::OnSelectAll()
{
	view->SelectAll();
}

void TsunamiWindow::OnViewPeaksMax()
{
	view->SetPeaksMode(BufferBox::PEAK_MODE_MAXIMUM);
}

void TsunamiWindow::OnViewPeaksMean()
{
	view->SetPeaksMode(BufferBox::PEAK_MODE_SQUAREMEAN);
}

void TsunamiWindow::OnViewMono()
{
	view->SetShowMono(true);
}

void TsunamiWindow::OnViewStereo()
{
	view->SetShowMono(false);
}

void TsunamiWindow::OnZoomIn()
{
	view->ZoomIn();
}

void TsunamiWindow::OnZoomOut()
{
	view->ZoomOut();
}

void TsunamiWindow::UpdateMenu()
{
	msg_db_f("UpdateMenu", 1);
	bool selected = !view->sel_range.empty();
// menu / toolbar
	// edit
	Enable("select_all", audio->used);
	Enable("select_nothing", audio->used);
	Enable("undo", audio->action_manager->Undoable());
	Enable("redo", audio->action_manager->Redoable());
	Enable("copy", tsunami->clipboard->CanCopy(audio));
	Enable("paste", tsunami->clipboard->HasData());
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
	Enable("stop", tsunami->output->IsPlaying());
	Enable("pause", tsunami->output->IsPlaying());
	Check("play_loop", tsunami->renderer->loop_if_allowed);
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


void TsunamiWindow::OnUpdate(Observable *o, const string &message)
{
	if (o == tsunami->output){
		view->ForceRedraw();
		UpdateMenu();
	}else // "Clipboard", "AudioFile" or "AudioView"
		UpdateMenu();
}


void TsunamiWindow::OnExit()
{
	if (AllowTermination())
		delete(this);
}


void TsunamiWindow::OnNew()
{
	if (!AllowTermination())
		return;
	NewDialog *d = new NewDialog(this, false, audio);
	d->Run();
}


void TsunamiWindow::OnOpen()
{
	if (!AllowTermination())
		return;
	if (tsunami->storage->AskOpen(this))
		tsunami->storage->Load(audio, HuiFilename);
}


void TsunamiWindow::OnSave()
{
	if (!audio->used)
		return;
	if (audio->filename == "")
		OnSaveAs();
	else
		tsunami->storage->Save(audio, audio->filename);
}


void TsunamiWindow::OnSaveAs()
{
	if (!audio->used)
		return;
	if (tsunami->storage->AskSave(this))
		tsunami->storage->Save(audio, HuiFilename);
}

void TsunamiWindow::OnExport()
{
	if (!audio->used)
		return;
	if (tsunami->storage->AskSaveExport(this))
		tsunami->storage->Export(audio, view->GetPlaybackSelection(), HuiFilename);
}
