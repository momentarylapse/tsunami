/*
 * Tsunami.cpp
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#include "lib/hui/hui.h"
#include "TsunamiWindow.h"
#include "Tsunami.h"
#include "View/Dialog/NewDialog.h"
#include "View/Dialog/CaptureDialog.h"
#include "View/Dialog/SettingsDialog.h"
#include "View/Dialog/MarkerDialog.h"
#include "View/BottomBar/BottomBar.h"
#include "View/BottomBar/MiniBar.h"
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
#include "Audio/SongRenderer.h"
#include "Data/Song.h"

#include "Plugins/FastFourierTransform.h"

extern string AppName;

HuiTimer debug_timer;

TsunamiWindow::TsunamiWindow() :
	Observer("Tsunami"),
	HuiWindow(AppName, -1, -1, 800, 600, NULL, false, HuiWinModeResizable | HuiWinModeControls)
{

	tsunami->win = this;

	copy_multi = false;

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
	HuiAddCommandM("copy_multi", "", -1, this, &TsunamiWindow::onCopyMulti);
	HuiAddCommandM("export_selection", "", KEY_X + KEY_CONTROL, this, &TsunamiWindow::onExport);
	HuiAddCommandM("undo", "hui:undo", KEY_Z + KEY_CONTROL, this, &TsunamiWindow::onUndo);
	HuiAddCommandM("redo", "hui:redo", KEY_Y + KEY_CONTROL, this, &TsunamiWindow::onRedo);
	HuiAddCommandM("add_track", "hui:add", -1, this, &TsunamiWindow::onAddTrack);
	HuiAddCommandM("add_time_track", "hui:add", -1, this, &TsunamiWindow::onAddTimeTrack);
	HuiAddCommandM("add_midi_track", "hui:add", -1, this, &TsunamiWindow::onAddMidiTrack);
	HuiAddCommandM("delete_track", "hui:delete", -1, this, &TsunamiWindow::onDeleteTrack);
	HuiAddCommandM("track_edit_midi", "hui:edit", -1, this, &TsunamiWindow::onTrackEditMidi);
	HuiAddCommandM("track_edit_fx", "hui:edit", -1, this, &TsunamiWindow::onTrackEditFX);
	HuiAddCommandM("track_add_marker", "hui:add", -1, this, &TsunamiWindow::onTrackAddMarker);
	HuiAddCommandM("level_manager", "hui:settings", -1, this, &TsunamiWindow::onLevelManager);
	HuiAddCommandM("level_add", "hui:add", -1, this, &TsunamiWindow::onAddLevel);
	HuiAddCommandM("level_delete", "hui:delete", -1, this, &TsunamiWindow::onDeleteLevel);
	HuiAddCommandM("level_up", "hui:up", -1, this, &TsunamiWindow::onCurLevelUp);
	HuiAddCommandM("level_down", "hui:down", -1, this, &TsunamiWindow::onCurLevelDown);
	HuiAddCommandM("bars_manager", "hui:settings", -1, this, &TsunamiWindow::onBarsManager);
	HuiAddCommandM("song_edit_bars", "hui:settings", -1, this, &TsunamiWindow::onBarsManager);
	HuiAddCommandM("sample_manager", "", -1, this, &TsunamiWindow::onSampleManager);
	HuiAddCommandM("song_edit_samples", "", -1, this, &TsunamiWindow::onSampleManager);
	HuiAddCommandM("show_mixing_console", "", -1, this, &TsunamiWindow::onMixingConsole);
	HuiAddCommandM("show_fx_console", "", -1, this, &TsunamiWindow::onFxConsole);
	HuiAddCommandM("sample_from_selection", "hui:cut", -1, this, &TsunamiWindow::onSampleFromSelection);
	HuiAddCommandM("insert_sample", "", KEY_I + KEY_CONTROL, this, &TsunamiWindow::onInsertSample);
	HuiAddCommandM("remove_sample", "", -1, this, &TsunamiWindow::onRemoveSample);
	HuiAddCommandM("delete_marker", "", -1, this, &TsunamiWindow::onDeleteMarker);
	HuiAddCommandM("edit_marker", "", -1, this, &TsunamiWindow::onEditMarker);
	HuiAddCommandM("track_import", "", -1, this, &TsunamiWindow::onTrackImport);
	HuiAddCommandM("sub_import", "", -1, this, &TsunamiWindow::onSampleImport);
	HuiAddCommandM("song_properties", "", KEY_F4, this, &TsunamiWindow::onSongProperties);
	HuiAddCommandM("track_properties", "", -1, this, &TsunamiWindow::onTrackProperties);
	HuiAddCommandM("sample_properties", "", -1, this, &TsunamiWindow::onSampleProperties);
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
	setSize(width, height);
	setBorderWidth(0);
	addGrid("", 0, 0, 2, 1, "root_table");
	setTarget("root_table", 0);
	addGrid("", 0, 0, 1, 3, "main_table");

	// main table
	setTarget("main_table", 0);
	addDrawingArea("!grabfocus", 0, 0, 0, 0, "area");


	toolbar[0]->setByID("toolbar");
	//ToolbarConfigure(false, true);

	setMenu(HuiCreateResourceMenu("menu"));
	//ToolBarConfigure(true, true);
	setMaximized(maximized);

	// events
	event("hui:close", this, &TsunamiWindow::onExit);
	for (int i=0;i<256;i++)
		event(format("jump_to_level_%d", i), this, &TsunamiWindow::onCurLevel);

	song = tsunami->song;


	view = new AudioView(this, song, tsunami->output);

	// side bar
	side_bar = new SideBar(view, song);
	embed(side_bar, "root_table", 1, 0);

	// bottom bar
	bottom_bar = new BottomBar(view, song, tsunami->output, tsunami->log);
	embed(bottom_bar, "main_table", 0, 1);
	mini_bar = new MiniBar(bottom_bar, view->stream, tsunami->output);
	embed(mini_bar, "main_table", 0, 2);

	subscribe(view);
	subscribe(song);
	subscribe(view->stream, AudioStream::MESSAGE_STATE_CHANGE);
	subscribe(tsunami->clipboard);
	subscribe(bottom_bar);


	if (song->tracks.num > 0)
		view->setCurTrack(song->tracks[0]);
	view->optimizeView();
	HuiRunLaterM(0.5f, view, &AudioView::optimizeView);

	updateMenu();
}

TsunamiWindow::~TsunamiWindow()
{
	unsubscribe(view);
	unsubscribe(song);
	unsubscribe(view->stream);
	unsubscribe(tsunami->clipboard);
	unsubscribe(bottom_bar);

	int w, h;
	getSizeDesired(w, h);
	HuiConfig.setInt("Window.Width", w);
	HuiConfig.setInt("Window.Height", h);
	HuiConfig.setBool("Window.Maximized", isMaximized());
	delete(side_bar);
	delete(mini_bar);
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
	song->addTrack(Track::TYPE_AUDIO);
}

void TsunamiWindow::onAddTimeTrack()
{
	song->action_manager->beginActionGroup();
	song->addTrack(Track::TYPE_TIME);
	// some default data
	for (int i=0; i<10; i++)
		song->addBar(-1, 90, 4, false);
	song->action_manager->endActionGroup();
}

void TsunamiWindow::onAddMidiTrack()
{
	song->addTrack(Track::TYPE_MIDI);
}

void TsunamiWindow::onDeleteTrack()
{
	if (song->tracks.num < 2){
		tsunami->log->error(_("Es muss mindestens eine Spur existieren"));
		return;
	}

	if (view->cur_track)
		song->deleteTrack(get_track_index(view->cur_track));
	else
		tsunami->log->error(_("Keine Spur ausgew&ahlt"));
}

void TsunamiWindow::onTrackEditMidi()
{
	if (view->cur_track)
		side_bar->open(SideBar::MIDI_EDITOR);
	else
		tsunami->log->error(_("Keine Spur ausgew&ahlt"));
}

void TsunamiWindow::onTrackEditFX()
{
	if (view->cur_track)
		side_bar->open(SideBar::FX_CONSOLE);
	else
		tsunami->log->error(_("Keine Spur ausgew&ahlt"));
}

void TsunamiWindow::onTrackAddMarker()
{
	if (view->cur_track){
		MarkerDialog *d = new MarkerDialog(this, false, view->cur_track, view->hover.pos, -1);
		d->run();
	}else
		tsunami->log->error(_("Keine Spur ausgew&ahlt"));
}

void TsunamiWindow::onSongProperties()
{
	side_bar->open(SideBar::SONG_CONSOLE);
}

void TsunamiWindow::onTrackProperties()
{
	if (view->cur_track)
		side_bar->open(SideBar::TRACK_CONSOLE);
	else
		tsunami->log->error(_("Keine Spur ausgew&ahlt"));
}

void TsunamiWindow::onSampleProperties()
{
	if (view->cur_sample)
		side_bar->open(SideBar::SAMPLEREF_DIALOG);
	else
		tsunami->log->error(_("Kein Sample ausgew&ahlt"));
}

void TsunamiWindow::onDeleteMarker()
{
	if (view->selection.type == Selection::TYPE_MARKER)
		view->cur_track->deleteMarker(view->selection.index);
	else
		tsunami->log->error(_("Kein Marker ausgew&ahlt"));
}

void TsunamiWindow::onEditMarker()
{
	if (view->selection.type == Selection::TYPE_MARKER){
		MarkerDialog *d = new MarkerDialog(this, false, view->cur_track, -1, view->selection.index);
		d->run();
	}else
		tsunami->log->error(_("Kein Marker ausgew&ahlt"));
}

void TsunamiWindow::onShowLog()
{
	bottom_bar->open(BottomBar::LOG_CONSOLE);
}

void TsunamiWindow::onUndo()
{
	song->action_manager->undo();
}

void TsunamiWindow::onRedo()
{
	song->action_manager->redo();
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
	if (song->action_manager->isSave())
		return true;
	string answer = HuiQuestionBox(this, _("Frage"), format(_("'%s'\nDatei speichern?"), title_filename(song->filename).c_str()), true);
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
	if (copy_multi)
		tsunami->clipboard->copy_from_selected_tracks(view);
	else
		tsunami->clipboard->copy_from_track(view->cur_track, view);
}

void TsunamiWindow::onPaste()
{
	tsunami->clipboard->paste(view);
}

void TsunamiWindow::onCopyMulti()
{
	copy_multi = isChecked("");
}

void TsunamiWindow::onFindAndExecutePlugin()
{
	tsunami->plugin_manager->FindAndExecutePlugin();
}

void TsunamiWindow::onDelete()
{
	song->deleteSelection(view->cur_level, view->sel_range, false);
}

void TsunamiWindow::onSampleManager()
{
	side_bar->open(SideBar::SAMPLE_CONSOLE);
}

void TsunamiWindow::onMixingConsole()
{
	bottom_bar->open(BottomBar::MIXING_CONSOLE);
}

void TsunamiWindow::onFxConsole()
{
	side_bar->open(SideBar::FX_CONSOLE);
}

void TsunamiWindow::onSampleImport()
{
}

void TsunamiWindow::onCommand(const string & id)
{
}

void TsunamiWindow::onSettings()
{
	SettingsDialog *dlg = new SettingsDialog(this, false);
	dlg->run();
}

void TsunamiWindow::onTrackImport()
{
	if (tsunami->storage->askOpenImport(this)){
		Track *t = song->addTrack(Track::TYPE_AUDIO);
		tsunami->storage->loadTrack(t, HuiFilename, view->sel_range.start(), view->cur_level);
	}
}

void TsunamiWindow::onRemoveSample()
{
	song->deleteSelectedSamples();
}

void TsunamiWindow::onPlayLoop()
{
	view->renderer->loop_if_allowed = !view->renderer->loop_if_allowed;
	updateMenu();
}

void TsunamiWindow::onPlay()
{
	view->renderer->prepare(song, view->getPlaybackSelection(), true);
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

void TsunamiWindow::onInsertSample()
{
	song->insertSelectedSamples(view->cur_level);
}

void TsunamiWindow::onRecord()
{
	CaptureDialog *dlg = new CaptureDialog(this, false, song);
	dlg->run();
}

void TsunamiWindow::onAddLevel()
{
	song->addLevel("");
}

void TsunamiWindow::onDeleteLevel()
{
	song->deleteLevel(view->cur_level, true);
}

void TsunamiWindow::onCurLevel()
{
	view->setCurLevel(HuiGetEvent()->id.substr(14, -1)._int());
}

void TsunamiWindow::onCurLevelUp()
{
	view->setCurLevel(view->cur_level + 1);
}

void TsunamiWindow::onCurLevelDown()
{
	view->setCurLevel(view->cur_level - 1);
}

void TsunamiWindow::onLevelManager()
{
	side_bar->open(SideBar::LEVEL_CONSOLE);
}

void TsunamiWindow::onBarsManager()
{
	side_bar->open(SideBar::BARS_CONSOLE);
}

void TsunamiWindow::onSampleFromSelection()
{
	song->createSamplesFromSelection(view->cur_level, view->sel_range);
}

void TsunamiWindow::onViewOptimal()
{
	view->optimizeView();
}

void TsunamiWindow::onSelectNone()
{
	view->selectNone();
}

void TsunamiWindow::onSelectAll()
{
	view->selectAll();
}

void TsunamiWindow::onViewPeaksMax()
{
	view->setPeaksMode(BufferBox::PEAK_MODE_MAXIMUM);
}

void TsunamiWindow::onViewPeaksMean()
{
	view->setPeaksMode(BufferBox::PEAK_MODE_SQUAREMEAN);
}

void TsunamiWindow::onViewMono()
{
	view->setShowMono(true);
}

void TsunamiWindow::onViewStereo()
{
	view->setShowMono(false);
}

void TsunamiWindow::onZoomIn()
{
	view->zoomIn();
}

void TsunamiWindow::onZoomOut()
{
	view->zoomOut();
}

void TsunamiWindow::updateMenu()
{
	msg_db_f("UpdateMenu", 1);
	bool selected = !view->sel_range.empty();
// menu / toolbar
	// edit
	enable("undo", song->action_manager->undoable());
	enable("redo", song->action_manager->redoable());
	enable("copy", tsunami->clipboard->canCopy(view));
	enable("paste", tsunami->clipboard->hasData());
	enable("delete", selected or (song->getNumSelectedSamples() > 0));
	// file
	//Enable("export_selection", true);
	//Enable("wave_properties", true);
	// track
	enable("delete_track", view->cur_track);
	enable("track_properties", view->cur_track);
	// level
	enable("level_delete", song->level_names.num > 1);
	enable("level_up", view->cur_level < song->level_names.num -1);
	enable("level_down", view->cur_level > 0);
	// sub
	enable("sample_from_selection", selected);
	enable("insert_sample", song->getNumSelectedSamples() > 0);
	enable("remove_sample", song->getNumSelectedSamples() > 0);
	enable("sample_properties", view->cur_sample);
	// sound
	enable("stop", view->stream->isPlaying());
	enable("pause", view->stream->isPlaying());
	check("play_loop", view->renderer->loop_if_allowed);
	// view
	check("show_mixing_console", bottom_bar->isActive(BottomBar::MIXING_CONSOLE));
	check("show_fx_console", side_bar->isActive(SideBar::FX_CONSOLE));
	check("sample_manager", side_bar->isActive(SideBar::SAMPLE_CONSOLE));

	HuiMenu *m = getMenu()->getSubMenuByID("menu_level_target");
	if (m){
		m->clear();
		for (int i=0; i<song->level_names.num; i++)
			m->addItemCheckable(song->getNiceLevelName(i), format("jump_to_level_%d", i));
		check(format("jump_to_level_%d", view->cur_level), true);
	}

	string title = title_filename(song->filename) + " - " + AppName;
	if (!song->action_manager->isSave())
		title = "*" + title;
	setTitle(title);
}


void TsunamiWindow::onUpdate(Observable *o, const string &message)
{
	// "Clipboard", "AudioFile" or "AudioView"
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
	NewDialog *d = new NewDialog(this, false, song);
	d->run();
}


void TsunamiWindow::onOpen()
{
	if (!allowTermination())
		return;
	if (tsunami->storage->askOpen(this))
		tsunami->storage->load(song, HuiFilename);
}


void TsunamiWindow::onSave()
{
	if (song->filename == "")
		onSaveAs();
	else
		tsunami->storage->save(song, song->filename);
}


void TsunamiWindow::onSaveAs()
{
	if (tsunami->storage->askSave(this))
		tsunami->storage->save(song, HuiFilename);
}

void TsunamiWindow::onExport()
{
	if (tsunami->storage->askSaveExport(this))
		tsunami->storage->_export(song, view->getPlaybackSelection(), HuiFilename);
}
