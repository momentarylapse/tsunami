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
#include "Audio/AudioStream.h"
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

	HuiAddCommandM("new", "hui:new", KEY_N + KEY_CONTROL, this, &TsunamiWindow::onNew);
	HuiAddCommandM("open", "hui:open", KEY_O + KEY_CONTROL, this, &TsunamiWindow::onOpen);
	HuiAddCommandM("save", "hui:save", KEY_S + KEY_CONTROL, this, &TsunamiWindow::onSave);
	HuiAddCommandM("save_as", "hui:save-as", KEY_S + KEY_CONTROL + KEY_SHIFT, this, &TsunamiWindow::onSaveAs);
	HuiAddCommandM("copy", "hui:copy", KEY_C + KEY_CONTROL, this, &TsunamiWindow::onCopy);
	HuiAddCommandM("paste", "hui:paste", KEY_V + KEY_CONTROL, this, &TsunamiWindow::onPaste);
	HuiAddCommandM("delete", "hui:delete", -1, this, &TsunamiWindow::onDelete);
	HuiAddCommandM("export_selection", "", KEY_X + KEY_CONTROL, this, &TsunamiWindow::onExport);
	HuiAddCommandM("undo", "hui:undo", KEY_Z + KEY_CONTROL, this, &TsunamiWindow::onUndo);
	HuiAddCommandM("redo", "hui:redo", KEY_Y + KEY_CONTROL, this, &TsunamiWindow::onRedo);
	HuiAddCommandM("add_track", "hui:add", -1, this, &TsunamiWindow::onAddTrack);
	HuiAddCommandM("add_time_track", "hui:add", -1, this, &TsunamiWindow::onAddTimeTrack);
	HuiAddCommandM("add_midi_track", "hui:add", -1, this, &TsunamiWindow::onAddMidiTrack);
	HuiAddCommandM("delete_track", "hui:delete", -1, this, &TsunamiWindow::onDeleteTrack);
	HuiAddCommandM("track_edit_midi", "hui:edit", -1, this, &TsunamiWindow::onTrackEditMidi);
	HuiAddCommandM("track_edit_fx", "hui:edit", -1, this, &TsunamiWindow::onTrackEditFX);
	HuiAddCommandM("level_add", "hui:add", -1, this, &TsunamiWindow::onAddLevel);
	HuiAddCommandM("level_delete", "hui:delete", -1, this, &TsunamiWindow::onDeleteLevel);
	HuiAddCommandM("level_up", "hui:up", -1, this, &TsunamiWindow::onCurLevelUp);
	HuiAddCommandM("level_down", "hui:down", -1, this, &TsunamiWindow::onCurLevelDown);
	HuiAddCommandM("sample_manager", "", -1, this, &TsunamiWindow::onSampleManager);
	HuiAddCommandM("show_mixing_console", "", -1, this, &TsunamiWindow::onMixingConsole);
	HuiAddCommandM("show_fx_console", "", -1, this, &TsunamiWindow::onFxConsole);
	HuiAddCommandM("sample_from_selection", "hui:cut", -1, this, &TsunamiWindow::onSubFromSelection);
	HuiAddCommandM("insert_sample", "", KEY_I + KEY_CONTROL, this, &TsunamiWindow::onInsertAdded);
	HuiAddCommandM("remove_sample", "", -1, this, &TsunamiWindow::onRemoveAdded);
	HuiAddCommandM("track_import", "", -1, this, &TsunamiWindow::onTrackImport);
	HuiAddCommandM("sub_import", "", -1, this, &TsunamiWindow::onSubImport);
	HuiAddCommandM("audio_file_properties", "", KEY_F4, this, &TsunamiWindow::onAudioProperties);
	HuiAddCommandM("track_properties", "", -1, this, &TsunamiWindow::onTrackProperties);
	HuiAddCommandM("sample_properties", "", -1, this, &TsunamiWindow::onSubProperties);
	HuiAddCommandM("settings", "", -1, this, &TsunamiWindow::onSettings);
	HuiAddCommandM("play", "hui:media-play", -1, this, &TsunamiWindow::onPlay);
	HuiAddCommandM("play_loop", "", -1, this, &TsunamiWindow::onPlayLoop);
	HuiAddCommandM("pause", "hui:media-pause", -1, this, &TsunamiWindow::onPause);
	HuiAddCommandM("stop", "hui:media-stop", -1, this, &TsunamiWindow::onStop);
	HuiAddCommandM("record", "hui:media-record", -1, this, &TsunamiWindow::onRecord);
	HuiAddCommandM("show_log", "", -1, this, &TsunamiWindow::onShowLog);
	HuiAddCommandM("about", "", -1, this, &TsunamiWindow::onAbout);
	HuiAddCommandM("run_plugin", "hui:execute", KEY_RETURN + KEY_SHIFT, this, &TsunamiWindow::onFindAndExecutePlugin);
	HuiAddCommandM("exit", "hui:quit", KEY_Q + KEY_CONTROL, this, &TsunamiWindow::onExit);

	HuiAddCommandM("select_all", "", KEY_A + KEY_CONTROL, this, &TsunamiWindow::onSelectAll);
	HuiAddCommandM("select_nothing", "", -1, this, &TsunamiWindow::onSelectNone);
	HuiAddCommandM("view_mono", "", -1, this, &TsunamiWindow::onViewMono);
	HuiAddCommandM("view_stereo", "", -1, this, &TsunamiWindow::onViewStereo);
	HuiAddCommandM("view_peaks_max", "", -1, this, &TsunamiWindow::onViewPeaksMax);
	HuiAddCommandM("view_peaks_mean", "", -1, this, &TsunamiWindow::onViewPeaksMean);
	HuiAddCommandM("view_optimal", "", -1, this, &TsunamiWindow::onViewOptimal);
	HuiAddCommandM("zoom_in", "", -1, this, &TsunamiWindow::onZoomIn);
	HuiAddCommandM("zoom_out", "", -1, this, &TsunamiWindow::onZoomOut);

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
	EventM("hui:close", this, &TsunamiWindow::onExit);
	for (int i=0;i<256;i++)
		EventM(format("jump_to_level_%d", i), this, &TsunamiWindow::onCurLevel);

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

	subscribe(view);
	subscribe(audio);
	subscribe(view->stream, AudioStream::MESSAGE_STATE_CHANGE);
	subscribe(tsunami->clipboard);
	subscribe(bottom_bar);


	if (audio->track.num > 0)
		view->SetCurTrack(audio->track[0]);
	view->OptimizeView();
	HuiRunLaterM(0.5f, view, &AudioView::OptimizeView);

	updateMenu();
}

