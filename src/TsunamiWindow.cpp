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
#include "View/Dialog/SettingsDialog.h"
#include "View/Dialog/MarkerDialog.h"
#include "View/Dialog/BarAddDialog.h"
#include "View/Dialog/BarEditDialog.h"
#include "View/Dialog/PauseAddDialog.h"
#include "View/Dialog/PauseEditDialog.h"
#include "View/BottomBar/BottomBar.h"
#include "View/BottomBar/MiniBar.h"
#include "View/SideBar/SideBar.h"
#include "View/SideBar/CaptureConsole.h"
#include "View/SideBar/LayerConsole.h"
#include "View/Mode/ViewModeDefault.h"
#include "View/Mode/ViewModeScaleBars.h"
#include "View/Helper/Slider.h"
#include "View/Helper/Progress.h"
#include "View/Helper/PeakMeter.h"
#include "View/AudioView.h"
#include "Plugins/PluginManager.h"
#include "Plugins/TsunamiPlugin.h"
#include "Plugins/Effect.h"
#include "Plugins/MidiEffect.h"
#include "Plugins/SongPlugin.h"
#include "Storage/Storage.h"
#include "Stuff/Log.h"
#include "Stuff/Clipboard.h"
#include "Device/OutputStream.h"
#include "Device/DeviceManager.h"
#include "Device/InputStreamAny.h"
#include "Audio/Renderer/SongRenderer.h"
#include "Data/Song.h"
#include "Data/SongSelection.h"

#include "Plugins/FastFourierTransform.h"

extern const string AppName;

hui::Timer debug_timer;

static Array<TsunamiWindow*> TsunamiWindows;

class TsunamiWindowObserver : public Observer
{
public:
	TsunamiWindow *win;
	TsunamiWindowObserver(TsunamiWindow *_win) : Observer("TsunamiWindow")
	{
		win = _win;
	}

	void onUpdate(Observable *o, const string &message)
	{
		win->onUpdate(o, message);
	}
};

