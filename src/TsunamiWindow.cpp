/*
 * Tsunami.cpp
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#include "TsunamiWindow.h"
#include "Session.h"
#include "EditModes.h"

#include "Module/Audio/SongRenderer.h"
#include "Tsunami.h"
#include "View/Dialog/NewDialog.h"
#include "View/Dialog/HelpDialog.h"
#include "View/Dialog/SettingsDialog.h"
#include "View/Dialog/MarkerDialog.h"
#include "View/Dialog/BarAddDialog.h"
#include "View/Dialog/BarDeleteDialog.h"
#include "View/Dialog/BarEditDialog.h"
#include "View/Dialog/PauseAddDialog.h"
#include "View/Dialog/PauseEditDialog.h"
#include "View/Dialog/TrackRoutingDialog.h"
#include "View/Dialog/QuestionDialog.h"
#include "View/BottomBar/BottomBar.h"
#include "View/BottomBar/MiniBar.h"
//#include "View/BottomBar/DeviceConsole.h"
#include "View/SideBar/SideBar.h"
#include "View/SideBar/CaptureConsole.h"
#include "View/Mode/ViewModeDefault.h"
#include "View/Mode/ViewModeMidi.h"
#include "View/Mode/ViewModeEdit.h"
#include "View/Mode/ViewModeCapture.h"
#include "View/Mode/ViewModeScaleBars.h"
#include "View/Helper/Slider.h"
#include "View/Helper/Progress.h"
#include "View/Helper/ModulePanel.h"
#include "View/AudioView.h"
#include "Plugins/PluginManager.h"
#include "Plugins/TsunamiPlugin.h"
#include "Plugins/SongPlugin.h"
#include "Storage/Storage.h"
#include "Stuff/Log.h"
#include "Stuff/Clipboard.h"
#include "Stuff/BackupManager.h"
#include "Data/base.h"
#include "Data/Track.h"
#include "Data/TrackLayer.h"
#include "Data/Song.h"
#include "Data/SongSelection.h"
#include "Data/Rhythm/Bar.h"
#include "Action/ActionManager.h"
#include "Action/Track/Buffer/ActionTrackEditBuffer.h"
#include "Action/Song/ActionSongMoveSelection.h"
#include "Module/Audio/AudioEffect.h"
#include "Module/Audio/AudioSource.h"
#include "Module/Midi/MidiEffect.h"
#include "Module/Midi/MidiSource.h"
#include "Module/ConfigPanel.h"
#include "Module/SignalChain.h"
#include "Plugins/FastFourierTransform.h"
#include "View/Helper/PeakMeterDisplay.h"
#include "lib/hui/hui.h"
#include "Device/DeviceManager.h"
#include "View/Graph/AudioViewLayer.h"
#include "View/Graph/AudioViewTrack.h"

extern const string AppName;

namespace hui {
extern string file_dialog_default;
}

TsunamiWindow::TsunamiWindow(Session *_session) :
		hui::Window(AppName, 800, 600) {
	session = _session;
	session->set_win(this);
	song = session->song.get();
	app = tsunami;

	int width = hui::Config.get_int("Window.Width", 800);
	int height = hui::Config.get_int("Window.Height", 600);
	bool maximized = hui::Config.get_bool("Window.Maximized", true);

	event("new", [=]{ on_new(); });
	set_key_code("new", hui::KEY_N + hui::KEY_CONTROL);
	event("open", [=]{ on_open(); });
	set_key_code("open", hui::KEY_O + hui::KEY_CONTROL);
	event("save", [=]{ on_save(); });
	set_key_code("save", hui::KEY_S + hui::KEY_CONTROL);
	event("save_as", [=]{ on_save_as(); });
	set_key_code("save_as", hui::KEY_S + hui::KEY_CONTROL + hui::KEY_SHIFT);
	event("copy", [=]{ on_copy(); });
	set_key_code("copy", hui::KEY_C + hui::KEY_CONTROL);
	event("paste", [=]{ on_paste(); });
	set_key_code("paste", hui::KEY_V + hui::KEY_CONTROL);
	event("paste_as_samples", [=]{ on_paste_as_samples(); });
	set_key_code("paste_as_samples", hui::KEY_V + hui::KEY_CONTROL + hui::KEY_SHIFT);
	event("paste_time", [=]{ on_paste_time(); });
	event("delete", [=]{ on_delete(); });
	set_key_code("delete", hui::KEY_DELETE);
	event("delete-shift", [=]{ on_delete_shift(); });
	set_key_code("delete-shift", hui::KEY_SHIFT + hui::KEY_DELETE);
	event("render_export_selection", [=]{ on_render_export_selection(); });
	set_key_code("render_export_selection", hui::KEY_X + hui::KEY_CONTROL);
	event("export_selection", [=]{ on_export_selection(); });
	event("quick_export", [=]{ on_quick_export(); });
	set_key_code("quick_export", hui::KEY_X + hui::KEY_CONTROL + hui::KEY_SHIFT);
	event("undo", [=]{ on_undo(); });
	set_key_code("undo", hui::KEY_Z + hui::KEY_CONTROL);
	event("redo", [=]{ on_redo(); });
	set_key_code("redo", hui::KEY_Y + hui::KEY_CONTROL);
	event("track_render", [=]{ on_track_render(); });
	event("track-add-audio-mono", [=]{ on_add_audio_track_mono(); });
	event("track-add-audio-stereo", [=]{ on_add_audio_track_stereo(); });
	event("track-add-group", [=]{ song->add_track(SignalType::GROUP); });
	event("track-add-beats", [=]{ on_add_time_track(); });
	event("track-add-midi", [=]{ on_add_midi_track(); });
	event("track-delete", [=]{ on_delete_track(); });
	//event("layer-edit-midi", [=]{ on_track_edit_midi(); });
	//set_key_code("layer-edit-midi", hui::KEY_ALT + hui::KEY_E);
	event("mode-edit-check", [=]{
		if (view->mode == view->mode_edit)
			session->set_mode(EditMode::Default);
		else
			session->set_mode(EditMode::EditTrack);
	});
	event("layer-edit", [=]{ on_track_edit_midi(); });
	set_key_code("layer-edit", hui::KEY_ALT + hui::KEY_E);
	event("layer-edit-curves", [=]{ session->set_mode(EditMode::Curves); });
	event("track-edit-fx", [=]{ on_track_edit_fx(); });
	event("track-add-marker", [=]{ on_track_add_marker(); });
	set_key_code("track-add-marker", hui::KEY_CONTROL + hui::KEY_J);
	event("track-convert-mono", [=]{ on_track_convert_mono(); });
	event("track-convert-stereo", [=]{ on_track_convert_stereo(); });
	event("buffer-delete", [=]{ on_buffer_delete(); });
	event("buffer-make-movable", [=]{ on_buffer_make_movable(); });

	event("edit-track-groups", [=]{ auto *dlg = new TrackRoutingDialog(this, song); dlg->run(); delete dlg; });
	set_key_code("edit-track-groups", hui::KEY_G + hui::KEY_CONTROL);

	event("track-midi-mode-linear", [=]{ on_layer_midi_mode_linear(); });
	event("track-midi-mode-tab", [=]{ on_layer_midi_mode_tab(); });
	event("track-midi-mode-classical", [=]{ on_layer_midi_mode_classical(); });
	
	//event("track-muted", [=]{ view->cur_track()->set_muted(!view->cur_track()->muted); });
	set_key_code("track-muted", hui::KEY_ALT + hui::KEY_M);
	//event("track-solo", [=]{ view->cur_vtrack()->set_solo(!view->cur_vtrack()->solo); });
	set_key_code("track-solo", hui::KEY_ALT + hui::KEY_S);
	set_key_code("layer-muted", hui::KEY_ALT + hui::KEY_SHIFT + hui::KEY_M);
	set_key_code("layer-solo", hui::KEY_ALT + hui::KEY_SHIFT + hui::KEY_S);
	set_key_code("track-explode", hui::KEY_ALT + hui::KEY_X);
	set_key_code("layer-up", hui::KEY_UP);
	set_key_code("layer-down", hui::KEY_DOWN);
	set_key_code("layer-expand-up", hui::KEY_SHIFT + hui::KEY_UP);
	set_key_code("layer-expand-down", hui::KEY_SHIFT + hui::KEY_DOWN);

	event("layer-add", [=]{ on_add_layer(); });
	event("layer-delete", [=]{ on_delete_layer(); });
	set_key_code("layer-delete", hui::KEY_CONTROL + hui::KEY_DELETE);
	event("layer-make-track", [=]{ on_layer_make_track(); });
	event("layer-merge", [=]{ on_layer_merge(); });
	event("layer-mark-dominant", [=]{ on_layer_mark_selection_dominant(); });
	set_key_code("layer-mark-dominant", hui::KEY_ALT + hui::KEY_D);
	event("layer-add-dominant", [=]{ on_layer_add_selection_dominant(); });
	event("bars-add", [=]{ on_add_bars(); });
	event("bars-add-pause", [=]{ on_add_pause(); });
	event("bars-delete", [=]{ on_delete_bars(); });
	event("delete_time", [=]{ on_delete_time_interval(); });
	event("insert_time", [=]{ on_insert_time_interval(); });
	event("bars-edit", [=]{ on_edit_bars(); });
	event("bars-scale", [=]{ on_scale_bars(); });
	event("sample_manager", [=]{ on_sample_manager(); });
	event("song-edit-samples", [=]{ on_sample_manager(); });
	event("show-mixing-console", [=]{ on_mixing_console(); });
	set_key_code("show-mixing-console", hui::KEY_CONTROL + hui::KEY_M);
	event("show-devices", [=]{ on_settings(); });
	event("show-signal-chain", [=]{ bottom_bar->toggle(BottomBar::SIGNAL_EDITOR); });
	event("show-mastering-console", [=]{ on_mastering_console(); });
	event("show-fx-console", [=]{ on_fx_console(); });
	event("sample_from_selection", [=]{ on_sample_from_selection(); });
	event("sample-insert", [=]{ on_insert_sample(); });
	set_key_code("sample-insert", hui::KEY_I + hui::KEY_CONTROL);
	event("sample-delete", [=]{ on_remove_sample(); });
	event("marker-delete", [=]{ on_delete_marker(); });
	event("marker-edit", [=]{ on_edit_marker(); });
	event("marker-resize", [=]{ on_marker_resize(); });
	event("track_import", [=]{ on_track_import(); });
	event("sub_import", [=]{ on_sample_import(); });
	event("song-properties", [=]{ on_song_properties(); });
	set_key_code("song-properties", hui::KEY_F4);
	event("track-properties", [=]{ on_track_properties(); });
	event("sample-properties", [=]{ on_sample_properties(); });
	event("settings", [=]{ on_settings(); });
	event("play", [=]{ on_play(); });
	event("play-toggle", [=]{ on_play_toggle(); });
	set_key_code("play-toggle", hui::KEY_SPACE);
	event("play-loop", [=]{ on_play_loop(); });
	set_key_code("play-loop", hui::KEY_CONTROL + hui::KEY_L);
	event("pause", [=]{ on_pause(); });
	event("stop", [=]{ on_stop(); });
	set_key_code("stop", hui::KEY_CONTROL + hui::KEY_T);
	event("record", [=]{ on_record(); });
	set_key_code("record", hui::KEY_CONTROL + hui::KEY_R);
	event("playback-range-lock", [=]{ view->set_playback_range_locked(!view->playback_range_locked); });
	event("show-log", [=]{ on_show_log(); });
	event("about", [=]{ on_about(); });
	event("help", [=]{ on_help(); });
	set_key_code("run_plugin", hui::KEY_SHIFT + hui::KEY_RETURN);
	event("exit", [=]{ on_exit(); });
	set_key_code("exit", hui::KEY_CONTROL + hui::KEY_Q);
	event("select_all", [=]{ on_select_all(); });
	set_key_code("select_all", hui::KEY_CONTROL + hui::KEY_A);
	event("select_nothing", [=]{ on_select_none(); });
	event("select_expand", [=]{ on_select_expand(); });
	set_key_code("select_expand", hui::KEY_TAB + hui::KEY_SHIFT);
	event("view-midi-linear", [=]{ on_view_midi_default(); });
	event("view-midi-tab", [=]{ on_view_midi_tab(); });
	event("view-midi-classical", [=]{ on_view_midi_score(); });
	event("view-optimal", [=]{ on_view_optimal(); });
	event("zoom-in", [=]{ on_zoom_in(); });
	set_key_code("zoom-in", hui::KEY_PLUS);
	event("zoom-out", [=]{ on_zoom_out(); });
	set_key_code("zoom-out", hui::KEY_MINUS);

	set_key_code("vertical-zoom-in", hui::KEY_PLUS + hui::KEY_SHIFT);
	set_key_code("vertical-zoom-out", hui::KEY_MINUS + hui::KEY_SHIFT);

	set_key_code("cam-move-right", hui::KEY_PAGE_DOWN);
	set_key_code("cam-move-left", hui::KEY_PAGE_UP);
	set_key_code("cursor-jump-start", hui::KEY_HOME);
	set_key_code("cursor-jump-end", hui::KEY_END);
	set_key_code("cursor-move-left", hui::KEY_LEFT);
	set_key_code("cursor-move-right", hui::KEY_RIGHT);
	set_key_code("cursor-expand-left", hui::KEY_LEFT + hui::KEY_SHIFT);
	set_key_code("cursor-expand-right", hui::KEY_RIGHT + hui::KEY_SHIFT);

	// table structure
	set_size(width, height);
	set_border_width(0);
	set_spacing(0);
	add_grid("", 0, 0, "root-grid");
	set_target("root-grid");
	add_grid("", 1, 0, "main-grid");

	// main table
	set_target("main-grid");
	add_drawing_area("!grabfocus,gesture=zoom", 0, 0, "area");
	if (hui::Config.get_bool("View.EventCompression", true) == false)
		set_options("area", "noeventcompression");

	toolbar[0]->set_by_id("toolbar");
	//ToolbarConfigure(false, true);

	set_menu(hui::CreateResourceMenu("menu"));
	//ToolBarConfigure(true, true);
	set_maximized(maximized);

	app->plugin_manager->add_plugins_to_menu(this);

	// events
	event("hui:close", [=]{ on_exit(); });

	for (int i=0; i<256; i++) {
		event("import-backup-" + i2s(i), [=]{ on_import_backup(); });
		event("delete-backup-" + i2s(i), [=]{ on_delete_backup(); });
	}

	view = new AudioView(session, "area");
	session->view = view;

	// side bar
	side_bar = new SideBar(session);
	embed(side_bar, "root-grid", 2, 0);

	// bottom bar
	bottom_bar = new BottomBar(session);
	embed(bottom_bar, "main-grid", 0, 1);
	//mini_bar = new MiniBar(bottom_bar, session);
	//embed(mini_bar.get(), "main-grid", 0, 2);

	view->subscribe(this, [=]{ on_update(); }, view->MESSAGE_SETTINGS_CHANGE);
	view->subscribe(this, [=]{ on_update(); }, view->MESSAGE_SELECTION_CHANGE);
	view->subscribe(this, [=]{ on_update(); }, view->MESSAGE_CUR_LAYER_CHANGE);
	view->subscribe(this, [=]{ on_update(); }, view->MESSAGE_CUR_SAMPLE_CHANGE);
	view->signal_chain->subscribe(this, [=]{ on_update(); });
	song->action_manager->subscribe(this, [=]{ on_update(); });
	app->clipboard->subscribe(this, [=]{ on_update(); });
	bottom_bar->subscribe(this, [=]{ on_bottom_bar_update(); });
	side_bar->subscribe(this, [=]{ on_side_bar_update(); });
	
	event("*", [=]{ view->on_command(hui::GetEvent()->id); });

	// firt time start?
	if (hui::Config.get_bool("FirstStart", true)) {
		hui::RunLater(0.2f, [=]{
			on_help();
			hui::Config.set_bool("FirstStart", false);
		});
	}

	update_menu();
}

void TsunamiCleanUp(Session *session) {
	foreachi(Session *s, weak(tsunami->sessions), i)
		if (s == session and s->auto_delete) {
			//msg_write("--------Tsunami erase...");
			tsunami->sessions.erase(i);
		}

	//msg_write(tsunami->sessions.num);
	if (tsunami->sessions.num == 0)
		tsunami->end();
}

TsunamiWindow::~TsunamiWindow() {
	int w, h;
	get_size_desired(w, h);
	hui::Config.set_int("Window.Width", w);
	hui::Config.set_int("Window.Height", h);
	hui::Config.set_bool("Window.Maximized", is_maximized());

	view->signal_chain->stop_hard();
	view->unsubscribe(this);
	view->signal_chain->unsubscribe(this);
	song->action_manager->unsubscribe(this);
	app->clipboard->unsubscribe(this);
	bottom_bar->unsubscribe(this);
	side_bar->unsubscribe(this);

	delete side_bar;
	delete bottom_bar;
	delete view;

	hui::RunLater(0.010f, [=]{ TsunamiCleanUp(session); });
}

void TsunamiWindow::on_about() {
	hui::AboutBox(this);
}

void TsunamiWindow::on_help() {
	auto dlg = ownify(new HelpDialog(this));
	dlg->run();
}

void TsunamiWindow::on_add_audio_track_mono() {
	song->add_track(SignalType::AUDIO_MONO);
}

void TsunamiWindow::on_add_audio_track_stereo() {
	song->add_track(SignalType::AUDIO_STEREO);
}

void TsunamiWindow::on_add_time_track() {
	song->begin_action_group();
	try {
		song->add_track(SignalType::BEATS, 0);

		// some default data
		auto b = BarPattern(0, 4, 1);
		b.set_bpm(90, song->sample_rate);
		for (int i=0; i<10; i++)
			song->add_bar(-1, b, false);
	} catch (Exception &e) {
		session->e(e.message());
	}
	song->end_action_group();
}

void TsunamiWindow::on_import_backup() {
	string id = hui::GetEvent()->id;
	int uuid = id.explode(":").back()._int();
	auto filename = BackupManager::get_filename_for_uuid(uuid);
	if (filename.is_empty())
		return;

	Storage::options_in = "format:f32,channels:2,samplerate:44100";
	if (song->is_empty()) {
		session->storage->load(song, filename);
		//BackupManager::set_save_state(session);
	} else {
		Session *s = tsunami->create_session();
		s->win->show();
		s->storage->load(s->song.get(), filename);
	}
	Storage::options_in = "";

	//BackupManager::del
}

void TsunamiWindow::on_delete_backup() {
	string id = hui::GetEvent()->id;
	int uuid = id.explode(":").back()._int();
	BackupManager::delete_old(uuid);
}

void TsunamiWindow::on_add_midi_track() {
	song->add_track(SignalType::MIDI);
}

void write_into_buffer(Port *out, AudioBuffer &buf, int len, Progress *prog = nullptr) {
	const int chunk_size = 1 << 12;
	int offset = 0;

	while (offset < len) {
		if (prog)
			prog->set((float) offset / len);

		Range r = RangeTo(offset, min(offset + chunk_size, len));

		AudioBuffer tbuf;
		tbuf.set_as_ref(buf, offset, r.length);

		out->read_audio(tbuf);

		offset += chunk_size;
		if (prog)
			if (prog->is_cancelled())
				break;
	}
}


void TsunamiWindow::on_track_render() {
	Range range = view->sel.range();
	if (range.is_empty()) {
		session->e(_("Selection range is empty"));
		return;
	}

	SongRenderer renderer(song);
	renderer.set_range(range);

	renderer.allow_layers(view->get_playable_layers());
	if (view->get_playable_layers() != view->sel.layers()) {
		int answer = QuestionDialogMultipleChoice::ask(this, _("Question"), _("Which tracks and layers should be rendered?"),
				{_("All non-muted"), _("From selection")},
				{_("respecting solo and mute, ignoring selection"), _("respecting selection and mute, but ignoring solo")}, true);
		if (answer == 1)
			renderer.allow_layers(view->sel.layers());
		else if (answer < 0)
			return;
	}

	auto p = ownify(new ProgressCancelable(_(""), this));

	AudioBuffer buf;
	buf.resize(range.length);

	write_into_buffer(renderer.out, buf, range.length, p.get());

	song->begin_action_group();
	Track *t = song->add_track(SignalType::AUDIO_STEREO);
	AudioBuffer buf_track;
	auto *a = t->layers[0]->edit_buffers(buf_track, range);
	buf_track.set(buf, 0, 1);

	t->layers[0]->edit_buffers_finish(a);
	song->end_action_group();

}

void TsunamiWindow::on_delete_track() {
	if (view->cur_track()) {
		try {
			song->delete_track(view->cur_track());
		} catch (Exception &e) {
			session->e(e.message());
		}
	} else {
		session->e(_("No track selected"));
	}
}

void TsunamiWindow::on_track_edit_midi() {
	session->set_mode(EditMode::EditTrack);
}

void TsunamiWindow::on_track_edit_fx() {
	session->set_mode(EditMode::DefaultFx);
}

void TsunamiWindow::on_track_add_marker() {
	if (view->cur_track()) {
		Range range = view->sel.range();
		auto dlg = ownify(new MarkerDialog(this, view->cur_layer(), range, ""));
		dlg->run();
	} else {
		session->e(_("No track selected"));
	}
}

void TsunamiWindow::on_track_convert_mono() {
	if (view->cur_track())
		view->cur_track()->set_channels(1);
	else
		session->e(_("No track selected"));
}

void TsunamiWindow::on_track_convert_stereo() {
	if (view->cur_track())
		view->cur_track()->set_channels(2);
	else
		session->e(_("No track selected"));
}
void TsunamiWindow::on_buffer_delete() {
	foreachi (auto &buf, view->cur_layer()->buffers, i)
		if (buf.range().is_inside(view->hover_before_leave.pos)) {
			auto s = SongSelection::from_range(song, buf.range()).filter({view->cur_layer()}).filter(0);
			song->delete_selection(s);
		}
}

void TsunamiWindow::on_buffer_make_movable() {
	for (auto &buf: view->cur_layer()->buffers) {
		if (buf.range().is_inside(view->hover_before_leave.pos)) {
			auto s = SongSelection::from_range(song, buf.range()).filter({view->cur_layer()}).filter(0);
			song->create_samples_from_selection(s, true);
		}
	}
}

void TsunamiWindow::on_layer_midi_mode_linear() {
	view->cur_vtrack()->set_midi_mode(MidiMode::LINEAR);
}

void TsunamiWindow::on_layer_midi_mode_tab() {
	view->cur_vtrack()->set_midi_mode(MidiMode::TAB);
}

void TsunamiWindow::on_layer_midi_mode_classical() {
	view->cur_vtrack()->set_midi_mode(MidiMode::CLASSICAL);
}

void TsunamiWindow::on_song_properties() {
	session->set_mode(EditMode::DefaultSong);
}

void TsunamiWindow::on_track_properties() {
	session->set_mode(EditMode::DefaultTrack);
}

void TsunamiWindow::on_sample_properties() {
	session->set_mode(EditMode::DefaultSampleRef);
}

void TsunamiWindow::on_delete_marker() {
	if (view->cur_selection.marker)
		view->cur_layer()->delete_marker(view->cur_selection.marker);
	else
		session->e(_("No marker selected"));
}

void TsunamiWindow::on_edit_marker() {
	if (view->cur_selection.marker) {
		auto dlg = ownify(new MarkerDialog(this, view->cur_layer(), view->cur_selection.marker));
		dlg->run();
	} else {
		session->e(_("No marker selected"));
	}
}

void TsunamiWindow::on_marker_resize() {
	session->set_mode(EditMode::ScaleMarker);
}

void TsunamiWindow::on_show_log() {
	bottom_bar->open(BottomBar::LOG_CONSOLE);
}

void TsunamiWindow::on_undo() {
	song->undo();
}

void TsunamiWindow::on_redo() {
	song->redo();
}

void TsunamiWindow::on_send_bug_report() {
}

string title_filename(const Path &filename) {
	if (!filename.is_empty())
		return filename.basename();
	return _("No name");
}

bool TsunamiWindow::allow_termination() {
	if (!side_bar->allow_close())
		return false;

	if (song->action_manager->is_save())
		return true;
	string answer = hui::QuestionBox(this, _("Question"), format(_("'%s'\nSave file?"), title_filename(song->filename)), true);
	if (answer == "hui:yes") {
		/*if (!OnSave())
		 return false;*/
		on_save();
		return true;
	} else if (answer == "hui:no") {
		return true;
	}

	// cancel
	return false;
}

