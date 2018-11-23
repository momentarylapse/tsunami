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
#include "View/Dialog/BarDeleteDialog.h"
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
#include "View/AudioViewTrack.h"
#include "View/AudioViewLayer.h"
#include "Plugins/PluginManager.h"
#include "Plugins/TsunamiPlugin.h"
#include "Plugins/SongPlugin.h"
#include "Storage/Storage.h"
#include "Stuff/Log.h"
#include "Stuff/Clipboard.h"
#include "Stuff/BackupManager.h"
#include "Device/DeviceManager.h"
#include "Data/base.h"
#include "Data/Track.h"
#include "Data/TrackLayer.h"
#include "Data/Song.h"
#include "Data/SongSelection.h"
#include "Data/Rhythm/Bar.h"
#include "Action/ActionManager.h"
#include "Action/Track/Buffer/ActionTrackEditBuffer.h"
#include "Module/Audio/AudioEffect.h"
#include "Module/Audio/AudioSource.h"
#include "Module/Midi/MidiEffect.h"
#include "Module/Midi/MidiSource.h"
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
	session->set_win(this);
	song = session->song;
	app = tsunami;

	int width = hui::Config.get_int("Window.Width", 800);
	int height = hui::Config.get_int("Window.Height", 600);
	bool maximized = hui::Config.get_bool("Window.Maximized", true);

	event("new", std::bind(&TsunamiWindow::on_new, this));
	set_key_code("new", hui::KEY_N + hui::KEY_CONTROL, "hui:new");
	event("open", std::bind(&TsunamiWindow::on_open, this));
	set_key_code("open", hui::KEY_O + hui::KEY_CONTROL, "hui:open");
	event("save", std::bind(&TsunamiWindow::on_save, this));
	set_key_code("save", hui::KEY_S + hui::KEY_CONTROL, "hui:save");
	event("save_as", std::bind(&TsunamiWindow::on_save_as, this));
	set_key_code("save_as", hui::KEY_S + hui::KEY_CONTROL + hui::KEY_SHIFT, "hui:save-as");
	event("copy", std::bind(&TsunamiWindow::on_copy, this));
	set_key_code("copy", hui::KEY_C + hui::KEY_CONTROL, "hui:copy");
	event("paste", std::bind(&TsunamiWindow::on_paste, this));
	set_key_code("paste", hui::KEY_V + hui::KEY_CONTROL, "hui:paste");
	event("paste_as_samples", std::bind(&TsunamiWindow::on_paste_as_samples, this));
	set_key_code("paste_as_samples", hui::KEY_V + hui::KEY_CONTROL + hui::KEY_SHIFT, "hui:paste");
	event("paste_time", std::bind(&TsunamiWindow::on_paste_time, this));
	event("delete", std::bind(&TsunamiWindow::on_delete, this));
	set_key_code("delete", hui::KEY_DELETE, "hui:delete");
	event("export_selection", std::bind(&TsunamiWindow::on_export, this));
	set_key_code("export_selection", hui::KEY_X + hui::KEY_CONTROL, "");
	event("quick_export", std::bind(&TsunamiWindow::on_quick_export, this));
	set_key_code("quick_export", hui::KEY_X + hui::KEY_CONTROL + hui::KEY_SHIFT, "");
	event("undo", std::bind(&TsunamiWindow::on_undo, this));
	set_key_code("undo", hui::KEY_Z + hui::KEY_CONTROL, "hui:undo");
	event("redo", std::bind(&TsunamiWindow::on_redo, this));
	set_key_code("redo", hui::KEY_Y + hui::KEY_CONTROL, "hui:redo");
	event("track_render", std::bind(&TsunamiWindow::on_track_render, this));
	event("add_audio_track_mono", std::bind(&TsunamiWindow::on_add_audio_track_mono, this));
	set_key_code("add_audio_track_mono", -1, "hui:add");
	event("add_audio_track_stereo", std::bind(&TsunamiWindow::on_add_audio_track_stereo, this));
	set_key_code("add_audio_track_stereo", -1, "hui:add");
	event("add_time_track", std::bind(&TsunamiWindow::on_add_time_track, this));
	set_key_code("add_time_track", -1, "hui:add");
	event("add_midi_track", std::bind(&TsunamiWindow::on_add_midi_track, this));
	set_key_code("add_midi_track", -1, "hui:add");
	event("delete_track", std::bind(&TsunamiWindow::on_delete_track, this));
	set_key_code("delete_track", -1, "hui:delete");
	event("track_edit_midi", std::bind(&TsunamiWindow::on_track_edit_midi, this));
	set_key_code("track_edit_midi", -1, "hui:edit");
	event("track_edit_fx", std::bind(&TsunamiWindow::on_track_edit_fx, this));
	set_key_code("track_edit_fx", -1, "hui:edit");
	event("track_add_marker", std::bind(&TsunamiWindow::on_track_add_marker, this));
	set_key_code("track_add_marker", -1, "hui:add");
	event("track_convert_mono", std::bind(&TsunamiWindow::on_track_convert_mono, this));
	event("track_convert_stereo", std::bind(&TsunamiWindow::on_track_convert_stereo, this));
	event("delete_buffer", std::bind(&TsunamiWindow::on_buffer_delete, this));
	event("make_buffer_movable", std::bind(&TsunamiWindow::on_buffer_make_movable, this));


	event("layer_midi_mode_linear", std::bind(&TsunamiWindow::on_layer_midi_mode_linear, this));
	event("layer_midi_mode_tab", std::bind(&TsunamiWindow::on_layer_midi_mode_tab, this));
	event("layer_midi_mode_classical", std::bind(&TsunamiWindow::on_layer_midi_mode_classical, this));

	event("layer_add", std::bind(&TsunamiWindow::on_add_layer, this));
	set_key_code("layer_add", -1, "hui:add");
	event("delete_layer", std::bind(&TsunamiWindow::on_delete_layer, this));
	set_key_code("delete_layer", -1, "hui:delete");
	event("layer_make_track", std::bind(&TsunamiWindow::on_layer_make_track, this));
	event("layer_merge", std::bind(&TsunamiWindow::on_layer_merge, this));
	event("layer_mark_dominant", std::bind(&TsunamiWindow::on_layer_mark_selection_dominant, this));
	event("add_bars", std::bind(&TsunamiWindow::on_add_bars, this));
	set_key_code("add_bars", -1, "hui:add");
	event("add_pause", std::bind(&TsunamiWindow::on_add_pause, this));
	set_key_code("add_pause", -1, "hui:add");
	event("delete_bars", std::bind(&TsunamiWindow::on_delete_bars, this));
	set_key_code("delete_bars", -1, "hui:delete");
	event("delete_time", std::bind(&TsunamiWindow::on_delete_time_interval, this));
	set_key_code("delete_time", -1, "hui:delete");
	event("insert_time", std::bind(&TsunamiWindow::on_insert_time_interval, this));
	set_key_code("insert_time", -1, "hui:add");
	event("edit_bars", std::bind(&TsunamiWindow::on_edit_bars, this));
	set_key_code("edit_bars", -1, "hui:edit");
	event("scale_bars", std::bind(&TsunamiWindow::on_scale_bars, this));
	set_key_code("scale_bars", -1, "hui:scale");
	event("sample_manager", std::bind(&TsunamiWindow::on_sample_manager, this));
	event("song_edit_samples", std::bind(&TsunamiWindow::on_sample_manager, this));
	event("show_mixing_console", std::bind(&TsunamiWindow::on_mixing_console, this));
	event("show_fx_console", std::bind(&TsunamiWindow::on_fx_console, this));
	event("sample_from_selection", std::bind(&TsunamiWindow::on_sample_from_selection, this));
	set_key_code("sample_from_selection", -1, "hui:cut");
	event("insert_sample", std::bind(&TsunamiWindow::on_insert_sample, this));
	set_key_code("insert_sample", hui::KEY_I + hui::KEY_CONTROL, "");
	event("remove_sample", std::bind(&TsunamiWindow::on_remove_sample, this));
	event("delete_marker", std::bind(&TsunamiWindow::on_delete_marker, this));
	event("edit_marker", std::bind(&TsunamiWindow::on_edit_marker, this));
	event("track_import", std::bind(&TsunamiWindow::on_track_import, this));
	event("sub_import", std::bind(&TsunamiWindow::on_sample_import, this));
	event("song_properties", std::bind(&TsunamiWindow::on_song_properties, this));
	set_key_code("song_properties", hui::KEY_F4, "");
	event("track_properties", std::bind(&TsunamiWindow::on_track_properties, this));
	event("sample_properties", std::bind(&TsunamiWindow::on_sample_properties, this));
	event("settings", std::bind(&TsunamiWindow::on_settings, this));
	event("play", std::bind(&TsunamiWindow::on_play, this));
	set_key_code("play", -1, "hui:media-play");
	event("play_loop", std::bind(&TsunamiWindow::on_play_loop, this));
	event("pause", std::bind(&TsunamiWindow::on_pause, this));
	set_key_code("pause", -1, "hui:media-pause");
	event("stop", std::bind(&TsunamiWindow::on_stop, this));
	set_key_code("stop", -1, "hui:media-stop");
	event("record", std::bind(&TsunamiWindow::on_record, this));
	set_key_code("record", -1, "hui:media-record");
	event("show_log", std::bind(&TsunamiWindow::on_show_log, this));
	event("about", std::bind(&TsunamiWindow::on_about, this));
	set_key_code("run_plugin", hui::KEY_RETURN + hui::KEY_SHIFT, "hui:execute");
	event("exit", std::bind(&TsunamiWindow::on_exit, this));
	set_key_code("exit", hui::KEY_Q + hui::KEY_CONTROL, "hui:quit");
	event("select_all", std::bind(&TsunamiWindow::on_select_all, this));
	set_key_code("select_all", hui::KEY_A + hui::KEY_CONTROL, "");
	event("select_nothing", std::bind(&TsunamiWindow::on_select_none, this));
	event("select_expand", std::bind(&TsunamiWindow::on_select_expand, this));
	set_key_code("select_expand", hui::KEY_TAB + hui::KEY_SHIFT, "");
	event("view_midi_default", std::bind(&TsunamiWindow::on_view_midi_default, this));
	event("view_midi_tab", std::bind(&TsunamiWindow::on_view_midi_tab, this));
	event("view_midi_score", std::bind(&TsunamiWindow::on_view_midi_score, this));
	event("view_optimal", std::bind(&TsunamiWindow::on_view_optimal, this));
	event("zoom_in", std::bind(&TsunamiWindow::on_zoom_in, this));
	event("zoom_out", std::bind(&TsunamiWindow::on_zoom_out, this));

	// table structure
	set_size(width, height);
	set_border_width(0);
	add_grid("", 0, 0, "root_table");
	set_target("root_table");
	add_grid("", 0, 0, "main_table");

	// main table
	set_target("main_table");
	add_drawing_area("!grabfocus", 0, 0, "area");


	toolbar[0]->set_by_id("toolbar");
	//ToolbarConfigure(false, true);

	set_menu(hui::CreateResourceMenu("menu"));
	//ToolBarConfigure(true, true);
	set_maximized(maximized);


	app->plugin_manager->add_plugins_to_menu(this);

	// events
	event("hui:close", std::bind(&TsunamiWindow::on_exit, this));

	for (int i=0; i<256; i++){
		event("import-backup-"+i2s(i), std::bind(&TsunamiWindow::on_import_backup, this));
		event("delete-backup-"+i2s(i), std::bind(&TsunamiWindow::on_delete_backup, this));
	}

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

	view->subscribe(this, std::bind(&TsunamiWindow::on_update, this));
	song->action_manager->subscribe(this, std::bind(&TsunamiWindow::on_update, this));
	app->clipboard->subscribe(this, std::bind(&TsunamiWindow::on_update, this));
	bottom_bar->subscribe(this, std::bind(&TsunamiWindow::on_bottom_bar_update, this));
	side_bar->subscribe(this, std::bind(&TsunamiWindow::on_side_bar_update, this));


	update_menu();
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
			if (s->win->got_destroyed() and s->win->auto_delete){
				delete s->win;
				delete s;
				tsunami->sessions.erase(i);
				again = true;
				break;
			}
	}while(again);

	if (tsunami->sessions.num == 0)
		tsunami->end();
}