TsunamiWindow::TsunamiWindow(Tsunami *_tsunami) :
	hui::Window(AppName, -1, -1, 800, 600, NULL, false, hui::WIN_MODE_RESIZABLE | hui::WIN_MODE_CONTROLS)
{
	app = _tsunami;

	app->win = this;

	int width = hui::Config.getInt("Window.Width", 800);
	int height = hui::Config.getInt("Window.Height", 600);
	bool maximized = hui::Config.getBool("Window.Maximized", true);

	//hui::AddKeyCode("insert_added", hui::KEY_RETURN);
	//hui::AddKeyCode("remove_added", hui::KEY_BACKSPACE);

	hui::AddCommand("new", "hui:new", hui::KEY_N + hui::KEY_CONTROL, std::bind(&TsunamiWindow::onNew, this));
	hui::AddCommand("open", "hui:open", hui::KEY_O + hui::KEY_CONTROL, std::bind(&TsunamiWindow::onOpen, this));
	hui::AddCommand("save", "hui:save", hui::KEY_S + hui::KEY_CONTROL, std::bind(&TsunamiWindow::onSave, this));
	hui::AddCommand("save_as", "hui:save-as", hui::KEY_S + hui::KEY_CONTROL + hui::KEY_SHIFT, std::bind(&TsunamiWindow::onSaveAs, this));
	hui::AddCommand("copy", "hui:copy", hui::KEY_C + hui::KEY_CONTROL, std::bind(&TsunamiWindow::onCopy, this));
	hui::AddCommand("paste", "hui:paste", hui::KEY_V + hui::KEY_CONTROL, std::bind(&TsunamiWindow::onPaste, this));
	hui::AddCommand("paste_as_samples", "hui:paste", hui::KEY_V + hui::KEY_CONTROL + hui::KEY_SHIFT, std::bind(&TsunamiWindow::onPasteAsSamples, this));
	hui::AddCommand("delete", "hui:delete", -1, std::bind(&TsunamiWindow::onDelete, this));
	hui::AddCommand("edit_multi", "", -1, std::bind(&TsunamiWindow::onEditMulti, this));
	hui::AddCommand("export_selection", "", hui::KEY_X + hui::KEY_CONTROL, std::bind(&TsunamiWindow::onExport, this));
	hui::AddCommand("undo", "hui:undo", hui::KEY_Z + hui::KEY_CONTROL, std::bind(&TsunamiWindow::onUndo, this));
	hui::AddCommand("redo", "hui:redo", hui::KEY_Y + hui::KEY_CONTROL, std::bind(&TsunamiWindow::onRedo, this));
	hui::AddCommand("add_track", "hui:add", -1, std::bind(&TsunamiWindow::onAddTrack, this));
	hui::AddCommand("add_time_track", "hui:add", -1, std::bind(&TsunamiWindow::onAddTimeTrack, this));
	hui::AddCommand("add_midi_track", "hui:add", -1, std::bind(&TsunamiWindow::onAddMidiTrack, this));
	hui::AddCommand("delete_track", "hui:delete", -1, std::bind(&TsunamiWindow::onDeleteTrack, this));
	hui::AddCommand("track_edit_midi", "hui:edit", -1, std::bind(&TsunamiWindow::onTrackEditMidi, this));
	hui::AddCommand("track_edit_fx", "hui:edit", -1, std::bind(&TsunamiWindow::onTrackEditFX, this));
	hui::AddCommand("track_add_marker", "hui:add", -1, std::bind(&TsunamiWindow::onTrackAddMarker, this));
	hui::AddCommand("layer_manager", "hui:settings", -1, std::bind(&TsunamiWindow::onLayerManager, this));
	hui::AddCommand("layer_add", "hui:add", -1, std::bind(&TsunamiWindow::onAddLayer, this));
	hui::AddCommand("layer_delete", "hui:delete", -1, std::bind(&TsunamiWindow::onDeleteLayer, this));
	hui::AddCommand("layer_up", "hui:up", -1, std::bind(&TsunamiWindow::onCurLayerUp, this));
	hui::AddCommand("layer_down", "hui:down", -1, std::bind(&TsunamiWindow::onCurLayerDown, this));
	hui::AddCommand("add_bars", "hui:add", -1, std::bind(&TsunamiWindow::onAddBars, this));
	hui::AddCommand("add_pause", "hui:add", -1, std::bind(&TsunamiWindow::onAddPause, this));
	hui::AddCommand("delete_bars", "hui:delete", -1, std::bind(&TsunamiWindow::onDeleteBars, this));
	hui::AddCommand("edit_bars", "hui:edit", -1, std::bind(&TsunamiWindow::onEditBars, this));
	hui::AddCommand("scale_bars", "hui:scale", -1, std::bind(&TsunamiWindow::onScaleBars, this));
	hui::AddCommand("bar_link_to_data", "", -1, std::bind(&TsunamiWindow::onBarsModifyMidi, this));
	hui::AddCommand("sample_manager", "", -1, std::bind(&TsunamiWindow::onSampleManager, this));
	hui::AddCommand("song_edit_samples", "", -1, std::bind(&TsunamiWindow::onSampleManager, this));
	hui::AddCommand("show_mixing_console", "", -1, std::bind(&TsunamiWindow::onMixingConsole, this));
	hui::AddCommand("show_fx_console", "", -1, std::bind(&TsunamiWindow::onFxConsole, this));
	hui::AddCommand("sample_from_selection", "hui:cut", -1, std::bind(&TsunamiWindow::onSampleFromSelection, this));
	hui::AddCommand("insert_sample", "", hui::KEY_I + hui::KEY_CONTROL, std::bind(&TsunamiWindow::onInsertSample, this));
	hui::AddCommand("remove_sample", "", -1, std::bind(&TsunamiWindow::onRemoveSample, this));
	hui::AddCommand("delete_marker", "", -1, std::bind(&TsunamiWindow::onDeleteMarker, this));
	hui::AddCommand("edit_marker", "", -1, std::bind(&TsunamiWindow::onEditMarker, this));
	hui::AddCommand("track_import", "", -1, std::bind(&TsunamiWindow::onTrackImport, this));
	hui::AddCommand("sub_import", "", -1, std::bind(&TsunamiWindow::onSampleImport, this));
	hui::AddCommand("song_properties", "", hui::KEY_F4, std::bind(&TsunamiWindow::onSongProperties, this));
	hui::AddCommand("track_properties", "", -1, std::bind(&TsunamiWindow::onTrackProperties, this));
	hui::AddCommand("sample_properties", "", -1, std::bind(&TsunamiWindow::onSampleProperties, this));
	hui::AddCommand("settings", "", -1, std::bind(&TsunamiWindow::onSettings, this));
	hui::AddCommand("play", "hui:media-play", -1, std::bind(&TsunamiWindow::onPlay, this));
	hui::AddCommand("play_loop", "", -1, std::bind(&TsunamiWindow::onPlayLoop, this));
	hui::AddCommand("pause", "hui:media-pause", -1, std::bind(&TsunamiWindow::onPause, this));
	hui::AddCommand("stop", "hui:media-stop", -1, std::bind(&TsunamiWindow::onStop, this));
	hui::AddCommand("record", "hui:media-record", -1, std::bind(&TsunamiWindow::onRecord, this));
	hui::AddCommand("show_log", "", -1, std::bind(&TsunamiWindow::onShowLog, this));
	hui::AddCommand("about", "", -1, std::bind(&TsunamiWindow::onAbout, this));
	hui::AddCommand("run_plugin", "hui:execute", hui::KEY_RETURN + hui::KEY_SHIFT, std::bind(&TsunamiWindow::onFindAndExecutePlugin, this));
	hui::AddCommand("exit", "hui:quit", hui::KEY_Q + hui::KEY_CONTROL, std::bind(&TsunamiWindow::onExit, this));

	hui::AddCommand("select_all", "", hui::KEY_A + hui::KEY_CONTROL, std::bind(&TsunamiWindow::onSelectAll, this));
	hui::AddCommand("select_nothing", "", -1, std::bind(&TsunamiWindow::onSelectNone, this));
	hui::AddCommand("select_expand", "", -1, std::bind(&TsunamiWindow::onSelectExpand, this));
	hui::AddCommand("view_midi_default", "", -1, std::bind(&TsunamiWindow::onViewMidiDefault, this));
	hui::AddCommand("view_midi_tab", "", -1, std::bind(&TsunamiWindow::onViewMidiTab, this));
	hui::AddCommand("view_midi_score", "", -1, std::bind(&TsunamiWindow::onViewMidiScore, this));
	hui::AddCommand("view_optimal", "", -1, std::bind(&TsunamiWindow::onViewOptimal, this));
	hui::AddCommand("zoom_in", "", -1, std::bind(&TsunamiWindow::onZoomIn, this));
	hui::AddCommand("zoom_out", "", -1, std::bind(&TsunamiWindow::onZoomOut, this));

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

	setMenu(hui::CreateResourceMenu("menu"));
	//ToolBarConfigure(true, true);
	setMaximized(maximized);


	app->plugin_manager->AddPluginsToMenu(this);

	// events
	event("hui:close", std::bind(&TsunamiWindow::onExit, this));
	for (int i=0;i<256;i++)
		event(format("jump_to_layer_%d", i), std::bind(&TsunamiWindow::onCurLayer, this));


	die_on_plugin_stop = false;
	auto_delete = false;

	song = new Song;


	view = new AudioView(this, song, app->device_manager);

	// side bar
	side_bar = new SideBar(view, song);
	embed(side_bar, "root_table", 1, 0);

	// bottom bar
	bottom_bar = new BottomBar(view, song, app->device_manager, app->log);
	embed(bottom_bar, "main_table", 0, 1);
	mini_bar = new MiniBar(bottom_bar, view->stream, app->device_manager, view);
	embed(mini_bar, "main_table", 0, 2);

	observer = new TsunamiWindowObserver(this);
	observer->subscribe(view);
	observer->subscribe(song);
	observer->subscribe(view->stream, OutputStream::MESSAGE_STATE_CHANGE);
	observer->subscribe(app->clipboard);
	observer->subscribe(bottom_bar);
	observer->subscribe(side_bar);



	song->newWithOneTrack(Track::TYPE_AUDIO, DEFAULT_SAMPLE_RATE);

	if (song->tracks.num > 0)
		view->setCurTrack(song->tracks[0]);
	view->optimizeView();
	hui::RunLater(0.5f, std::bind(&AudioView::optimizeView, view));

	updateMenu();
	TsunamiWindows.add(this);
}