void TsunamiWindow::on_copy() {
	app->clipboard->copy(view);
}

void TsunamiWindow::on_paste() {
	app->clipboard->paste(view);
}

void TsunamiWindow::on_paste_as_samples() {
	app->clipboard->paste_as_samples(view);
}

void TsunamiWindow::on_paste_time() {
	app->clipboard->paste_with_time(view);
}

void fx_process_layer(TrackLayer *l, const Range &r, AudioEffect *fx, hui::Window *win) {
	auto p = ownify(new Progress(_("applying effect"), win));
	fx->reset_state();

	AudioBuffer buf;
	auto *a = l->edit_buffers(buf, r);

	int chunk_size = 2048;
	int done = 0;
	while (done < r.length) {
		p->set((float) done / (float) r.length);

		auto ref = buf.ref(done, min(done + chunk_size, r.length));
		fx->process(ref);
		done += chunk_size;
	}

	l->edit_buffers_finish(a);
}

void source_process_layer(TrackLayer *l, const Range &r, AudioSource *fx, hui::Window *win) {
	auto p = ownify(new Progress(_("applying source"), win));
	fx->reset_state();
	
	AudioBuffer buf;
	auto *a = l->edit_buffers(buf, r);
	buf.set_zero();

	int chunk_size = 2048;
	int done = 0;
	while (done < r.length) {
		p->set((float) done / (float) r.length);

		auto ref = buf.ref(done, min(done + chunk_size, r.length));
		fx->read(ref);
		done += chunk_size;
	}

	l->edit_buffers_finish(a);
}

