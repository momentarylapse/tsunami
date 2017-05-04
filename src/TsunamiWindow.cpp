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

extern string AppName;

HuiTimer debug_timer;

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

TsunamiWindow::TsunamiWindow() :
	HuiWindow(AppName, -1, -1, 800, 600, NULL, false, HUI_WIN_MODE_RESIZABLE | HUI_WIN_MODE_CONTROLS)
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
	HuiAddCommandM("paste_as_samples", "hui:paste", KEY_V + KEY_CONTROL + KEY_SHIFT, this, &TsunamiWindow::onPasteAsSamples);
	HuiAddCommandM("delete", "hui:delete", -1, this, &TsunamiWindow::onDelete);
	HuiAddCommandM("edit_multi", "", -1, this, &TsunamiWindow::onEditMulti);
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
	HuiAddCommandM("layer_manager", "hui:settings", -1, this, &TsunamiWindow::onLayerManager);
	HuiAddCommandM("layer_add", "hui:add", -1, this, &TsunamiWindow::onAddLayer);
	HuiAddCommandM("layer_delete", "hui:delete", -1, this, &TsunamiWindow::onDeleteLayer);
	HuiAddCommandM("layer_up", "hui:up", -1, this, &TsunamiWindow::onCurLayerUp);
	HuiAddCommandM("layer_down", "hui:down", -1, this, &TsunamiWindow::onCurLayerDown);
	HuiAddCommandM("add_bars", "hui:add", -1, this, &TsunamiWindow::onAddBars);
	HuiAddCommandM("add_pause", "hui:add", -1, this, &TsunamiWindow::onAddPause);
	HuiAddCommandM("delete_bars", "hui:delete", -1, this, &TsunamiWindow::onDeleteBars);
	HuiAddCommandM("edit_bars", "hui:edit", -1, this, &TsunamiWindow::onEditBars);
	HuiAddCommandM("scale_bars", "hui:scale", -1, this, &TsunamiWindow::onScaleBars);
	HuiAddCommandM("bar_link_to_data", "", -1, this, &TsunamiWindow::onBarsModifyMidi);
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
	HuiAddCommandM("select_expand", "", -1, this, &TsunamiWindow::onSelectExpand);
	HuiAddCommandM("view_midi_default", "", -1, this, &TsunamiWindow::onViewMidiDefault);
	HuiAddCommandM("view_midi_tab", "", -1, this, &TsunamiWindow::onViewMidiTab);
	HuiAddCommandM("view_midi_score", "", -1, this, &TsunamiWindow::onViewMidiScore);
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


	tsunami->plugin_manager->AddPluginsToMenu(this);

	// events
	event("hui:close", this, &TsunamiWindow::onExit);
	for (int i=0;i<256;i++)
		event(format("jump_to_layer_%d", i), this, &TsunamiWindow::onCurLayer);


	die_on_plugin_stop = false;
	auto_delete = false;

	song = new Song;


	view = new AudioView(this, song, tsunami->device_manager);

	// side bar
	side_bar = new SideBar(view, song);
	embed(side_bar, "root_table", 1, 0);

	// bottom bar
	bottom_bar = new BottomBar(view, song, tsunami->device_manager, tsunami->log);
	embed(bottom_bar, "main_table", 0, 1);
	mini_bar = new MiniBar(bottom_bar, view->stream, tsunami->device_manager, view);
	embed(mini_bar, "main_table", 0, 2);

	observer = new TsunamiWindowObserver(this);
	observer->subscribe(view);
	observer->subscribe(song);
	observer->subscribe(view->stream, OutputStream::MESSAGE_STATE_CHANGE);
	observer->subscribe(tsunami->clipboard);
	observer->subscribe(bottom_bar);
	observer->subscribe(side_bar);



	song->newWithOneTrack(Track::TYPE_AUDIO, DEFAULT_SAMPLE_RATE);

	if (song->tracks.num > 0)
		view->setCurTrack(song->tracks[0]);
	view->optimizeView();
	HuiRunLaterM(0.5f, view, &AudioView::optimizeView);

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
		HuiEnd();
}

void TsunamiWindow::onDestroy()
{
	int w, h;
	getSizeDesired(w, h);
	HuiConfig.setInt("Window.Width", w);
	HuiConfig.setInt("Window.Height", h);
	HuiConfig.setBool("Window.Maximized", isMaximized());

	observer->unsubscribe(view);
	observer->unsubscribe(song);
	observer->unsubscribe(view->stream);
	observer->unsubscribe(tsunami->clipboard);
	observer->unsubscribe(bottom_bar);
	observer->unsubscribe(side_bar);
	delete(observer);

	delete(side_bar);
	delete(mini_bar);
	delete(bottom_bar);
	delete(view);
	delete(song);

	HuiRunLater(0.010f, &TsunamiCleanUp);
}


void TsunamiWindow::onAbout()
{
	HuiAboutBox(this);
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
		tsunami->log->error(e.message);
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
			tsunami->log->error(e.message);
		}
	}else
		tsunami->log->error(_("No track selected"));
}

