/*
 * Tsunami.cpp
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#include "TsunamiWindow.h"
#include "Session.h"

#include "Module/Audio/SongRenderer.h"
#include "Tsunami.h"
#include "View/Dialog/NewDialog.h"
#include "View/Dialog/SettingsDialog.h"
#include "View/Dialog/MarkerDialog.h"
#include "View/Dialog/BarAddDialog.h"
#include "View/Dialog/BarEditDialog.h"
#include "View/Dialog/PauseAddDialog.h"
#include "View/Dialog/PauseEditDialog.h"
#include "View/BottomBar/BottomBar.h"
#include "View/BottomBar/MiniBar.h"
//#include "View/BottomBar/DeviceConsole.h"
#include "View/SideBar/SideBar.h"
#include "View/SideBar/CaptureConsole.h"
#include "View/Mode/ViewModeDefault.h"
#include "View/Mode/ViewModeScaleBars.h"
#include "View/Helper/Slider.h"
#include "View/Helper/Progress.h"
#include "View/AudioView.h"
#include "Plugins/PluginManager.h"
#include "Plugins/TsunamiPlugin.h"
#include "Plugins/SongPlugin.h"
#include "Storage/Storage.h"
#include "Stuff/Log.h"
#include "Stuff/Clipboard.h"
#include "Stuff/BackupManager.h"
#include "Device/DeviceManager.h"
#include "Data/Song.h"
#include "Data/SongSelection.h"
#include "Action/Track/Buffer/ActionTrackEditBuffer.h"
#include "Data/Rhythm/Bar.h"
#include "Module/Audio/AudioEffect.h"
#include "Module/Midi/MidiEffect.h"
#include "Plugins/FastFourierTransform.h"
#include "View/Helper/PeakMeterDisplay.h"
#include "lib/hui/hui.h"

extern const string AppName;

namespace hui{
	extern string file_dialog_default;
}


hui::Timer debug_timer;

TsunamiWindow::TsunamiWindow(Session *_session) :
	hui::Window(AppName, 800, 600)
{
	session = _session;
	session->setWin(this);
	song = session->song;
	app = tsunami;

	int width = hui::Config.getInt("Window.Width", 800);
	int height = hui::Config.getInt("Window.Height", 600);
	bool maximized = hui::Config.getBool("Window.Maximized", true);

	event("new", std::bind(&TsunamiWindow::onNew, this));
	setKeyCode("new", hui::KEY_N + hui::KEY_CONTROL, "hui:new");
	event("open", std::bind(&TsunamiWindow::onOpen, this));
	setKeyCode("open", hui::KEY_O + hui::KEY_CONTROL, "hui:open");
	event("save", std::bind(&TsunamiWindow::onSave, this));
	setKeyCode("save", hui::KEY_S + hui::KEY_CONTROL, "hui:save");
	event("save_as", std::bind(&TsunamiWindow::onSaveAs, this));
	setKeyCode("save_as", hui::KEY_S + hui::KEY_CONTROL + hui::KEY_SHIFT, "hui:save-as");
	event("copy", std::bind(&TsunamiWindow::onCopy, this));
	setKeyCode("copy", hui::KEY_C + hui::KEY_CONTROL, "hui:copy");
	event("paste", std::bind(&TsunamiWindow::onPaste, this));
	setKeyCode("paste", hui::KEY_V + hui::KEY_CONTROL, "hui:paste");
	event("paste_as_samples", std::bind(&TsunamiWindow::onPasteAsSamples, this));
	setKeyCode("paste_as_samples", hui::KEY_V + hui::KEY_CONTROL + hui::KEY_SHIFT, "hui:paste");
	event("delete", std::bind(&TsunamiWindow::onDelete, this));
	setKeyCode("delete", hui::KEY_DELETE, "hui:delete");
	event("export_selection", std::bind(&TsunamiWindow::onExport, this));
	setKeyCode("export_selection", hui::KEY_X + hui::KEY_CONTROL, "");
	event("undo", std::bind(&TsunamiWindow::onUndo, this));
	setKeyCode("undo", hui::KEY_Z + hui::KEY_CONTROL, "hui:undo");
	event("redo", std::bind(&TsunamiWindow::onRedo, this));
	setKeyCode("redo", hui::KEY_Y + hui::KEY_CONTROL, "hui:redo");
	event("track_render", std::bind(&TsunamiWindow::onTrackRender, this));
	event("add_track", std::bind(&TsunamiWindow::onAddTrack, this));
	setKeyCode("add_track", -1, "hui:add");
	event("add_time_track", std::bind(&TsunamiWindow::onAddTimeTrack, this));
	setKeyCode("add_time_track", -1, "hui:add");
	event("add_midi_track", std::bind(&TsunamiWindow::onAddMidiTrack, this));
	setKeyCode("add_midi_track", -1, "hui:add");
	event("delete_track", std::bind(&TsunamiWindow::onDeleteTrack, this));
	setKeyCode("delete_track", -1, "hui:delete");
	event("track_edit_midi", std::bind(&TsunamiWindow::onTrackEditMidi, this));
	setKeyCode("track_edit_midi", -1, "hui:edit");
	event("track_edit_fx", std::bind(&TsunamiWindow::onTrackEditFX, this));
	setKeyCode("track_edit_fx", -1, "hui:edit");
	event("track_add_marker", std::bind(&TsunamiWindow::onTrackAddMarker, this));
	setKeyCode("track_add_marker", -1, "hui:add");
	event("layer_add", std::bind(&TsunamiWindow::onAddLayer, this));
	setKeyCode("layer_add", -1, "hui:add");
	event("delete_layer", std::bind(&TsunamiWindow::onDeleteLayer, this));
	setKeyCode("delete_layer", -1, "hui:delete");
	event("add_bars", std::bind(&TsunamiWindow::onAddBars, this));
	setKeyCode("add_bars", -1, "hui:add");
	event("add_pause", std::bind(&TsunamiWindow::onAddPause, this));
	setKeyCode("add_pause", -1, "hui:add");
	event("delete_bars", std::bind(&TsunamiWindow::onDeleteBars, this));
	setKeyCode("delete_bars", -1, "hui:delete");
	event("delete_time", std::bind(&TsunamiWindow::onDeleteTimeInterval, this));
	setKeyCode("delete_time", -1, "hui:delete");
	event("insert_time", std::bind(&TsunamiWindow::onInsertTimeInterval, this));
	setKeyCode("insert_time", -1, "hui:add");
	event("edit_bars", std::bind(&TsunamiWindow::onEditBars, this));
	setKeyCode("edit_bars", -1, "hui:edit");
	event("scale_bars", std::bind(&TsunamiWindow::onScaleBars, this));
	setKeyCode("scale_bars", -1, "hui:scale");
	event("bar_link_to_data", std::bind(&TsunamiWindow::onBarsModifyMidi, this));
	event("sample_manager", std::bind(&TsunamiWindow::onSampleManager, this));
	event("song_edit_samples", std::bind(&TsunamiWindow::onSampleManager, this));
	event("show_mixing_console", std::bind(&TsunamiWindow::onMixingConsole, this));
	event("show_fx_console", std::bind(&TsunamiWindow::onFxConsole, this));
	event("sample_from_selection", std::bind(&TsunamiWindow::onSampleFromSelection, this));
	setKeyCode("sample_from_selection", -1, "hui:cut");
	event("insert_sample", std::bind(&TsunamiWindow::onInsertSample, this));
	setKeyCode("insert_sample", hui::KEY_I + hui::KEY_CONTROL, "");
	event("remove_sample", std::bind(&TsunamiWindow::onRemoveSample, this));
	event("delete_marker", std::bind(&TsunamiWindow::onDeleteMarker, this));
	event("edit_marker", std::bind(&TsunamiWindow::onEditMarker, this));
	event("track_import", std::bind(&TsunamiWindow::onTrackImport, this));
	event("sub_import", std::bind(&TsunamiWindow::onSampleImport, this));
	event("song_properties", std::bind(&TsunamiWindow::onSongProperties, this));
	setKeyCode("song_properties", hui::KEY_F4, "");
	event("track_properties", std::bind(&TsunamiWindow::onTrackProperties, this));
	event("sample_properties", std::bind(&TsunamiWindow::onSampleProperties, this));
	event("settings", std::bind(&TsunamiWindow::onSettings, this));
	event("play", std::bind(&TsunamiWindow::onPlay, this));
	setKeyCode("play", -1, "hui:media-play");
	event("play_loop", std::bind(&TsunamiWindow::onPlayLoop, this));
	event("pause", std::bind(&TsunamiWindow::onPause, this));
	setKeyCode("pause", -1, "hui:media-pause");
	event("stop", std::bind(&TsunamiWindow::onStop, this));
	setKeyCode("stop", -1, "hui:media-stop");
	event("record", std::bind(&TsunamiWindow::onRecord, this));
	setKeyCode("record", -1, "hui:media-record");
	event("show_log", std::bind(&TsunamiWindow::onShowLog, this));
	event("about", std::bind(&TsunamiWindow::onAbout, this));
	setKeyCode("run_plugin", hui::KEY_RETURN + hui::KEY_SHIFT, "hui:execute");
	event("exit", std::bind(&TsunamiWindow::onExit, this));
	setKeyCode("exit", hui::KEY_Q + hui::KEY_CONTROL, "hui:quit");
	event("select_all", std::bind(&TsunamiWindow::onSelectAll, this));
	setKeyCode("select_all", hui::KEY_A + hui::KEY_CONTROL, "");
	event("select_nothing", std::bind(&TsunamiWindow::onSelectNone, this));
	event("select_expand", std::bind(&TsunamiWindow::onSelectExpand, this));
	event("view_midi_default", std::bind(&TsunamiWindow::onViewMidiDefault, this));
	event("view_midi_tab", std::bind(&TsunamiWindow::onViewMidiTab, this));
	event("view_midi_score", std::bind(&TsunamiWindow::onViewMidiScore, this));
	event("view_optimal", std::bind(&TsunamiWindow::onViewOptimal, this));
	event("zoom_in", std::bind(&TsunamiWindow::onZoomIn, this));
	event("zoom_out", std::bind(&TsunamiWindow::onZoomOut, this));

	// table structure
	setSize(width, height);
	setBorderWidth(0);
	addGrid("", 0, 0, "root_table");
	setTarget("root_table");
	addGrid("", 0, 0, "main_table");

	// main table
	setTarget("main_table");
	addDrawingArea("!grabfocus", 0, 0, "area");


	toolbar[0]->setByID("toolbar");
	//ToolbarConfigure(false, true);

	setMenu(hui::CreateResourceMenu("menu"));
	//ToolBarConfigure(true, true);
	setMaximized(maximized);


	app->plugin_manager->AddPluginsToMenu(this);

	// events
	event("hui:close", std::bind(&TsunamiWindow::onExit, this));

	auto_delete = false;


	view = new AudioView(session, "area");
	session->view = view;

	// side bar
	side_bar = new SideBar(session);
	embed(side_bar, "root_table", 1, 0);

	// bottom bar
	bottom_bar = new BottomBar(session);
	embed(bottom_bar, "main_table", 0, 1);
	mini_bar = new MiniBar(bottom_bar, session);
	embed(mini_bar, "main_table", 0, 2);

	view->subscribe(this, std::bind(&TsunamiWindow::onUpdate, this));
	song->subscribe(this, std::bind(&TsunamiWindow::onUpdate, this));
	app->clipboard->subscribe(this, std::bind(&TsunamiWindow::onUpdate, this));
	bottom_bar->subscribe(this, std::bind(&TsunamiWindow::onBottomBarUpdate, this));
	side_bar->subscribe(this, std::bind(&TsunamiWindow::onSideBarUpdate, this));



	song->newWithOneTrack(Track::Type::AUDIO, DEFAULT_SAMPLE_RATE);
	song->notify(song->MESSAGE_FINISHED_LOADING);

	updateMenu();
}

TsunamiWindow::~TsunamiWindow()
{
	// all done by onDestroy()
}

void TsunamiCleanUp()
{
	bool again = false;
	do{
		again = false;
		foreachi(Session *s, tsunami->sessions, i)
			if (s->win->gotDestroyed() and s->win->auto_delete){
				delete(s->win);
				delete(s);
				tsunami->sessions.erase(i);
				again = true;
				break;
			}
	}while(again);

	if (tsunami->sessions.num == 0)
		tsunami->end();
}

void TsunamiWindow::onDestroy()
{
	int w, h;
	getSizeDesired(w, h);
	hui::Config.setInt("Window.Width", w);
	hui::Config.setInt("Window.Height", h);
	hui::Config.setBool("Window.Maximized", isMaximized());

	view->unsubscribe(this);
	song->unsubscribe(this);
	app->clipboard->unsubscribe(this);
	bottom_bar->unsubscribe(this);
	side_bar->unsubscribe(this);

	delete(side_bar);
	delete(mini_bar);
	delete(bottom_bar);
	delete(view);

	hui::RunLater(0.010f, &TsunamiCleanUp);
}


void TsunamiWindow::onAbout()
{
	hui::AboutBox(this);
}



void TsunamiWindow::onAddTrack()
{
	song->addTrack(Track::Type::AUDIO);
}

void TsunamiWindow::onAddTimeTrack()
{
	song->action_manager->beginActionGroup();
	try{
		song->addTrack(Track::Type::TIME, 0);
		// some default data
		for (int i=0; i<10; i++)
			song->addBar(-1, 90, 4, 1, false);
	}catch(Song::Exception &e){
		session->e(e.message);
	}
	song->action_manager->endActionGroup();
}

void TsunamiWindow::onAddMidiTrack()
{
	song->addTrack(Track::Type::MIDI);
}

void TsunamiWindow::onTrackRender()
{
	Range range = view->sel.range;
	if (range.empty()){
		session->e(_("Selection range is empty"));
		return;
	}
	song->action_manager->beginActionGroup();
	Track *t = song->addTrack(Track::Type::AUDIO);

	SongRenderer renderer(song);
	renderer.prepare(range, false);
	renderer.allow_tracks(view->get_selected_tracks());
	AudioBuffer buf = t->layers[0]->getBuffers(range);

	ActionTrackEditBuffer *a = new ActionTrackEditBuffer(t->layers[0], range);
	renderer.read(buf);
	song->execute(a);
	song->action_manager->endActionGroup();

}

void TsunamiWindow::onDeleteTrack()
{
	if (view->cur_track){
		try{
			song->deleteTrack(view->cur_track);
		}catch(Song::Exception &e){
			session->e(e.message);
		}
	}else{
		session->e(_("No track selected"));
	}
}

void TsunamiWindow::onTrackEditMidi()
{
	if (view->cur_track)
		side_bar->open(SideBar::MIDI_EDITOR_CONSOLE);
	else
		session->e(_("No track selected"));
}

void TsunamiWindow::onTrackEditFX()
{
	if (view->cur_track)
		side_bar->open(SideBar::FX_CONSOLE);
	else
		session->e(_("No track selected"));
}

void TsunamiWindow::onTrackAddMarker()
{
	if (view->hover.track){
		Range range = view->sel.range;
		if (!range.is_inside(view->hover.pos))
			range = Range(view->hover.pos, 0);
		MarkerDialog *dlg = new MarkerDialog(this, view->hover.track, range, NULL);
		dlg->run();
		delete(dlg);
	}else{
		session->e(_("No track selected"));
	}
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
		session->e(_("No track selected"));
}

void TsunamiWindow::onSampleProperties()
{
	if (view->cur_sample)
		side_bar->open(SideBar::SAMPLEREF_CONSOLE);
	else
		session->e(_("No sample selected"));
}

void TsunamiWindow::onDeleteMarker()
{
	if (view->hover.type == Selection::Type::MARKER)
		view->cur_track->deleteMarker(view->hover.marker);
	else
		session->e(_("No marker selected"));
}

void TsunamiWindow::onEditMarker()
{
	if (view->hover.type == Selection::Type::MARKER){
		MarkerDialog *dlg = new MarkerDialog(this, view->cur_track, Range::EMPTY, view->hover.marker);
		dlg->run();
		delete(dlg);
	}else
		session->e(_("No marker selected"));
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
	return _("No name");
}

bool TsunamiWindow::allowTermination()
{
	if (side_bar->isActive(SideBar::CAPTURE_CONSOLE)){
		if (side_bar->capture_console->isCapturing()){
			string answer = hui::QuestionBox(this, _("Question"), _("Cancel recording?"), true);
			if (answer != "hui:yes")
				return false;
			side_bar->capture_console->onDump();
			side_bar->_hide();
		}
	}

	if (song->action_manager->isSave())
		return true;
	string answer = hui::QuestionBox(this, _("Question"), format(_("'%s'\nSave file?"), title_filename(song->filename).c_str()), true);
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
	app->clipboard->copy(view);
}

void TsunamiWindow::onPaste()
{
	app->clipboard->paste(view);
}

void TsunamiWindow::onPasteAsSamples()
{
	app->clipboard->pasteAsSamples(view);
}

void TsunamiWindow::onMenuExecuteEffect()
{
	string name = hui::GetEvent()->id.explode("--")[1];

	AudioEffect *fx = CreateAudioEffect(session, name);

	fx->reset_config();
	if (fx->configure(this)){
		song->action_manager->beginActionGroup();
		for (Track *t: song->tracks)
			for (TrackLayer *l: t->layers)
				if (view->sel.has(l) and (t->type == t->Type::AUDIO)){
					fx->reset_state();
					fx->do_process_track(l, view->sel.range);
				}
		song->action_manager->endActionGroup();
	}
	delete(fx);
}

void TsunamiWindow::onMenuExecuteMidiEffect()
{
	string name = hui::GetEvent()->id.explode("--")[1];

	MidiEffect *fx = CreateMidiEffect(session, name);

	fx->reset_config();
	if (fx->configure(this)){
		song->action_manager->beginActionGroup();
		for (Track *t : song->tracks)
			for (TrackLayer *l : t->layers)
			if (view->sel.has(l) and (t->type == t->Type::MIDI)){
				fx->reset_state();
				fx->process_layer(l, view->sel);
			}
		song->action_manager->endActionGroup();
	}
	delete(fx);
}

void TsunamiWindow::onMenuExecuteSongPlugin()
{
	string name = hui::GetEvent()->id.explode("--")[1];

	SongPlugin *p = CreateSongPlugin(session, name);

	p->apply();
	delete(p);
}

void TsunamiWindow::onMenuExecuteTsunamiPlugin()
{
	string name = hui::GetEvent()->id.explode("--")[1];

	session->executeTsunamiPlugin(name);
}

void TsunamiWindow::onDelete()
{
	if (!view->sel.is_empty())
		song->deleteSelection(view->sel);
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
	SettingsDialog *dlg = new SettingsDialog(view, this);
	dlg->run();
	delete(dlg);
}

void TsunamiWindow::onTrackImport()
{
	if (session->storage->askOpenImport(this)){
		Track *t = song->addTrack(Track::Type::AUDIO);
		session->storage->loadTrack(view->cur_layer, hui::Filename, view->sel.range.start());
	}
}

void TsunamiWindow::onRemoveSample()
{
	song->deleteSelectedSamples(view->sel);
}

void TsunamiWindow::onPlayLoop()
{
	view->renderer->loop_if_allowed = !view->renderer->loop_if_allowed;
	updateMenu();
}

void TsunamiWindow::onPlay()
{
	if (side_bar->isActive(SideBar::CAPTURE_CONSOLE))
		return;
	if (view->isPaused())
		view->pause(false);
	else
		view->play(view->getPlaybackSelection(false), true);
}

void TsunamiWindow::onPause()
{
	if (side_bar->isActive(SideBar::CAPTURE_CONSOLE))
		return;
	view->pause(true);
}

void TsunamiWindow::onStop()
{
	if (side_bar->isActive(SideBar::CAPTURE_CONSOLE))
		side_bar->_hide();
	else
		view->stop();
}

void TsunamiWindow::onInsertSample()
{
	song->insertSelectedSamples(view->sel);
}

void TsunamiWindow::onRecord()
{
	side_bar->open(SideBar::CAPTURE_CONSOLE);
}

void TsunamiWindow::onAddLayer()
{
	view->cur_track->addLayer(false);
}

void TsunamiWindow::onDeleteLayer()
{
	view->cur_layer->track->deleteLayer(view->cur_layer);
}

void TsunamiWindow::onSampleFromSelection()
{
	song->createSamplesFromSelection(view->sel);
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

void TsunamiWindow::onSelectExpand()
{
	view->selectExpand();
}

void TsunamiWindow::onViewMidiDefault()
{
	view->setMidiViewMode(view->MidiMode::LINEAR);
}

void TsunamiWindow::onViewMidiTab()
{
	view->setMidiViewMode(view->MidiMode::TAB);
}

void TsunamiWindow::onViewMidiScore()
{
	view->setMidiViewMode(view->MidiMode::CLASSICAL);
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
// menu / toolbar
	// edit
	enable("undo", song->action_manager->undoable());
	enable("redo", song->action_manager->redoable());
	enable("copy", app->clipboard->canCopy(view));
	enable("paste", app->clipboard->hasData());
	enable("delete", !view->sel.is_empty());
	// file
	//Enable("export_selection", true);
	//Enable("wave_properties", true);
	// track
	enable("delete_track", view->cur_track);
	enable("track_properties", view->cur_track);
	// layer
/*	enable("layer_delete", song->layers.num > 1);
	enable("layer_up", view->cur_layer < song->layers.num -1);
	enable("layer_down", view->cur_layer > 0);*/
	// bars
	enable("delete_time", !view->sel.range.empty());
	enable("delete_bars", view->sel.bars.num > 0);
	enable("edit_bars", view->sel.bars.num > 0);
	enable("scale_bars", view->sel.bars.num > 0);
	check("bar_link_to_data", view->bars_edit_data);
	// sample
	enable("sample_from_selection", !view->sel.range.empty());
	enable("insert_sample", view->sel.num_samples() > 0);
	enable("remove_sample", view->sel.num_samples() > 0);
	enable("sample_properties", view->cur_sample);
	// sound
	enable("play", !side_bar->isActive(SideBar::CAPTURE_CONSOLE));
	enable("stop", view->isPlaybackActive() or side_bar->isActive(SideBar::CAPTURE_CONSOLE));
	enable("pause", view->isPlaybackActive() and !view->isPaused());
	check("play_loop", view->renderer->loop_if_allowed);
	enable("record", !side_bar->isActive(SideBar::CAPTURE_CONSOLE));
	// view
	check("show_mixing_console", bottom_bar->isActive(BottomBar::MIXING_CONSOLE));
	check("show_fx_console", side_bar->isActive(SideBar::FX_CONSOLE));
	check("sample_manager", side_bar->isActive(SideBar::SAMPLE_CONSOLE));

	string title = title_filename(song->filename) + " - " + AppName;
	if (!song->action_manager->isSave())
		title = "*" + title;
	setTitle(title);
}