void TsunamiWindow::on_menu_execute_audio_effect() {
	string name = hui::GetEvent()->id.explode("--")[1];
	int n_layers = 0;

	auto fx = ownify(CreateAudioEffect(session, name));

	if (!configure_module(this, fx.get()))
		return;
	song->begin_action_group();
	for (Track *t: weak(song->tracks))
		for (auto *l: weak(t->layers))
			if (view->sel.has(l) and (t->type == SignalType::AUDIO)) {
				fx_process_layer(l, view->sel.range(), fx.get(), this);
				n_layers ++;
			}
	song->end_action_group();
	
	if (n_layers == 0)
		session->e(_("no audio tracks selected"));
}

void TsunamiWindow::on_menu_execute_audio_source() {
	string name = hui::GetEvent()->id.explode("--")[1];
	int n_layers = 0;

	auto s = ownify(CreateAudioSource(session, name));

	if (!configure_module(this, s.get()))
		return;
	song->begin_action_group();
	for (Track *t: weak(song->tracks))
		for (auto *l: weak(t->layers))
			if (view->sel.has(l) and (t->type == SignalType::AUDIO)) {
				source_process_layer(l, view->sel.range(), s.get(), this);
				n_layers ++;
			}
	song->end_action_group();
	
	if (n_layers == 0)
		session->e(_("no audio tracks selected"));
}