void TsunamiWindow::on_destroy()
{
	int w, h;
	get_size_desired(w, h);
	hui::Config.set_int("Window.Width", w);
	hui::Config.set_int("Window.Height", h);
	hui::Config.set_bool("Window.Maximized", is_maximized());

	view->unsubscribe(this);
	song->action_manager->unsubscribe(this);
	app->clipboard->unsubscribe(this);
	bottom_bar->unsubscribe(this);
	side_bar->unsubscribe(this);

	delete side_bar;
	delete mini_bar;
	delete bottom_bar;
	delete view;

	hui::RunLater(0.010f, &TsunamiCleanUp);
}


void TsunamiWindow::on_about()
{
	hui::AboutBox(this);
}



void TsunamiWindow::on_add_audio_track_mono()
{
	song->add_track(SignalType::AUDIO_MONO);
}

void TsunamiWindow::on_add_audio_track_stereo()
{
	song->add_track(SignalType::AUDIO_STEREO);
}

void TsunamiWindow::on_add_time_track()
{
	song->begin_action_group();
	try{
		song->add_track(SignalType::BEATS, 0);
		// some default data
		for (int i=0; i<10; i++)
			song->add_bar(-1, 90, 4, 1, false);
	}catch(Song::Exception &e){
		session->e(e.message);
	}
	song->end_action_group();
}