TsunamiWindow::~TsunamiWindow()
{
}

void TsunamiCleanUp()
{
	bool again = false;
	do{
		again = false;
		foreachi(TsunamiWindow *w, TsunamiWindows, i)
			if (w->gotDestroyed() and w->auto_delete){
				delete(w);
				TsunamiWindows.erase(i);
				again = true;
				break;
			}
	}while(again);

	if (TsunamiWindows.num == 0)
		hui::End();
}

void TsunamiWindow::onDestroy()
{
	int w, h;
	getSizeDesired(w, h);
	hui::Config.setInt("Window.Width", w);
	hui::Config.setInt("Window.Height", h);
	hui::Config.setBool("Window.Maximized", isMaximized());

	observer->unsubscribe(view);
	observer->unsubscribe(song);
	observer->unsubscribe(view->stream);
	observer->unsubscribe(app->clipboard);
	observer->unsubscribe(bottom_bar);
	observer->unsubscribe(side_bar);
	delete(observer);

	delete(side_bar);
	delete(mini_bar);
	delete(bottom_bar);
	delete(view);
	delete(song);

	hui::RunLater(0.010f, &TsunamiCleanUp);
}


void TsunamiWindow::onAbout()
{
	hui::AboutBox(this);
}



void TsunamiWindow::onAddTrack()
{
	song->addTrack(Track::TYPE_AUDIO, view->cur_track->get_index() + 1);
}