void TsunamiWindow::on_menu_execute_midi_effect() {
	string name = hui::GetEvent()->id.explode("--")[1];
	int n_layers = 0;

	auto fx = ownify(CreateMidiEffect(session, name));

	if (!configure_module(this, fx.get()))
		return;
	
	song->action_manager->group_begin();
	for (Track *t: weak(song->tracks))
		for (auto *l: weak(t->layers))
			if (view->sel.has(l) and (t->type == SignalType::MIDI)) {
				fx->reset_state();
				fx->process_layer(l, view->sel);
				n_layers ++;
			}
	song->action_manager->group_end();
	
	if (n_layers == 0)
		session->e(_("no midi tracks selected"));
}

void TsunamiWindow::on_menu_execute_midi_source() {
	string name = hui::GetEvent()->id.explode("--")[1];
	int n_layers = 0;

	auto s = ownify(CreateMidiSource(session, name));

	if (!configure_module(this, s.get()))
		return;
	
	song->begin_action_group();
	for (Track *t: weak(song->tracks))
		for (auto *l: weak(t->layers))
			if (view->sel.has(l) and (t->type == SignalType::MIDI)) {
				s->reset_state();
				MidiEventBuffer buf;
				buf.samples = view->sel.range().length;
				s->read(buf);
				l->insert_midi_data(view->sel.range().offset, midi_events_to_notes(buf));
				n_layers ++;
			}
	song->end_action_group();
	
	if (n_layers == 0)
		session->e(_("no midi tracks selected"));
}