void TsunamiWindow::on_import_backup()
{
	string id = hui::GetEvent()->id;
	int uuid = id.explode(":").back()._int();
	string filename = BackupManager::get_filename_for_uuid(uuid);
	if (filename == "")
		return;


	if (song->is_empty()){
		session->storage_options = "f32:2:44100";
		session->storage->load(song, filename);
		//BackupManager::set_save_state(session);
		session->storage_options = "";
	}else{
		Session *s = tsunami->create_session();
		s->storage_options = "f32:2:44100";
		s->win->show();
		s->storage->load(s->song, filename);
		s->storage_options = "";
	}

	//BackupManager::del
}

void TsunamiWindow::on_delete_backup()
{
	string id = hui::GetEvent()->id;
	int uuid = id.explode(":").back()._int();
	BackupManager::delete_old(uuid);
}

void TsunamiWindow::on_add_midi_track()
{
	song->add_track(SignalType::MIDI);
}

void TsunamiWindow::on_track_render()
{
	Range range = view->sel.range;
	if (range.empty()){
		session->e(_("Selection range is empty"));
		return;
	}

	auto *p = new ProgressCancelable(_(""), this);
	song->begin_action_group();
	Track *t = song->add_track(SignalType::AUDIO);

	SongRenderer renderer(song);
	renderer.prepare(range, false);
	renderer.allow_tracks(view->get_selected_tracks());
	renderer.allow_layers(view->get_playable_layers());

	int chunk_size = 1<<12;
	int offset = range.offset;

	while (offset < range.end()){
		p->set((float)(offset - range.offset) / range.length);

		AudioBuffer buf;
		Range r = Range(offset, min(chunk_size, range.end() - offset));
		t->layers[0]->get_buffers(buf, r);

		ActionTrackEditBuffer *a = new ActionTrackEditBuffer(t->layers[0], r);
		renderer.read(buf);
		song->execute(a);

		offset += chunk_size;
		if (p->is_cancelled())
			break;
	}
	song->end_action_group();
	delete p;

}