void TsunamiWindow::onSideBarUpdate()
{
	if (!side_bar->visible)
		activate(view->id);
	updateMenu();
}

void TsunamiWindow::onBottomBarUpdate()
{
	if (!bottom_bar->visible)
		activate(view->id);
	updateMenu();
}

void TsunamiWindow::onUpdate()
{
	// "Clipboard", "AudioFile" or "AudioView"
	updateMenu();
}


void TsunamiWindow::onExit()
{
	if (allowTermination()){
		BackupManager::set_save_state(session);
		destroy();
	}
}


void TsunamiWindow::onNew()
{
	NewDialog *dlg = new NewDialog(this);
	dlg->run();
	delete(dlg);
	//BackupManager::set_save_state();
}


void TsunamiWindow::onOpen()
{
	if (session->storage->askOpen(this)){
		if (song->is_empty()){
			session->storage->load(song, hui::Filename);
			BackupManager::set_save_state(session);
		}else{
			Session *s = tsunami->createSession();
			s->win->show();
			s->storage->load(s->song, hui::Filename);
		}
	}
}


void TsunamiWindow::onSave()
{
	if (song->filename == "")
		onSaveAs();
	else{
		session->storage->save(song, song->filename);
		BackupManager::set_save_state(session);
	}
}

string _suggest_filename(Song *s, const string &dir)
{
	if (s->filename != "")
		return s->filename.basename();
	string base = get_current_date().format("%Y-%m-%d");

	string ext = "nami";
	if ((s->tracks.num == 1) and (s->tracks[0]->type == Track::Type::AUDIO))
		ext = "ogg";
	bool allow_midi = true;
	for (Track* t: s->tracks)
		if ((t->type != Track::Type::MIDI) and (t->type != Track::Type::TIME))
			allow_midi = false;
	if (allow_midi)
		ext = "midi";

	for (int i=0; i<26; i++){
		string name = base + "a." + ext;
		name[name.num - ext.num - 2] += i;
		msg_write(dir + name);
		if (!file_test_existence(dir + name))
			return name;
	}
	return "";
}