void TsunamiWindow::on_menu_execute_song_plugin() {
	string name = hui::GetEvent()->id.explode("--")[1];

	auto p = ownify(CreateSongPlugin(session, name));
	p->apply();
}

void TsunamiWindow::on_menu_execute_tsunami_plugin() {
	string name = hui::GetEvent()->id.explode("--")[1];

	session->execute_tsunami_plugin(name);
}

void TsunamiWindow::on_delete() {
	if (view->sel.is_empty())
		return;
	song->delete_selection(view->sel);
	//view->set_cursor_pos(view->cursor_range().start());
}

void TsunamiWindow::on_delete_shift() {
	if (view->sel.is_empty())
		return;
	song->begin_action_group();
	song->delete_selection(view->sel);
	auto sel = SongSelection::from_range(song, RangeTo(view->cursor_range().end(), 2000000000)).filter(view->sel.layers());
	auto a = new ActionSongMoveSelection(song, sel, true);
	a->set_param_and_notify(song, -view->sel.range().length);
	song->execute(a);
	song->end_action_group();

	view->set_cursor_pos(view->cursor_range().start());
}

void TsunamiWindow::on_sample_manager() {
	session->set_mode(EditMode::DefaultSamples);
}

void TsunamiWindow::on_mixing_console() {
	bottom_bar->toggle(BottomBar::MIXING_CONSOLE);
}