void TsunamiWindow::on_delete_track()
{
	if (view->cur_track()){
		try{
			song->delete_track(view->cur_track());
		}catch(Song::Exception &e){
			session->e(e.message);
		}
	}else{
		session->e(_("No track selected"));
	}
}

void TsunamiWindow::on_track_edit_midi()
{
	if (view->cur_track())
		side_bar->open(SideBar::MIDI_EDITOR_CONSOLE);
	else
		session->e(_("No track selected"));
}

void TsunamiWindow::on_track_edit_fx()
{
	if (view->cur_track())
		side_bar->open(SideBar::FX_CONSOLE);
	else
		session->e(_("No track selected"));
}

void TsunamiWindow::on_track_add_marker()
{
	if (view->hover_before_leave.track){
		Range range = view->sel.range;
		if (!range.is_inside(view->hover_before_leave.pos))
			range = Range(view->hover_before_leave.pos, 0);
		MarkerDialog *dlg = new MarkerDialog(this, view->hover_before_leave.track, range, nullptr);
		dlg->run();
		delete dlg;
	}else{
		session->e(_("No track selected"));
	}
}

void TsunamiWindow::on_track_convert_mono()
{
	if (view->cur_track())
		view->cur_track()->set_channels(1);
	else
		session->e(_("No track selected"));
}

void TsunamiWindow::on_track_convert_stereo()
{
	if (view->cur_track())
		view->cur_track()->set_channels(2);
	else
		session->e(_("No track selected"));
}
void TsunamiWindow::on_buffer_delete()
{
	foreachi (AudioBuffer &buf, view->cur_layer()->buffers, i)
		if (buf.range().is_inside(view->hover_before_leave.pos)){
			SongSelection s = SongSelection::from_range(song, buf.range(), {}, view->cur_layer()).filter(0);
			song->delete_selection(s);
		}
}