void TsunamiWindow::onSaveAs()
{
	if (song->filename == "")
		hui::file_dialog_default = _suggest_filename(song, session->storage->current_directory);

	if (session->storage->askSave(this))
		session->storage->save(song, hui::Filename);

	hui::file_dialog_default = "";
}

void TsunamiWindow::onExport()
{
	if (session->storage->askSaveExport(this)){
		SongRenderer renderer(song);
		renderer.prepare(view->getPlaybackSelection(false), false);
		renderer.allow_tracks(view->get_selected_tracks());
		session->storage->saveViaRenderer(renderer.out, hui::Filename, renderer.get_num_samples(), song->tags);
	}
}

int pref_bar_index(AudioView *view)
{
	if (view->sel.bar_gap >= 0)
		return view->sel.bar_gap;
	if (!view->sel.bar_indices.empty())
		return view->sel.bar_indices.end() + 1;
	if (view->hover.pos > 0)
		return view->song->bars.num;
	return 0;
}

void TsunamiWindow::onAddBars()
{
	auto dlg = new BarAddDialog(win, song, view, pref_bar_index(view));
	dlg->run();
	delete(dlg);
}

void TsunamiWindow::onAddPause()
{
	auto dlg = new PauseAddDialog(win, song, view, pref_bar_index(view));
	dlg->run();
	delete(dlg);
}