void TsunamiWindow::on_fx_console() {
	session->set_mode(EditMode::DefaultFx);
}

void TsunamiWindow::on_mastering_console() {
	session->set_mode(EditMode::DefaultMastering);
}

void TsunamiWindow::on_sample_import() {
}

void TsunamiWindow::on_command(const string & id) {
}

void TsunamiWindow::on_settings() {
	auto dlg = ownify(new SettingsDialog(view, this));
	dlg->run();
}

void TsunamiWindow::on_track_import() {
	if (session->storage->ask_open_import(this)) {
		Track *t = song->add_track(SignalType::AUDIO_STEREO);
		session->storage->load_track(t->layers[0].get(), hui::Filename, view->cursor_pos());
	}
}

void TsunamiWindow::on_remove_sample() {
	song->delete_selected_samples(view->sel);
}

void TsunamiWindow::on_play_loop() {
	view->set_playback_loop(!view->looping());
}

void TsunamiWindow::on_play() {
	if (session->in_mode(EditMode::Capture))
		return;

	view->play();
}

void TsunamiWindow::on_play_toggle() {
	if (session->in_mode(EditMode::Capture))
		return;

	if (view->is_playback_active()) {
		view->pause(!view->is_paused());
	} else {
		view->play();
	}
}

void TsunamiWindow::on_pause() {
	if (session->in_mode(EditMode::Capture))
		return;
	view->pause(true);
}