void TsunamiWindow::on_buffer_make_movable()
{
	for (AudioBuffer &buf: view->cur_layer()->buffers){
		if (buf.range().is_inside(view->hover_before_leave.pos)){
			SongSelection s = SongSelection::from_range(song, buf.range(), {}, view->cur_layer()).filter(0);
			song->create_samples_from_selection(s, true);
		}
	}
}

void TsunamiWindow::on_layer_midi_mode_linear()
{
	view->cur_vlayer->set_midi_mode(MidiMode::LINEAR);
}

void TsunamiWindow::on_layer_midi_mode_tab()
{
	view->cur_vlayer->set_midi_mode(MidiMode::TAB);
}

void TsunamiWindow::on_layer_midi_mode_classical()
{
	view->cur_vlayer->set_midi_mode(MidiMode::CLASSICAL);
}

void TsunamiWindow::on_song_properties()
{
	side_bar->open(SideBar::SONG_CONSOLE);
}

void TsunamiWindow::on_track_properties()
{
	if (view->cur_track())
		side_bar->open(SideBar::TRACK_CONSOLE);
	else
		session->e(_("No track selected"));
}

void TsunamiWindow::on_sample_properties()
{
	if (view->cur_sample)
		side_bar->open(SideBar::SAMPLEREF_CONSOLE);
	else
		session->e(_("No sample selected"));
}

void TsunamiWindow::on_delete_marker()
{
	if (view->hover_before_leave.type == Selection::Type::MARKER)
		view->cur_track()->delete_marker(view->hover_before_leave.marker);
	else
		session->e(_("No marker selected"));
}

void TsunamiWindow::on_edit_marker()
{
	if (view->hover_before_leave.type == Selection::Type::MARKER){
		MarkerDialog *dlg = new MarkerDialog(this, view->cur_track(), Range::EMPTY, view->hover_before_leave.marker);
		dlg->run();
		delete dlg;
	}else
		session->e(_("No marker selected"));
}

void TsunamiWindow::on_show_log()
{
	bottom_bar->open(BottomBar::LOG_CONSOLE);
}

void TsunamiWindow::on_undo()
{
	song->undo();
}

void TsunamiWindow::on_redo()
{
	song->redo();
}

void TsunamiWindow::on_send_bug_report()
{
}


string title_filename(const string &filename)
{
	if (filename.num > 0)
		return filename.basename();// + " (" + filename.dirname() + ")";
	return _("No name");
}

bool TsunamiWindow::allow_termination()
{
	if (side_bar->is_active(SideBar::CAPTURE_CONSOLE)){
		if (side_bar->capture_console->is_capturing()){
			string answer = hui::QuestionBox(this, _("Question"), _("Cancel recording?"), true);
			if (answer != "hui:yes")
				return false;
			side_bar->capture_console->on_dump();
			side_bar->_hide();
		}
	}

	if (song->action_manager->is_save())
		return true;
	string answer = hui::QuestionBox(this, _("Question"), format(_("'%s'\nSave file?"), title_filename(song->filename).c_str()), true);
	if (answer == "hui:yes"){
		/*if (!OnSave())
			return false;*/
		on_save();
		return true;
	}else if (answer == "hui:no")
		return true;

	// cancel
	return false;
}

void TsunamiWindow::on_copy()
{
	app->clipboard->copy(view);
}

void TsunamiWindow::on_paste()
{
	app->clipboard->paste(view);
}

void TsunamiWindow::on_paste_as_samples()
{
	app->clipboard->paste_as_samples(view);
}

void TsunamiWindow::on_paste_time()
{
	app->clipboard->paste_with_time(view);
}

void TsunamiWindow::on_menu_execute_audio_effect()
{
	string name = hui::GetEvent()->id.explode("--")[1];

	AudioEffect *fx = CreateAudioEffect(session, name);

	fx->reset_config();
	if (fx->configure(this)){
		song->begin_action_group();
		for (Track *t: song->tracks)
			for (TrackLayer *l: t->layers)
				if (view->sel.has(l) and (t->type == SignalType::AUDIO)){
					fx->reset_state();
					fx->do_process_track(l, view->sel.range);
				}
		song->end_action_group();
	}
	delete fx;
}