void TsunamiWindow::onDeleteBars()
{
	song->action_manager->beginActionGroup();

	for (int i=view->sel.bar_indices.end()-1; i>=view->sel.bar_indices.start(); i--){
		song->deleteBar(i, view->bars_edit_data);
	}
	song->action_manager->endActionGroup();
}

void TsunamiWindow::onDeleteTimeInterval()
{
	hui::ErrorBox(this, "todo", "todo");
	/*song->action_manager->beginActionGroup();

	for (int i=view->sel.bars.end()-1; i>=view->sel.bars.start(); i--){
		song->deleteBar(i, view->bars_edit_data);
	}
	song->action_manager->endActionGroup();*/
}

void TsunamiWindow::onInsertTimeInterval()
{
	hui::ErrorBox(this, "todo", "todo");
	/*song->action_manager->beginActionGroup();

	for (int i=view->sel.bars.end()-1; i>=view->sel.bars.start(); i--){
		song->deleteBar(i, view->bars_edit_data);
	}
	song->action_manager->endActionGroup();*/
}

void TsunamiWindow::onEditBars()
{
	if (view->sel.bars.num == 0){
		return;
	}
	int num_bars = 0;
	int num_pauses = 0;
	for (int i=view->sel.bar_indices.offset; i<view->sel.bar_indices.end(); i++)
		if (song->bars[i]->is_pause())
			num_pauses ++;
		else
			num_bars ++;
	if (num_bars > 0 and num_pauses == 0){
		hui::Dialog *dlg = new BarEditDialog(win, song, view->sel.bar_indices, view->bars_edit_data);
		dlg->run();
		delete(dlg);
	}else if (num_bars == 0 and num_pauses == 1){
		hui::Dialog *dlg = new PauseEditDialog(win, song, view->sel.bar_indices.start(), view->bars_edit_data);
		dlg->run();
		delete(dlg);
	}else{
		hui::ErrorBox(this, _("Error"), _("Can only edit bars or a single pause at a time."));
	}
}

void TsunamiWindow::onScaleBars()
{
	view->setMode(view->mode_scale_bars);
	Set<int> s;
	for (int i=view->sel.bar_indices.start(); i<view->sel.bar_indices.end(); i++)
		s.add(i);
	view->mode_scale_bars->startScaling(s);
}

void TsunamiWindow::onBarsModifyMidi()
{
	view->bars_edit_data = ! view->bars_edit_data;
	updateMenu();
}