void TsunamiWindow::on_stop() {
	if (session->in_mode(EditMode::Capture)) {
		session->set_mode(EditMode::Default);
	} else {
		view->stop();
	}
}

void TsunamiWindow::on_insert_sample() {
	song->insert_selected_samples(view->sel);
}

void TsunamiWindow::on_record() {
	session->set_mode(EditMode::Capture);
}

void TsunamiWindow::on_add_layer() {
	view->cur_track()->add_layer();
}

void TsunamiWindow::on_delete_layer() {
	if (view->cur_track()->layers.num > 1)
		view->cur_track()->delete_layer(view->cur_layer());
	else
		session->e(_("can not delete the only version of a track"));
}

void TsunamiWindow::on_layer_make_track() {
	if (view->cur_track()->layers.num > 1)
		view->cur_layer()->make_own_track();
	else
		session->e(_("this is already the only version of the track"));
}

void TsunamiWindow::on_layer_merge() {
	view->cur_track()->merge_layers();
}

void TsunamiWindow::on_layer_mark_selection_dominant() {
	view->cur_track()->mark_dominant(view->sel.layers(), view->sel.range());
}

void TsunamiWindow::on_layer_add_selection_dominant() {
	//...
	//view->cur_layer()->mark_add_dominant(view->sel.range);
}

void TsunamiWindow::on_sample_from_selection() {
	song->create_samples_from_selection(view->sel, false);
}

void TsunamiWindow::on_view_optimal() {
	view->request_optimize_view();
}

void TsunamiWindow::on_select_none() {
	view->select_none();
}

void TsunamiWindow::on_select_all() {
	view->select_all();
}

void TsunamiWindow::on_select_expand() {
	view->select_expand();
}

void TsunamiWindow::on_view_midi_default() {
	view->set_midi_view_mode(MidiMode::LINEAR);
}

void TsunamiWindow::on_view_midi_tab() {
	view->set_midi_view_mode(MidiMode::TAB);
}

void TsunamiWindow::on_view_midi_score() {
	view->set_midi_view_mode(MidiMode::CLASSICAL);
}

void TsunamiWindow::on_zoom_in() {
	view->zoom_in();
}

void TsunamiWindow::on_zoom_out() {
	view->zoom_out();
}

void TsunamiWindow::update_menu() {
// menu / toolbar
	// edit
	enable("undo", song->action_manager->undoable());
	enable("redo", song->action_manager->redoable());
	enable("copy", app->clipboard->can_copy(view));
	enable("paste", app->clipboard->has_data());
	enable("delete", !view->sel.is_empty());
	enable("delete-shift", !view->sel.is_empty());

	check("mode-edit-check", view->mode == view->mode_edit);

	// file
	//Enable("export_selection", true);
	// bars
	enable("delete_time", !view->sel.range().is_empty());
	enable("bars-delete", view->sel._bars.num > 0);
	enable("bars-edit", view->sel._bars.num > 0);
	enable("bars-scale", view->sel._bars.num > 0);
	// sample
	enable("sample_from_selection", !view->sel.range().is_empty());
	enable("sample-insert", view->sel.num_samples() > 0);
	enable("sample-delete", view->sel.num_samples() > 0);
	enable("sample-properties", view->cur_sample());
	// sound
	enable("play", !session->in_mode(EditMode::Capture));
	enable("stop", view->is_playback_active() or session->in_mode(EditMode::Capture));
	enable("pause", view->is_playback_active() and !view->is_paused());
	check("play-loop", view->looping());
	enable("record", !session->in_mode(EditMode::Capture));
	// view
	check("show-mixing-console", bottom_bar->is_active(BottomBar::MIXING_CONSOLE));
	check("show_signal_chain", bottom_bar->is_active(BottomBar::SIGNAL_EDITOR));
	check("sample_manager", session->in_mode(EditMode::DefaultSamples));

	string title = title_filename(song->filename) + " - " + AppName;
	if (!song->action_manager->is_save())
		title = "*" + title;
	set_title(title);
}

void TsunamiWindow::on_side_bar_update() {
	if (!side_bar->visible)
		activate(view->id);
	update_menu();
}

bool view_should_show_peaks(AudioView *view);

void TsunamiWindow::on_bottom_bar_update() {
	if (!bottom_bar->visible)
		activate(view->id);
	view->peak_meter_display->hidden = !view_should_show_peaks(view);
	update_menu();
}

void TsunamiWindow::on_update() {
	// "Clipboard", "AudioFile" or "AudioView"
	update_menu();
}

void TsunamiWindow::on_exit() {
	if (allow_termination()) {
		BackupManager::set_save_state(session);
		//request_destroy();
		hui::RunLater(0.01f, [=]{ session->win = nullptr; });
	}
}

void TsunamiWindow::on_new() {
	auto dlg = ownify(new NewDialog(this));
	dlg->run();
}

void TsunamiWindow::on_open() {
	if (session->storage->ask_open(this)) {
		if (song->is_empty()) {
			if (session->storage->load(song, hui::Filename))
				BackupManager::set_save_state(session);
		} else {
			auto *s = tsunami->create_session();
			s->win->show();
			s->storage->load(s->song.get(), hui::Filename);
		}
	}
}

void TsunamiWindow::on_save() {
	if (song->filename == "") {
		on_save_as();
	} else {
		if (session->storage->save(song, song->filename)) {
			view->set_message(_("file saved"));
			BackupManager::set_save_state(session);
		}
	}
}