void TsunamiWindow::on_menu_execute_audio_source()
{
	string name = hui::GetEvent()->id.explode("--")[1];

	AudioSource *s = CreateAudioSource(session, name);

	s->reset_config();
	if (s->configure(this)){
		song->begin_action_group();
		for (Track *t: song->tracks)
			for (TrackLayer *l: t->layers)
				if (view->sel.has(l) and (t->type == SignalType::AUDIO)){
					s->reset_state();
					AudioBuffer buf;
					l->get_buffers(buf, view->sel.range);
					s->read(buf);
				}
		song->end_action_group();
	}
	delete s;
}

void TsunamiWindow::on_menu_execute_midi_effect()
{
	string name = hui::GetEvent()->id.explode("--")[1];

	MidiEffect *fx = CreateMidiEffect(session, name);

	fx->reset_config();
	if (fx->configure(this)){
		song->action_manager->group_begin();
		for (Track *t : song->tracks)
			for (TrackLayer *l : t->layers)
			if (view->sel.has(l) and (t->type == SignalType::MIDI)){
				fx->reset_state();
				fx->process_layer(l, view->sel);
			}
		song->action_manager->group_end();
	}
	delete fx;
}

void TsunamiWindow::on_menu_execute_midi_source()
{
	string name = hui::GetEvent()->id.explode("--")[1];

	MidiSource *s = CreateMidiSource(session, name);

	s->reset_config();
	if (s->configure(this)){
		song->begin_action_group();
		for (Track *t : song->tracks)
			for (TrackLayer *l : t->layers)
			if (view->sel.has(l) and (t->type == SignalType::MIDI)){
				s->reset_state();
				MidiEventBuffer buf;
				buf.samples = view->sel.range.length;
				s->read(buf);
				l->insert_midi_data(view->sel.range.offset, midi_events_to_notes(buf));
			}
		song->end_action_group();
	}
	delete s;
}

void TsunamiWindow::on_menu_execute_song_plugin()
{
	string name = hui::GetEvent()->id.explode("--")[1];

	SongPlugin *p = CreateSongPlugin(session, name);

	p->apply();
	delete p;
}

void TsunamiWindow::on_menu_execute_tsunami_plugin()
{
	string name = hui::GetEvent()->id.explode("--")[1];

	session->execute_tsunami_plugin(name);
}

void TsunamiWindow::on_delete()
{
	if (!view->sel.is_empty())
		song->delete_selection(view->sel);
}

void TsunamiWindow::on_sample_manager()
{
	side_bar->open(SideBar::SAMPLE_CONSOLE);
}

void TsunamiWindow::on_mixing_console()
{
	bottom_bar->open(BottomBar::MIXING_CONSOLE);
}

void TsunamiWindow::on_fx_console()
{
	side_bar->open(SideBar::FX_CONSOLE);
}

void TsunamiWindow::on_sample_import()
{
}

void TsunamiWindow::on_command(const string & id)
{
}

void TsunamiWindow::on_settings()
{
	SettingsDialog *dlg = new SettingsDialog(view, this);
	dlg->run();
	delete dlg;
}

void TsunamiWindow::on_track_import()
{
	if (session->storage->ask_open_import(this)){
		Track *t = song->add_track(SignalType::AUDIO_STEREO);
		session->storage->load_track(t->layers[0], hui::Filename, view->sel.range.start());
	}
}

void TsunamiWindow::on_remove_sample()
{
	song->delete_selected_samples(view->sel);
}

void TsunamiWindow::on_play_loop()
{
	view->renderer->loop_if_allowed = !view->renderer->loop_if_allowed;
	update_menu();
}

void TsunamiWindow::on_play()
{
	if (side_bar->is_active(SideBar::CAPTURE_CONSOLE))
		return;
	if (view->is_paused())
		view->pause(false);
	else
		view->play(view->get_playback_selection(false), true);
}

void TsunamiWindow::on_pause()
{
	if (side_bar->is_active(SideBar::CAPTURE_CONSOLE))
		return;
	view->pause(true);
}

void TsunamiWindow::on_stop()
{
	if (side_bar->is_active(SideBar::CAPTURE_CONSOLE))
		side_bar->_hide();
	else
		view->stop();
}

void TsunamiWindow::on_insert_sample()
{
	song->insert_selected_samples(view->sel);
}