TsunamiWindow::~TsunamiWindow()
{
	unsubscribe(view);
	unsubscribe(audio);
	unsubscribe(view->stream);
	unsubscribe(tsunami->clipboard);
	unsubscribe(bottom_bar);

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


void TsunamiWindow::onAbout()
{
	HuiAboutBox(this);
}



void TsunamiWindow::onAddTrack()
{
	audio->AddTrack(Track::TYPE_AUDIO);
}

void TsunamiWindow::onAddTimeTrack()
{
	audio->action_manager->BeginActionGroup();
	Track *t = audio->AddTrack(Track::TYPE_TIME);
	if (t)
		t->AddBars(-1, 90, 4, 10);
	audio->action_manager->EndActionGroup();
}

void TsunamiWindow::onAddMidiTrack()
{
	audio->AddTrack(Track::TYPE_MIDI);
}

void TsunamiWindow::onDeleteTrack()
{
	if (audio->track.num < 2){
		tsunami->log->error(_("Es muss mindestens eine Spur existieren"));
		return;
	}

	if (view->cur_track)
		audio->DeleteTrack(get_track_index(view->cur_track));
	else
		tsunami->log->error(_("Keine Spur ausgew&ahlt"));
}

void TsunamiWindow::onTrackEditMidi()
{
	if (view->cur_track)
		bottom_bar->choose(BottomBar::MIDI_EDITOR);
	else
		tsunami->log->error(_("Keine Spur ausgew&ahlt"));
}

void TsunamiWindow::onTrackEditFX()
{
	if (view->cur_track)
		bottom_bar->choose(BottomBar::FX_CONSOLE);
	else
		tsunami->log->error(_("Keine Spur ausgew&ahlt"));
}

void TsunamiWindow::onAudioProperties()
{
	side_bar->choose(SideBar::AUDIO_FILE_DIALOG);
}

void TsunamiWindow::onTrackProperties()
{
	if (view->cur_track)
		side_bar->open(SideBar::TRACK_DIALOG);
	else
		tsunami->log->error(_("Keine Spur ausgew&ahlt"));
}

void TsunamiWindow::onSubProperties()
{
	if (view->cur_sample)
		side_bar->open(SideBar::SUB_DIALOG);
	else
		tsunami->log->error(_("Kein Sample ausgew&ahlt"));
}

void TsunamiWindow::onShowLog()
{
	bottom_bar->choose(BottomBar::LOG_CONSOLE);
}

void TsunamiWindow::onUndo()
{
	audio->action_manager->Undo();
}

void TsunamiWindow::onRedo()
{
	audio->action_manager->Redo();
}

void TsunamiWindow::onSendBugReport()
{
}


string title_filename(const string &filename)
{
	if (filename.num > 0)
		return filename.basename();// + " (" + filename.dirname() + ")";
	return _("Unbenannt");
}

bool TsunamiWindow::allowTermination()
{
	if (audio->action_manager->IsSave())
		return true;
	string answer = HuiQuestionBox(this, _("Frage"), format(_("'%s'\nDatei speichern?"), title_filename(audio->filename).c_str()), true);
	if (answer == "hui:yes"){
		/*if (!OnSave())
			return false;*/
		onSave();
		return true;
	}else if (answer == "hui:no")
		return true;

	// cancel
	return false;
}

void TsunamiWindow::onCopy()
{
	tsunami->clipboard->Copy(view);
}

void TsunamiWindow::onPaste()
{
	tsunami->clipboard->Paste(view);
}

void TsunamiWindow::onFindAndExecutePlugin()
{
	tsunami->plugin_manager->FindAndExecutePlugin();
}

void TsunamiWindow::onDelete()
{
	audio->DeleteSelection(view->cur_level, view->sel_range, false);
}

void TsunamiWindow::onSampleManager()
{
	bottom_bar->choose(BottomBar::SAMPLE_CONSOLE);
}

void TsunamiWindow::onMixingConsole()
{
	bottom_bar->choose(BottomBar::MIXING_CONSOLE);
}

void TsunamiWindow::onFxConsole()
{
	bottom_bar->choose(BottomBar::FX_CONSOLE);
}

void TsunamiWindow::onSubImport()
{
}

void TsunamiWindow::onCommand(const string & id)
{
}

void TsunamiWindow::onSettings()
{
	SettingsDialog *dlg = new SettingsDialog(this, false);
	dlg->Run();
}

void TsunamiWindow::onTrackImport()
{
	if (tsunami->storage->askOpenImport(this)){
		Track *t = audio->AddTrack(Track::TYPE_AUDIO);
		tsunami->storage->loadTrack(t, HuiFilename, view->sel_range.start(), view->cur_level);
	}
}

void TsunamiWindow::onRemoveAdded()
{
	audio->DeleteSelectedSamples();
}

void TsunamiWindow::onPlayLoop()
{
	tsunami->renderer->loop_if_allowed = !tsunami->renderer->loop_if_allowed;
	updateMenu();
}

void TsunamiWindow::onPlay()
{
	tsunami->renderer->Prepare(audio, view->GetPlaybackSelection(), true);
	view->stream->play();
}

void TsunamiWindow::onPause()
{
	view->stream->pause();
}

void TsunamiWindow::onStop()
{
	view->stream->stop();
}

void TsunamiWindow::onInsertAdded()
{
	audio->InsertSelectedSamples(view->cur_level);
}

void TsunamiWindow::onRecord()
{
	CaptureDialog *dlg = new CaptureDialog(this, false, audio);
	dlg->Run();
}

void TsunamiWindow::onAddLevel()
{
	audio->AddLevel("");
}

void TsunamiWindow::onDeleteLevel()
{
	audio->DeleteLevel(view->cur_level, true);
}

void TsunamiWindow::onCurLevel()
{
	view->SetCurLevel(HuiGetEvent()->id.substr(14, -1)._int());
}

void TsunamiWindow::onCurLevelUp()
{
	view->SetCurLevel(view->cur_level + 1);
}

void TsunamiWindow::onCurLevelDown()
{
	view->SetCurLevel(view->cur_level - 1);
}

void TsunamiWindow::onSubFromSelection()
{
	audio->CreateSamplesFromSelection(view->cur_level, view->sel_range);
}

void TsunamiWindow::onViewOptimal()
{
	view->OptimizeView();
}

void TsunamiWindow::onSelectNone()
{
	view->SelectNone();
}

void TsunamiWindow::onSelectAll()
{
	view->SelectAll();
}

void TsunamiWindow::onViewPeaksMax()
{
	view->SetPeaksMode(BufferBox::PEAK_MODE_MAXIMUM);
}

void TsunamiWindow::onViewPeaksMean()
{
	view->SetPeaksMode(BufferBox::PEAK_MODE_SQUAREMEAN);
}

void TsunamiWindow::onViewMono()
{
	view->SetShowMono(true);
}

void TsunamiWindow::onViewStereo()
{
	view->SetShowMono(false);
}

void TsunamiWindow::onZoomIn()
{
	view->ZoomIn();
}

void TsunamiWindow::onZoomOut()
{
	view->ZoomOut();
}

void TsunamiWindow::updateMenu()
{
	msg_db_f("UpdateMenu", 1);
	bool selected = !view->sel_range.empty();
// menu / toolbar
	// edit
	Enable("undo", audio->action_manager->Undoable());
	Enable("redo", audio->action_manager->Redoable());
	Enable("copy", tsunami->clipboard->CanCopy(view));
	Enable("paste", tsunami->clipboard->HasData());
	Enable("delete", selected or (audio->GetNumSelectedSamples() > 0));
	// file
	//Enable("export_selection", true);
	//Enable("wave_properties", true);
	// track
	Enable("delete_track", view->cur_track);
	Enable("track_properties", view->cur_track);
	// level
	Enable("level_delete", audio->level_name.num > 1);
	Enable("level_up", view->cur_level < audio->level_name.num -1);
	Enable("level_down", view->cur_level > 0);
	// sub
	Enable("sample_from_selection", selected);
	Enable("insert_sample", audio->GetNumSelectedSamples() > 0);
	Enable("remove_sample", audio->GetNumSelectedSamples() > 0);
	Enable("sample_properties", view->cur_sample);
	// sound
	Enable("stop", view->stream->isPlaying());
	Enable("pause", view->stream->isPlaying());
	Check("play_loop", tsunami->renderer->loop_if_allowed);
	// view
	Check("show_mixing_console", bottom_bar->isActive(BottomBar::MIXING_CONSOLE));
	Check("show_fx_console", bottom_bar->isActive(BottomBar::FX_CONSOLE));
	Check("sample_manager", bottom_bar->isActive(BottomBar::SAMPLE_CONSOLE));

	HuiMenu *m = GetMenu()->GetSubMenuByID("menu_level_target");
	if (m){
		m->Clear();
		for (int i=0; i<audio->level_name.num; i++)
			m->AddItemCheckable(audio->GetNiceLevelName(i), format("jump_to_level_%d", i));
		Check(format("jump_to_level_%d", view->cur_level), true);
	}

	string title = title_filename(audio->filename) + " - " + AppName;
	if (!audio->action_manager->IsSave())
		title = "*" + title;
	SetTitle(title);
}


void TsunamiWindow::onUpdate(Observable *o, const string &message)
{
	if (o == tsunami->output){
		view->ForceRedraw();
		updateMenu();
	}else // "Clipboard", "AudioFile" or "AudioView"
		updateMenu();
}


void TsunamiWindow::onExit()
{
	if (allowTermination())
		delete(this);
}


void TsunamiWindow::onNew()
{
	if (!allowTermination())
		return;
	NewDialog *d = new NewDialog(this, false, audio);
	d->Run();
}


void TsunamiWindow::onOpen()
{
	if (!allowTermination())
		return;
	if (tsunami->storage->askOpen(this))
		tsunami->storage->load(audio, HuiFilename);
}


void TsunamiWindow::onSave()
{
	if (audio->filename == "")
		onSaveAs();
	else
		tsunami->storage->save(audio, audio->filename);
}


void TsunamiWindow::onSaveAs()
{
	if (tsunami->storage->askSave(this))
		tsunami->storage->save(audio, HuiFilename);
}

void TsunamiWindow::onExport()
{
	if (tsunami->storage->askSaveExport(this))
		tsunami->storage->_export(audio, view->GetPlaybackSelection(), HuiFilename);
}