bool song_is_simple_audio(Song *s) {
	return ((s->tracks.num == 1) and (s->tracks[0]->type == SignalType::AUDIO) and (s->tracks[0]->layers.num == 1));
}

bool song_is_simple_midi(Song *s) {
	for (Track* t: weak(s->tracks))
		if ((t->type != SignalType::MIDI) and (t->type != SignalType::BEATS))
			return false;
	return true;
}

string _suggest_filename(Song *s, const Path &dir) {
	if (!s->filename.is_empty())
		return s->filename.basename();
	string base = Date::now().format("%Y-%m-%d");

	string ext = "nami";
	if (song_is_simple_audio(s))
		ext = "ogg";
	else if (song_is_simple_midi(s))
		ext = "midi";

	for (int i=0; i<26; i++) {
		string name = base + "a." + ext;
		name[name.num - ext.num - 2] += i;
		if (!file_exists(dir << name))
			return name;
	}
	return "";
}

void TsunamiWindow::on_save_as() {
	if (song->filename == "")
		hui::file_dialog_default = _suggest_filename(song, session->storage->current_directory);

	if (session->storage->ask_save(this)) {
		if (session->storage->save(song, hui::Filename))
			view->set_message(_("file saved"));
	}

	hui::file_dialog_default = "";
}

void TsunamiWindow::on_render_export_selection() {
	if (session->storage->ask_save_render_export(this)) {
		if (session->storage->render_export_selection(song, view->sel, hui::Filename))
			view->set_message(_("file exported"));
	}
}

Song *copy_song_from_selection(Song *song, const SongSelection &sel);

void TsunamiWindow::on_export_selection() {
	if (session->storage->ask_save(this)) {
		auto s = ownify(copy_song_from_selection(song, view->sel));
		if (session->storage->save(s.get(), hui::Filename))
			view->set_message(_("file exported"));
	}
}

void TsunamiWindow::on_quick_export() {
	string dir = hui::Config.get_str("QuickExportDir", hui::Application::directory.str());
	if (session->storage->save(song, dir + _suggest_filename(song, dir)))
		view->set_message(_("file saved"));
}

int pref_bar_index(AudioView *view) {
	if (view->cur_selection.type == HoverData::Type::BAR_GAP)
		return view->cur_selection.index;
	/*if (view->cur_selection.bar)
		return view->cur_selection.index + 1;*/
	if (view->sel.bar_indices(view->song).num > 0)
		return view->sel.bar_indices(view->song).back() + 1;
	if (view->hover_before_leave.pos > 0)
		return view->song->bars.num;
	return 0;
}

void TsunamiWindow::on_add_bars() {
	auto dlg = ownify(new BarAddDialog(win, song, pref_bar_index(view)));
	dlg->run();
}

void TsunamiWindow::on_add_pause() {
	auto dlg = ownify(new PauseAddDialog(win, song, pref_bar_index(view)));
	dlg->run();
}

void TsunamiWindow::on_delete_bars() {
	auto dlg = ownify(new BarDeleteDialog(win, song, view->sel.bar_indices(song)));
	dlg->run();
}

void TsunamiWindow::on_delete_time_interval() {
	hui::ErrorBox(this, "todo", "todo");
	/*song->action_manager->beginActionGroup();

	for (int i=view->sel.bars.end()-1; i>=view->sel.bars.start(); i--){
	song->deleteBar(i, view->bars_edit_data);
	}
	song->action_manager->endActionGroup();*/
}

void TsunamiWindow::on_insert_time_interval() {
	hui::ErrorBox(this, "todo", "todo");
	/*song->action_manager->beginActionGroup();

	for (int i=view->sel.bars.end()-1; i>=view->sel.bars.start(); i--){
	song->deleteBar(i, view->bars_edit_data);
	}
	song->action_manager->endActionGroup();*/
}

void TsunamiWindow::on_edit_bars() {
	if (view->sel._bars.num == 0) {
		return;
	}
	int num_bars = 0;
	int num_pauses = 0;
	for (int i: view->sel.bar_indices(song)) {
		if (song->bars[i]->is_pause())
			num_pauses++;
		else
			num_bars++;
	}
	if (num_bars > 0 and num_pauses == 0) {
		auto dlg = ownify(new BarEditDialog(win, song, view->sel.bar_indices(song)));
		dlg->run();
	} else if (num_bars == 0 and num_pauses == 1) {
		auto dlg = ownify(new PauseEditDialog(win, song, view->sel.bar_indices(song)[0]));
		dlg->run();
	} else {
		hui::ErrorBox(this, _("Error"), _("Can only edit bars or a single pause at a time."));
	}
}

void TsunamiWindow::on_scale_bars() {
	session->set_mode(EditMode::ScaleBars);
}

void TsunamiWindow::set_big_panel(ModulePanel* p) {
	big_module_panel = p;
	if (big_module_panel) {
		big_module_panel->set_func_close([=]{ msg_write("...close"); remove_control("plugin-grid"); });
		int w, h;
		get_size(w, h);
		big_module_panel->set_width(w / 2);
		set_target("root-grid");
		//add_paned("!expandx", 0, 0, "plugin-grid");
		add_grid("", 0, 0, "plugin-grid");
		embed(big_module_panel.get(), "plugin-grid", 0, 0);
	}
}