void TsunamiWindow::on_record()
{
	side_bar->open(SideBar::CAPTURE_CONSOLE);
}

void TsunamiWindow::on_add_layer()
{
	view->cur_track()->add_layer();
}

void TsunamiWindow::on_delete_layer()
{
	if (view->cur_track()->layers.num > 1)
		view->cur_track()->delete_layer(view->cur_layer());
	else
		session->e(_("can not delete the only version of a track"));
}

void TsunamiWindow::on_layer_make_track()
{
	view->cur_layer()->make_own_track();
}

void TsunamiWindow::on_layer_merge()
{
	view->cur_track()->merge_layers();
}

void TsunamiWindow::on_layer_mark_selection_dominant()
{
	view->cur_layer()->mark_dominant(view->sel.range);
}

void TsunamiWindow::on_sample_from_selection()
{
	song->create_samples_from_selection(view->sel, false);
}

void TsunamiWindow::on_view_optimal()
{
	view->optimize_view();
}

void TsunamiWindow::on_select_none()
{
	view->select_none();
}

void TsunamiWindow::on_select_all()
{
	view->select_all();
}

void TsunamiWindow::on_select_expand()
{
	view->select_expand();
}

void TsunamiWindow::on_view_midi_default()
{
	view->set_midi_view_mode(MidiMode::LINEAR);
}

void TsunamiWindow::on_view_midi_tab()
{
	view->set_midi_view_mode(MidiMode::TAB);
}

void TsunamiWindow::on_view_midi_score()
{
	view->set_midi_view_mode(MidiMode::CLASSICAL);
}

void TsunamiWindow::on_zoom_in()
{
	view->zoom_in();
}

void TsunamiWindow::on_zoom_out()
{
	view->zoom_out();
}

void TsunamiWindow::update_menu()
{
// menu / toolbar
	// edit
	enable("undo", song->action_manager->undoable());
	enable("redo", song->action_manager->redoable());
	enable("copy", app->clipboard->can_copy(view));
	enable("paste", app->clipboard->has_data());
	enable("delete", !view->sel.is_empty());
	// file
	//Enable("export_selection", true);
	// bars
	enable("delete_time", !view->sel.range.empty());
	enable("delete_bars", view->sel.bars.num > 0);
	enable("edit_bars", view->sel.bars.num > 0);
	enable("scale_bars", view->sel.bars.num > 0);
	// sample
	enable("sample_from_selection", !view->sel.range.empty());
	enable("insert_sample", view->sel.num_samples() > 0);
	enable("remove_sample", view->sel.num_samples() > 0);
	enable("sample_properties", view->cur_sample);
	// sound
	enable("play", !side_bar->is_active(SideBar::CAPTURE_CONSOLE));
	enable("stop", view->is_playback_active() or side_bar->is_active(SideBar::CAPTURE_CONSOLE));
	enable("pause", view->is_playback_active() and !view->is_paused());
	check("play_loop", view->renderer->loop_if_allowed);
	enable("record", !side_bar->is_active(SideBar::CAPTURE_CONSOLE));
	// view
	check("show_mixing_console", bottom_bar->is_active(BottomBar::MIXING_CONSOLE));
	check("show_fx_console", side_bar->is_active(SideBar::FX_CONSOLE));
	check("sample_manager", side_bar->is_active(SideBar::SAMPLE_CONSOLE));

	string title = title_filename(song->filename) + " - " + AppName;
	if (!song->action_manager->is_save())
		title = "*" + title;
	set_title(title);
}

void TsunamiWindow::on_side_bar_update()
{
	if (!side_bar->visible)
		activate(view->id);
	update_menu();
}

void TsunamiWindow::on_bottom_bar_update()
{
	if (!bottom_bar->visible)
		activate(view->id);
	update_menu();
}

void TsunamiWindow::on_update()
{
	// "Clipboard", "AudioFile" or "AudioView"
	update_menu();
}


void TsunamiWindow::on_exit()
{
	if (allow_termination()){
		BackupManager::set_save_state(session);
		destroy();
	}
}


void TsunamiWindow::on_new()
{
	NewDialog *dlg = new NewDialog(this);
	dlg->run();
	delete dlg;
	//BackupManager::set_save_state();
}