void TsunamiWindow::onAddTimeTrack()
{
	song->action_manager->beginActionGroup();
	try{
		song->addTrack(Track::TYPE_TIME, 0);
		// some default data
		for (int i=0; i<10; i++)
			song->addBar(-1, 90, 4, 1, false);
	}catch(SongException &e){
		app->log->error(e.message);
	}
	song->action_manager->endActionGroup();
}

void TsunamiWindow::onAddMidiTrack()
{
	song->addTrack(Track::TYPE_MIDI, view->cur_track->get_index() + 1);
}

void TsunamiWindow::onDeleteTrack()
{
	if (view->cur_track){
		try{
			song->deleteTrack(get_track_index(view->cur_track));
		}catch(SongException &e){
			app->log->error(e.message);
		}
	}else{
		app->log->error(_("No track selected"));
	}
}

void TsunamiWindow::onTrackEditMidi()
{
	if (view->cur_track)
		side_bar->open(SideBar::MIDI_EDITOR_CONSOLE);
	else
		app->log->error(_("No track selected"));
}

void TsunamiWindow::onTrackEditFX()
{
	if (view->cur_track)
		side_bar->open(SideBar::FX_CONSOLE);
	else
		app->log->error(_("No track selected"));
}

void TsunamiWindow::onTrackAddMarker()
{
	if (view->cur_track){
		MarkerDialog *dlg = new MarkerDialog(this, view->cur_track, view->hover.pos, -1);
		dlg->run();
		delete(dlg);
	}else{
		app->log->error(_("No track selected"));
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
		app->log->error(_("No track selected"));
}

void TsunamiWindow::onSampleProperties()
{
	if (view->cur_sample)
		side_bar->open(SideBar::SAMPLEREF_CONSOLE);
	else
		app->log->error(_("No sample selected"));
}

void TsunamiWindow::onDeleteMarker()
{
	if (view->selection.type == Selection::TYPE_MARKER)
		view->cur_track->deleteMarker(view->selection.index);
	else
		app->log->error(_("No marker selected"));
}

void TsunamiWindow::onEditMarker()
{
	if (view->selection.type == Selection::TYPE_MARKER){
		MarkerDialog *dlg = new MarkerDialog(this, view->cur_track, -1, view->selection.index);
		dlg->run();
		delete(dlg);
	}else
		app->log->error(_("No marker selected"));
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
			side_bar->capture_console->onDelete();
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

void TsunamiWindow::onEditMulti()
{
	view->setEditMulti(isChecked(""));
}

void TsunamiWindow::onFindAndExecutePlugin()
{
	if (hui::FileDialogOpen(win, _("Select plugin script"), hui::AppDirectoryStatic + "Plugins/", _("Script (*.kaba)"), "*.kaba"))
		app->plugin_manager->_ExecutePlugin(this, hui::Filename);
}

void TsunamiWindow::onMenuExecuteEffect()
{
	string name = hui::GetEvent()->id.explode("--")[1];

	Effect *fx = CreateEffect(name, song);

	fx->resetConfig();
	if (fx->configure()){
		Range range = view->getPlaybackSelection();
		SongSelection sel = view->getEditSeletion();
		song->action_manager->beginActionGroup();
		for (Track *t : song->tracks)
			if (sel.has(t) and (t->type == t->TYPE_AUDIO)){
				fx->resetState();
				fx->doProcessTrack(t, view->cur_layer, range);
			}
		song->action_manager->endActionGroup();
	}
	delete(fx);
}

void TsunamiWindow::onMenuExecuteMidiEffect()
{
	string name = hui::GetEvent()->id.explode("--")[1];

	MidiEffect *fx = CreateMidiEffect(name, song);

	fx->resetConfig();
	if (fx->configure()){
		Range range = view->getPlaybackSelection();
		SongSelection sel = view->getEditSeletion();
		song->action_manager->beginActionGroup();
		for (Track *t : song->tracks)
			if (sel.has(t) and (t->type == t->TYPE_MIDI)){
				fx->resetState();
				fx->DoProcessTrack(t, range);
			}
		song->action_manager->endActionGroup();
	}
	delete(fx);
}

void TsunamiWindow::onMenuExecuteSongPlugin()
{
	string name = hui::GetEvent()->id.explode("--")[1];

	SongPlugin *p = CreateSongPlugin(name, this);

	p->apply(song);
	delete(p);
}

void TsunamiWindow::onMenuExecuteTsunamiPlugin()
{
	string name = hui::GetEvent()->id.explode("--")[1];

	for (TsunamiPlugin *p: plugins)
		if (p->name == name){
			if (p->active)
				p->stop();
			else
				p->start();
			return;
		}

	TsunamiPlugin *p = CreateTsunamiPlugin(name, this);

	plugins.add(p);
	observer->subscribe(p, p->MESSAGE_STOP_REQUEST);
	p->start();
}

void TsunamiWindow::onDelete()
{
	song->deleteSelection(view->getEditSeletion(), view->cur_layer, false);
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
	if (app->storage->askOpenImport(this)){
		Track *t = song->addTrack(Track::TYPE_AUDIO);
		app->storage->loadTrack(t, hui::Filename, view->sel.range.start(), view->cur_layer);
	}
}

void TsunamiWindow::onRemoveSample()
{
	song->deleteSelectedSamples(view->getEditSeletion());
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
	view->renderer->prepare(view->getPlaybackSelection(), true);
	view->stream->play();
}

void TsunamiWindow::onPause()
{
	if (side_bar->isActive(SideBar::CAPTURE_CONSOLE))
		return;
	view->stream->pause();
}

void TsunamiWindow::onStop()
{
	if (side_bar->isActive(SideBar::CAPTURE_CONSOLE))
		side_bar->_hide();
	else
		view->stream->stop();
}

void TsunamiWindow::onInsertSample()
{
	song->insertSelectedSamples(view->getEditSeletion(), view->cur_layer);
}

void TsunamiWindow::onRecord()
{
	side_bar->open(SideBar::CAPTURE_CONSOLE);
}

void TsunamiWindow::onAddLayer()
{
	side_bar->layer_console->onAdd();
}

void TsunamiWindow::onDeleteLayer()
{
	side_bar->layer_console->onDelete();
}

void TsunamiWindow::onCurLayer()
{
	view->setCurLayer(hui::GetEvent()->id.substr(14, -1)._int());
}

void TsunamiWindow::onCurLayerUp()
{
	view->setCurLayer(view->cur_layer + 1);
}

void TsunamiWindow::onCurLayerDown()
{
	view->setCurLayer(view->cur_layer - 1);
}

void TsunamiWindow::onLayerManager()
{
	side_bar->open(SideBar::LAYER_CONSOLE);
}

void TsunamiWindow::onSampleFromSelection()
{
	song->createSamplesFromSelection(view->getEditSeletion(), view->cur_layer);
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
	view->setMidiViewMode(view->MIDI_MODE_LINEAR);
}

void TsunamiWindow::onViewMidiTab()
{
	view->setMidiViewMode(view->MIDI_MODE_TAB);
}

void TsunamiWindow::onViewMidiScore()
{
	view->setMidiViewMode(view->MIDI_MODE_CLASSICAL);
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
	bool selected = !view->sel.range.empty();
// menu / toolbar
	// edit
	enable("undo", song->action_manager->undoable());
	enable("redo", song->action_manager->redoable());
	enable("copy", app->clipboard->canCopy(view));
	enable("paste", app->clipboard->hasData());
	enable("delete", selected or (view->sel.getNumSamples() > 0));
	// file
	//Enable("export_selection", true);
	//Enable("wave_properties", true);
	// track
	enable("delete_track", view->cur_track);
	enable("track_properties", view->cur_track);
	// layer
	enable("layer_delete", song->layer_names.num > 1);
	enable("layer_up", view->cur_layer < song->layer_names.num -1);
	enable("layer_down", view->cur_layer > 0);
	// bars
	enable("delete_bars", !view->sel.bars.empty());
	enable("edit_bars", !view->sel.bars.empty());
	enable("scale_bars", !view->sel.bars.empty());
	check("bar_link_to_data", view->bars_edit_data);
	// sample
	enable("sample_from_selection", selected);
	enable("insert_sample", view->sel.getNumSamples() > 0);
	enable("remove_sample", view->sel.getNumSamples() > 0);
	enable("sample_properties", view->cur_sample);
	// sound
	enable("play", !side_bar->isActive(SideBar::CAPTURE_CONSOLE));
	enable("stop", view->stream->isPlaying() or side_bar->isActive(SideBar::CAPTURE_CONSOLE));
	enable("pause", view->stream->isPlaying());
	check("play_loop", view->renderer->loop_if_allowed);
	enable("record", !side_bar->isActive(SideBar::CAPTURE_CONSOLE));
	// view
	check("show_mixing_console", bottom_bar->isActive(BottomBar::MIXING_CONSOLE));
	check("show_fx_console", side_bar->isActive(SideBar::FX_CONSOLE));
	check("sample_manager", side_bar->isActive(SideBar::SAMPLE_CONSOLE));

	hui::Menu *m = getMenu()->getSubMenuByID("menu_layer_target");
	if (m){
		m->clear();
		for (int i=0; i<song->layer_names.num; i++)
			m->addItemCheckable(song->getNiceLayerName(i), format("jump_to_layer_%d", i));
		check(format("jump_to_layer_%d", view->cur_layer), true);
	}

	string title = title_filename(song->filename) + " - " + AppName;
	if (!song->action_manager->isSave())
		title = "*" + title;
	setTitle(title);
}


void TsunamiWindow::onUpdate(Observable *o, const string &message)
{
	if (message == TsunamiPlugin::MESSAGE_STOP_REQUEST){
		TsunamiPlugin *tpl = (TsunamiPlugin*)o;
		tpl->stop();

		if (die_on_plugin_stop)
			hui::End();//hui::RunLater(0.01f, this, &TsunamiWindow::destroy);
	}else{
		// "Clipboard", "AudioFile" or "AudioView"
		updateMenu();
	}
}


void TsunamiWindow::onExit()
{
	if (allowTermination())
		destroy();
}


void TsunamiWindow::onNew()
{
	if (!allowTermination())
		return;
	NewDialog *dlg = new NewDialog(this, song);
	dlg->run();
	delete(dlg);
}


void TsunamiWindow::onOpen()
{
	if (!allowTermination())
		return;
	if (app->storage->askOpen(this))
		app->storage->load(song, hui::Filename);
}


void TsunamiWindow::onSave()
{
	if (song->filename == "")
		onSaveAs();
	else
		app->storage->save(song, song->filename);
}


void TsunamiWindow::onSaveAs()
{
	if (app->storage->askSave(this))
		app->storage->save(song, hui::Filename);
}

void TsunamiWindow::onExport()
{
	if (app->storage->askSaveExport(this)){
		SongRenderer rr(song, &view->sel);
		rr.prepare(view->getPlaybackSelection(), false);
		app->storage->saveViaRenderer(&rr, hui::Filename);
	}
}

void TsunamiWindow::onAddBars()
{
	auto dlg = new BarAddDialog(win, song, view);
	dlg->run();
	delete(dlg);
}

void TsunamiWindow::onAddPause()
{
	auto dlg = new PauseAddDialog(win, song, view);
	dlg->run();
	delete(dlg);
}

void TsunamiWindow::onDeleteBars()
{
	song->action_manager->beginActionGroup();

	for (int i=view->sel.bars.end()-1; i>=view->sel.bars.start(); i--){
		song->deleteBar(i, view->bars_edit_data);
	}
	song->action_manager->endActionGroup();
}

void TsunamiWindow::onEditBars()
{
	if (view->sel.bars.length == 0){
		return;
	}
	int num_bars = 0;
	int num_pauses = 0;
	for (int i=view->sel.bars.offset; i<view->sel.bars.end(); i++)
		if (song->bars[i].type == BarPattern::TYPE_BAR)
			num_bars ++;
		else if (song->bars[i].type == BarPattern::TYPE_PAUSE)
			num_pauses ++;
	if (num_bars > 0 and num_pauses == 0){
		hui::Dialog *dlg = new BarEditDialog(win, song, view->sel.bars, view->bars_edit_data);
		dlg->run();
		delete(dlg);
	}else if (num_bars == 0 and num_pauses == 1){
		hui::Dialog *dlg = new PauseEditDialog(win, song, view->sel.bars.start(), view->bars_edit_data);
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
	for (int i=view->sel.bars.start(); i<view->sel.bars.end(); i++)
		s.add(i);
	view->mode_scale_bars->startScaling(s);
}

void TsunamiWindow::onBarsModifyMidi()
{
	view->bars_edit_data = ! view->bars_edit_data;
	updateMenu();
}