void TsunamiWindow::onTrackEditMidi()
{
	if (view->cur_track)
		side_bar->open(SideBar::MIDI_EDITOR_CONSOLE);
	else
		tsunami->log->error(_("No track selected"));
}

void TsunamiWindow::onTrackEditFX()
{
	if (view->cur_track)
		side_bar->open(SideBar::FX_CONSOLE);
	else
		tsunami->log->error(_("No track selected"));
}

void TsunamiWindow::onTrackAddMarker()
{
	if (view->cur_track){
		MarkerDialog *dlg = new MarkerDialog(this, view->cur_track, view->hover.pos, -1);
		dlg->run();
		delete(dlg);
	}else{
		tsunami->log->error(_("No track selected"));
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
		tsunami->log->error(_("No track selected"));
}

void TsunamiWindow::onSampleProperties()
{
	if (view->cur_sample)
		side_bar->open(SideBar::SAMPLEREF_CONSOLE);
	else
		tsunami->log->error(_("No sample selected"));
}

void TsunamiWindow::onDeleteMarker()
{
	if (view->selection.type == Selection::TYPE_MARKER)
		view->cur_track->deleteMarker(view->selection.index);
	else
		tsunami->log->error(_("No marker selected"));
}

void TsunamiWindow::onEditMarker()
{
	if (view->selection.type == Selection::TYPE_MARKER){
		MarkerDialog *dlg = new MarkerDialog(this, view->cur_track, -1, view->selection.index);
		dlg->run();
		delete(dlg);
	}else
		tsunami->log->error(_("No marker selected"));
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
			string answer = HuiQuestionBox(this, _("Question"), _("Cancel recording?"), true);
			if (answer != "hui:yes")
				return false;
			side_bar->capture_console->onDelete();
			side_bar->_hide();
		}
	}

	if (song->action_manager->isSave())
		return true;
	string answer = HuiQuestionBox(this, _("Question"), format(_("'%s'\nSave file?"), title_filename(song->filename).c_str()), true);
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
	tsunami->clipboard->copy(view);
}

void TsunamiWindow::onPaste()
{
	tsunami->clipboard->paste(view);
}

void TsunamiWindow::onPasteAsSamples()
{
	tsunami->clipboard->pasteAsSamples(view);
}

void TsunamiWindow::onEditMulti()
{
	view->setEditMulti(isChecked(""));
}

void TsunamiWindow::onFindAndExecutePlugin()
{
	if (HuiFileDialogOpen(win, _("Select plugin script"), HuiAppDirectoryStatic + "Plugins/", _("Script (*.kaba)"), "*.kaba"))
		tsunami->plugin_manager->_ExecutePlugin(this, HuiFilename);
}

void TsunamiWindow::onMenuExecuteEffect()
{
	string name = HuiGetEvent()->id.explode("--")[1];

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
	string name = HuiGetEvent()->id.explode("--")[1];

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
	string name = HuiGetEvent()->id.explode("--")[1];

	SongPlugin *p = CreateSongPlugin(name, this);

	p->apply(song);
	delete(p);
}

void TsunamiWindow::onMenuExecuteTsunamiPlugin()
{
	string name = HuiGetEvent()->id.explode("--")[1];

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
	if (tsunami->storage->askOpenImport(this)){
		Track *t = song->addTrack(Track::TYPE_AUDIO);
		tsunami->storage->loadTrack(t, HuiFilename, view->sel.range.start(), view->cur_layer);
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
	view->setCurLayer(HuiGetEvent()->id.substr(14, -1)._int());
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
	enable("copy", tsunami->clipboard->canCopy(view));
	enable("paste", tsunami->clipboard->hasData());
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

	HuiMenu *m = getMenu()->getSubMenuByID("menu_layer_target");
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
			HuiEnd();//HuiRunLaterM(0.01f, this, &TsunamiWindow::destroy);
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
	if (tsunami->storage->askSaveExport(this)){
		SongRenderer rr(song, &view->sel);
		rr.prepare(view->getPlaybackSelection(), false);
		tsunami->storage->saveViaRenderer(&rr, HuiFilename);
	}
}

void TsunamiWindow::onAddBars()
{
	HuiDialog *dlg = new BarAddDialog(win, song, view);
	dlg->run();
	delete(dlg);
}

void TsunamiWindow::onAddPause()
{
	HuiDialog *dlg = new PauseAddDialog(win, song, view);
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
		HuiDialog *dlg = new BarEditDialog(win, song, view->sel.bars, view->bars_edit_data);
		dlg->run();
		delete(dlg);
	}else if (num_bars == 0 and num_pauses == 1){
		HuiDialog *dlg = new PauseEditDialog(win, song, view->sel.bars.start(), view->bars_edit_data);
		dlg->run();
		delete(dlg);
	}else{
		HuiErrorBox(this, _("Error"), _("Can only edit bars or a single pause at a time."));
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