void TsunamiWindow::on_open()
{
	if (session->storage->ask_open(this)){
		if (song->is_empty()){
			if (session->storage->load(song, hui::Filename))
				BackupManager::set_save_state(session);
		}else{
			Session *s = tsunami->create_session();
			s->win->show();
			s->storage->load(s->song, hui::Filename);
		}
	}
}


void TsunamiWindow::on_save()
{
	if (song->filename == ""){
		on_save_as();
	}else{
		if (session->storage->save(song, song->filename)){
			view->set_message(_("file saved"));
			BackupManager::set_save_state(session);
		}
	}
}

string _suggest_filename(Song *s, const string &dir)
{
	if (s->filename != "")
		return s->filename.basename();
	string base = get_current_date().format("%Y-%m-%d");

	string ext = "nami";
	if ((s->tracks.num == 1) and (s->tracks[0]->type == SignalType::AUDIO))
		ext = "ogg";
	bool allow_midi = true;
	for (Track* t: s->tracks)
		if ((t->type != SignalType::MIDI) and (t->type != SignalType::BEATS))
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

void TsunamiWindow::on_save_as()
{
	if (song->filename == "")
		hui::file_dialog_default = _suggest_filename(song, session->storage->current_directory);

	if (session->storage->ask_save(this)){
		if (session->storage->save(song, hui::Filename))
			view->set_message(_("file saved"));
	}

	hui::file_dialog_default = "";
}

void TsunamiWindow::on_export()
{
	if (session->storage->ask_save_export(this)){
		SongRenderer renderer(song);
		renderer.prepare(view->get_playback_selection(false), false);
		renderer.allow_tracks(view->get_selected_tracks());
		if (session->storage->save_via_renderer(renderer.out, hui::Filename, renderer.get_num_samples(), song->tags))
			view->set_message(_("file exported"));
	}
}

void TsunamiWindow::on_quick_export()
{
	string dir = hui::Config.get_str("QuickExportDir", hui::Application::directory);
	if (session->storage->save(song, dir + _suggest_filename(song, dir)))
		view->set_message(_("file saved"));
}

int pref_bar_index(AudioView *view)
{
	if (view->sel.bar_gap >= 0)
		return view->sel.bar_gap;
	if (!view->sel.bar_indices.empty())
		return view->sel.bar_indices.end() + 1;
	if (view->hover_before_leave.pos > 0)
		return view->song->bars.num;
	return 0;
}

void TsunamiWindow::on_add_bars()
{
	auto dlg = new BarAddDialog(win, song, pref_bar_index(view));
	dlg->run();
	delete dlg;
}

void TsunamiWindow::on_add_pause()
{
	auto *dlg = new PauseAddDialog(win, song, pref_bar_index(view));
	dlg->run();
	delete dlg;
}

void TsunamiWindow::on_delete_bars()
{
	auto *dlg = new BarDeleteDialog(win, song, view->sel.bar_indices);
	dlg->run();
	delete dlg;
}

void TsunamiWindow::on_delete_time_interval()
{
	hui::ErrorBox(this, "todo", "todo");
	/*song->action_manager->beginActionGroup();

	for (int i=view->sel.bars.end()-1; i>=view->sel.bars.start(); i--){
		song->deleteBar(i, view->bars_edit_data);
	}
	song->action_manager->endActionGroup();*/
}

void TsunamiWindow::on_insert_time_interval()
{
	hui::ErrorBox(this, "todo", "todo");
	/*song->action_manager->beginActionGroup();

	for (int i=view->sel.bars.end()-1; i>=view->sel.bars.start(); i--){
		song->deleteBar(i, view->bars_edit_data);
	}
	song->action_manager->endActionGroup();*/
}

void TsunamiWindow::on_edit_bars()
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
		hui::Dialog *dlg = new BarEditDialog(win, song, view->sel.bar_indices);
		dlg->run();
		delete dlg;
	}else if (num_bars == 0 and num_pauses == 1){
		hui::Dialog *dlg = new PauseEditDialog(win, song, view->sel.bar_indices.start());
		dlg->run();
		delete dlg;
	}else{
		hui::ErrorBox(this, _("Error"), _("Can only edit bars or a single pause at a time."));
	}
}

void TsunamiWindow::on_scale_bars()
{
	view->set_mode(view->mode_scale_bars);
	Set<int> s;
	for (int i=view->sel.bar_indices.start(); i<view->sel.bar_indices.end(); i++)
		s.add(i);
	view->mode_scale_bars->start_scaling(s);
}
