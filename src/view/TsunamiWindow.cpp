/*
 * Tsunami.cpp
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#include "TsunamiWindow.h"
#include "HeaderBar.h"
#include "mainview/MainView.h"
#include "audioview/AudioView.h"
#include "audioview/graph/AudioViewTrack.h"
#include "MouseDelayPlanner.h"
#include "dialog/NewSongDialog.h"
#include "dialog/HelpDialog.h"
#include "dialog/SettingsDialog.h"
#include "dialog/MarkerDialog.h"
#include "dialog/BarAddDialog.h"
#include "dialog/BarDeleteDialog.h"
#include "dialog/BarEditSpeedDialog.h"
#include "dialog/BarReplaceDialog.h"
#include "dialog/NewTrackDialog.h"
#include "dialog/PauseAddDialog.h"
#include "dialog/PauseEditDialog.h"
#include "dialog/TrackRoutingDialog.h"
#include "dialog/QuestionDialog.h"
#include "dialog/BufferCompressionDialog.h"
#include "helper/graph/SceneGraph.h"
#include "bottombar/BottomBar.h"
#include "sidebar/SideBar.h"
#include "sidebar/CaptureConsole.h"
#include "mode/ViewModeDefault.h"
#include "mode/ViewModeEdit.h"
#include "mode/ViewModeCapture.h"
#include "module/ModulePanel.h"
#include "module/ConfigPanel.h"
#include "module/ConfigurationDialog.h"
#include "../Session.h"
#include "../Playback.h"
#include "../EditModes.h"
#include "../Tsunami.h"
#include "../data/base.h"
#include "../data/Track.h"
#include "../data/TrackLayer.h"
#include "../data/audio/AudioBuffer.h"
#include "../action/ActionManager.h"
#include "../command/song/Export.h"
#include "../command/Unsorted.h"
#include "../module/audio/AudioEffect.h"
#include "../module/audio/AudioSource.h"
#include "../module/midi/MidiEffect.h"
#include "../module/midi/MidiSource.h"
#include "../module/SignalChain.h"
#include "../plugins/PluginManager.h"
#include "../plugins/TsunamiPlugin.h"
#include "../storage/Storage.h"
#include "../stuff/Log.h"
#include "../stuff/Clipboard.h"
#include "../stuff/BackupManager.h"
#include "../stuff/SessionManager.h"
#include "../lib/hui/hui.h"
#include "../lib/os/date.h"
#include "../lib/os/filesystem.h"

extern const string AppName;


TsunamiWindow::TsunamiWindow(Session *_session) :
		obs::Node<hui::Window>(AppName, 800, 600),
		in_update(this, [this] { on_update(); })
{
	session = _session;
	session->set_win(this);
	song = session->song.get();
	app = tsunami;

	int width = hui::config.get_int("Window.Width", 800);
	int height = hui::config.get_int("Window.Height", 600);
	bool maximized = hui::config.get_bool("Window.Maximized", true);

	set_size(width, height);
	set_maximized(maximized);

	event("new", [this] { on_new(); });
	set_key_code("new", hui::KEY_N + hui::KEY_CONTROL);
	event("open", [this] { on_open(); });
	set_key_code("open", hui::KEY_O + hui::KEY_CONTROL);
	event("save", [this] { on_save(); });
	set_key_code("save", hui::KEY_S + hui::KEY_CONTROL);
	event("save_as", [this] { on_save_as(); });
	set_key_code("save_as", hui::KEY_S + hui::KEY_CONTROL + hui::KEY_SHIFT);
	event("copy", [this] { on_copy(); });
	set_key_code("copy", hui::KEY_C + hui::KEY_CONTROL);
	event("paste", [this] { on_paste(); });
	set_key_code("paste", hui::KEY_V + hui::KEY_CONTROL);
	event("paste-as-samples", [this] { on_paste_as_samples(); });
	event("paste-insert-time", [this] { on_paste_insert_time(); });
	set_key_code("paste-insert-time", hui::KEY_V + hui::KEY_CONTROL + hui::KEY_SHIFT);
	event("paste-aligned-to-beats", [this] { on_paste_aligned_to_beats(); });
	set_key_code("paste-aligned-to-beats", hui::KEY_B + hui::KEY_CONTROL);
	event("delete", [this] { on_delete(); });
	set_key_code("delete", hui::KEY_DELETE);
	event("delete-shift", [this] { on_delete_shift(); });
	set_key_code("delete-shift", hui::KEY_SHIFT + hui::KEY_DELETE);
	event("export", [this] { on_export(); });
	set_key_code("export", hui::KEY_CONTROL + hui::KEY_E);
	event("export-selection", [this] { on_export_selection(); });
	set_key_code("export-selection", hui::KEY_E + hui::KEY_CONTROL + hui::KEY_SHIFT);
	event("quick-export", [this] { on_quick_export(); });
	set_key_code("quick-export", hui::KEY_X + hui::KEY_CONTROL + hui::KEY_SHIFT);
	event("undo", [this] { on_undo(); });
	set_key_code("undo", hui::KEY_Z + hui::KEY_CONTROL);
	event("redo", [this] { on_redo(); });
	set_key_code("redo", hui::KEY_Y + hui::KEY_CONTROL);
	event("track_render", [this] { on_track_render(); });
	event("track-add-new", [this] { on_add_new_track(SignalType::AUDIO); });
	set_key_code("track-add-new", hui::KEY_N + hui::KEY_CONTROL + hui::KEY_SHIFT);
	event("track-add-beats", [this] { on_add_new_track(SignalType::BEATS); });
	event("track-delete", [this] { on_track_delete(); });
	event("track-create-group", [this] { on_track_group(); });
	event("track-ungroup", [this] { on_track_ungroup(); });
	event("mode-edit-check", [this] { on_mode_edit_checked(); });
	event("layer-edit", [this] { on_track_edit(); });
	set_key_code("layer-edit", hui::KEY_ALT + hui::KEY_E);
	event("edit_curves", [this] { session->set_mode(EditMode::Curves); });
	event("track-edit-curves", [this] { session->set_mode(EditMode::Curves); });
	event("track-edit-fx", [this] { on_track_edit_fx(); });
	event("track-add-marker", [this] { on_track_add_marker(); });
	set_key_code("track-add-marker", hui::KEY_CONTROL + hui::KEY_J);
	add_action_checkable("track-convert-mono");
	event("track-convert-mono", [this] { on_track_convert_mono(); });
	add_action_checkable("track-convert-stereo");
	event("track-convert-stereo", [this] { on_track_convert_stereo(); });
	event("buffer-delete", [this] { on_buffer_delete(); });
	event("buffer-make-movable", [this] { on_buffer_make_movable(); });
	event("buffer-compress", [this] { on_buffer_compress(); });

	event("edit-track-groups", [this] {
		hui::fly(new TrackRoutingDialog(this, song));
	});


	add_action_checkable("track-midi-mode-linear");
	event("track-midi-mode-linear", [this] { on_layer_midi_mode_linear(); });
	add_action_checkable("track-midi-mode-tab");
	event("track-midi-mode-tab", [this] { on_layer_midi_mode_tab(); });
	add_action_checkable("track-midi-mode-classical");
	event("track-midi-mode-classical", [this] { on_layer_midi_mode_classical(); });
	add_action_checkable("track-audio-mode-peaks");
	event("track-audio-mode-peaks", [this] { view->cur_vtrack()->set_audio_mode(AudioViewMode::PEAKS); });
	add_action_checkable("track-audio-mode-spectrum");
	event("track-audio-mode-spectrum", [this] { view->cur_vtrack()->set_audio_mode(AudioViewMode::SPECTRUM); });
	
	add_action_checkable("track-muted");
	//event("track-muted", [this] { view->cur_track()->set_muted(!view->cur_track()->muted); });
	set_key_code("track-muted", hui::KEY_ALT + hui::KEY_M);
	add_action_checkable("track-solo");
	//event("track-solo", [this] { view->cur_vtrack()->set_solo(!view->cur_vtrack()->solo); });
	set_key_code("track-solo", hui::KEY_ALT + hui::KEY_S);
	add_action_checkable("layer-muted");
	set_key_code("layer-muted", hui::KEY_ALT + hui::KEY_SHIFT + hui::KEY_M);
	add_action_checkable("layer-solo");
	set_key_code("layer-solo", hui::KEY_ALT + hui::KEY_SHIFT + hui::KEY_S);
	add_action_checkable("track-explode");
	set_key_code("track-explode", hui::KEY_ALT + hui::KEY_X);
	set_key_code("layer-up", hui::KEY_UP);
	set_key_code("layer-down", hui::KEY_DOWN);
	set_key_code("layer-expand-up", hui::KEY_SHIFT + hui::KEY_UP);
	set_key_code("layer-expand-down", hui::KEY_SHIFT + hui::KEY_DOWN);

	event("layer-add", [this] { on_add_layer(); });
	event("layer-delete", [this] { on_delete_layer(); });
	set_key_code("layer-delete", hui::KEY_CONTROL + hui::KEY_DELETE);
	event("layer-make-track", [this] { on_layer_make_track(); });
	event("layer-merge", [this] { on_layer_merge(); });
	event("layer-mark-dominant", [this] { on_layer_mark_selection_dominant(); });
	set_key_code("layer-mark-dominant", hui::KEY_ALT + hui::KEY_D);
	event("layer-add-dominant", [this] { on_layer_add_selection_dominant(); });
	event("bars-add", [this] { on_add_bars(); });
	event("bars-add-pause", [this] { on_add_pause(); });
	event("bars-delete", [this] { on_delete_bars(); });
	event("delete_time", [this] { on_delete_time_interval(); });
	event("insert_time", [this] { on_insert_time_interval(); });
	event("bars-edit-speed", [this] { on_edit_bars_speed(); });
	event("bars-replace", [this] { on_replace_bars(); });
	event("bars-scale", [this] { on_scale_bars(); });
	event("sample_manager", [this] { on_sample_manager(); });
	event("song-edit-samples", [this] { on_sample_manager(); });
	add_action_checkable("show-mixing-console");
	event("show-mixing-console", [this] { on_mixing_console(); });
	set_key_code("show-mixing-console", hui::KEY_CONTROL + hui::KEY_M);
	add_action_checkable("show-plugin-console");
	event("show-plugin-console", [this] { on_plugin_console(); });
	set_key_code("show-plugin-console", hui::KEY_CONTROL + hui::KEY_P);
	event("show-devices", [this] { on_settings(); });
	add_action_checkable("show-signal-chain");
	event("show-signal-chain", [this] { session->set_mode(EditMode::XSignalEditor); });
	event("show-mastering-console", [this] { on_mastering_console(); });
	event("show-fx-console", [this] { on_fx_console(); });
	event("show-session-console", [this] { bottom_bar->open(BottomBar::SESSION_CONSOLE); });
	event("sample_from_selection", [this] { on_sample_from_selection(); });
	event("sample-insert", [this] { on_insert_sample(); });
	set_key_code("sample-insert", hui::KEY_I + hui::KEY_CONTROL);
	event("sample-delete", [this] { on_remove_sample(); });
	event("marker-delete", [this] { on_delete_marker(); });
	event("marker-edit", [this] { on_edit_marker(); });
	event("marker-resize", [this] { on_marker_resize(); });
	event("track_import", [this] { on_track_import(); });
	event("sub_import", [this] { on_sample_import(); });
	event("song-properties", [this] { on_song_properties(); });
	set_key_code("song-properties", hui::KEY_F4);
	event("track-properties", [this] { on_track_properties(); });
	event("sample-properties", [this] { on_sample_properties(); });
	event("settings", [this] { on_settings(); });
	event("play", [this] { on_play(); });
	event("play-toggle", [this] { on_play_toggle(); });
	set_key_code("play-toggle", hui::KEY_SPACE);
	add_action_checkable("play-loop");
	event("play-loop", [this] { on_play_loop(); });
	set_key_code("play-loop", hui::KEY_CONTROL + hui::KEY_L);
	event("pause", [this] { on_pause(); });
	event("stop", [this] { on_stop(); });
	set_key_code("stop", hui::KEY_CONTROL + hui::KEY_T);
	event("record", [this] { on_record(false); });
	set_key_code("record", hui::KEY_CONTROL + hui::KEY_R);
	event("record-simple", [this] { on_record(false); });
	event("record-complex", [this] { on_record(true); });
	event("playback-range-lock", [this] { view->set_playback_range_locked(!view->playback_range_locked); });
	event("show-log", [this] { on_show_log(); });
	event("about", [this] { on_about(); });
	event("help", [this] { on_help(); });
	set_key_code("run_plugin", hui::KEY_SHIFT + hui::KEY_RETURN);
	event("exit", [this] { on_exit(); });
	set_key_code("exit", hui::KEY_CONTROL + hui::KEY_Q);
	event("select_all", [this] { on_select_all(); });
	set_key_code("select_all", hui::KEY_CONTROL + hui::KEY_A);
	event("select_nothing", [this] { on_select_none(); });
	event("select_expand", [this] { on_select_expand(); });
	set_key_code("select_expand", hui::KEY_TAB + hui::KEY_SHIFT);
	add_action_checkable("view-midi-linear");
	event("view-midi-linear", [this] { on_view_midi_default(); });
	add_action_checkable("view-midi-tab");
	event("view-midi-tab", [this] { on_view_midi_tab(); });
	add_action_checkable("view-midi-classical");
	event("view-midi-classical", [this] { on_view_midi_score(); });
	event("view-optimal", [this] { on_view_optimal(); });
	event("zoom-in", [this] { on_zoom_in(); });
	set_key_code("zoom-in", hui::KEY_PLUS);
	event("zoom-out", [this] { on_zoom_out(); });
	set_key_code("zoom-out", hui::KEY_MINUS);

	set_key_code("vertical-zoom-in", hui::KEY_PLUS + hui::KEY_SHIFT);
	set_key_code("vertical-zoom-out", hui::KEY_MINUS + hui::KEY_SHIFT);

	set_key_code("cam-move-right", hui::KEY_PAGE_DOWN);
	set_key_code("cam-move-left", hui::KEY_PAGE_UP);
	set_key_code("cursor-jump-start", hui::KEY_HOME);
	set_key_code("cursor-jump-end", hui::KEY_END);
	set_key_code("cursor-expand-start", hui::KEY_HOME + hui::KEY_SHIFT);
	set_key_code("cursor-expand-end", hui::KEY_END + hui::KEY_SHIFT);
	set_key_code("cursor-move-left", hui::KEY_LEFT);
	set_key_code("cursor-move-right", hui::KEY_RIGHT);
	set_key_code("cursor-expand-left", hui::KEY_LEFT + hui::KEY_SHIFT);
	set_key_code("cursor-expand-right", hui::KEY_RIGHT + hui::KEY_SHIFT);


	set_border_width(0);
	set_spacing(0);

	if (hui::config.get_bool("Window.HeaderBar", true)) {
		header_bar = new HeaderBar(this);
	} else {
		add_basic_layout("menubar|toolbar-top");
		set_menu(hui::create_resource_menu("menu", this));
		app->plugin_manager->add_plugins_to_menu(this);
		get_toolbar(0)->set_by_id("toolbar");
	}

	// table structure
	set_target("");
	add_grid("", 0, 0, "root-grid");
	set_target("root-grid");
	add_grid("", 1, 0, "main-grid");

	// main table
	set_target("main-grid");
	add_drawing_area("!mainwindowcontrol,gesture=zoom,gesture=drag", 0, 0, "area");
	if (hui::config.get_bool("View.EventCompression", true) == false)
		set_options("area", "noeventcompression");

	// events
	event("hui:close", [this] { on_exit(); });

	main_view = new MainView(session, "area");

	view = new AudioView(session);
	main_view->_add_view(view);
	session->main_view = main_view.get();
	session->view = view;
	hui::run_later(0.01f, [this] {
		activate(main_view->id);
	});

	// side bar
	side_bar = new SideBar(session, this);
	embed(side_bar.get(), "root-grid", 2, 0);


	side_bar->out_changed >> in_update;

	// bottom bar
	bottom_bar = new BottomBar(session, this);
	embed(bottom_bar.get(), "main-grid", 0, 1);

	view->out_settings_changed >> in_update;
	view->out_selection_changed >> in_update;
	view->out_cur_layer_changed >> in_update;
	view->out_cur_sample_changed >> in_update;
	session->playback->signal_chain->out_changed >> create_sink([this] {
		on_update();
	});
	session->playback->signal_chain->out_state_changed >> create_sink([this] {
		on_update();
	});
	song->action_manager->out_changed >> in_update;
	song->action_manager->out_undo_action >> create_sink([this] {
		session->status(_("undo: ") + hui::get_language_s(song->action_manager->get_current_action()));
	});
	song->action_manager->out_redo_action >> create_sink([this] {
		session->status(_("redo: ") + hui::get_language_s(song->action_manager->get_current_action()));
	});
	song->out_after_change >> in_update;
	app->clipboard->out_changed >> in_update;
	bottom_bar->out_changed >> create_sink([this] {
		on_bottom_bar_update();
	});
	side_bar->out_changed >> create_sink([this] {
		on_side_bar_update();
	});
	session->out_mode_changed >> in_update;
	
	event("*", [this] {
		view->on_command(hui::get_event()->id);
	});

	// first time start?
	if (hui::config.get_bool("FirstStart", true)) {
		hui::run_later(0.2f, [this] {
			on_help();
			hui::config.set_bool("FirstStart", false);
		});
	}

	hui::run_later(0.5f, [this] {
		if (BackupManager::should_notify_found_backups()) {
			session->q("Old recording backup found\nto load or delete, open the session manager in the bottom bar", {"show-session-console:yes"});
			BackupManager::notify_found_backups_done();
		}
	});

	update_menu();
}

TsunamiWindow::~TsunamiWindow() {
	session->prepare_end();

	int w, h;
	get_size_desired(w, h);
	hui::config.set_int("Window.Width", w);
	hui::config.set_int("Window.Height", h);
	hui::config.set_bool("Window.Maximized", is_maximized());

	session->playback->signal_chain->stop_hard();
	view->unsubscribe(side_bar.get());
	view->unsubscribe(this);
	session->playback->signal_chain->unsubscribe(this);
	song->action_manager->unsubscribe(this);
	app->clipboard->unsubscribe(this);
	bottom_bar->unsubscribe(this);
	side_bar->unsubscribe(this);

	unembed(side_bar.get());
	side_bar = nullptr;
	unembed(bottom_bar.get());
	bottom_bar = nullptr;
	view = nullptr;

	auto _session = session;
	hui::run_later(0.010f, [_session] {
		tsunami->session_manager->end_session(_session);
	});
}

void TsunamiWindow::on_about() {
	hui::about_box(this);
}

void TsunamiWindow::on_help() {
	hui::fly(new HelpDialog(this));
}

void TsunamiWindow::on_add_new_track(SignalType type) {
	hui::fly(new NewTrackDialog(this, session, type));
}

void TsunamiWindow::on_track_render() {
	Range range = view->sel.range();
	if (range.is_empty()) {
		session->e(_("Selection range is empty"));
		return;
	}

	if (view->get_playable_layers() == view->sel.layers()) {
		song_render_track(song, range, view->sel.layers(), this);
	} else {
		QuestionDialogMultipleChoice::ask(this, _("Question"), _("Which tracks and layers should be rendered?"),
				{_("All non-muted"), _("From selection")},
				{_("respecting solo and mute, ignoring selection"), _("respecting selection, ignoring solo and mute")}, true).then([this, range] (int answer) {
					if (answer == 1)
						song_render_track(song, range, view->sel.layers(), this);
					else
						song_render_track(song, range, view->get_playable_layers(), this);
				});
	}

}

void TsunamiWindow::on_track_delete() {
	auto tracks = view->sel.tracks();
	if (tracks.num > 0) {
		song_delete_tracks(song, tracks);
	} else {
		session->e(_("No track selected"));
	}
}

void TsunamiWindow::on_track_group() {
	auto tracks = selected_tracks_sorted(view);
	song_group_tracks(song, tracks);
}

void TsunamiWindow::on_track_ungroup() {
	auto tracks = selected_tracks_sorted(view);
	song_ungroup_tracks(song, tracks);
}

void TsunamiWindow::on_track_edit() {
	session->set_mode(EditMode::EditTrack);
}

void TsunamiWindow::on_track_edit_fx() {
	session->set_mode(EditMode::DefaultTrackFx);
}

void TsunamiWindow::on_track_add_marker() {
	if (view->cur_track()) {
		Range range = view->sel.range();
		hui::fly(new MarkerDialog(this, view->cur_layer(), range, ""));
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
	song_make_buffers_movable(song, view->sel);
}

void TsunamiWindow::on_buffer_compress() {
	auto dlg = new BufferCompressionDialog(this);
	hui::fly(dlg).then([dlg, this] {
		if (dlg->codec != "")
			song_compress_buffers(song, view->sel, dlg->codec);
	});
}

void TsunamiWindow::on_mode_edit_checked() {
	if (view->mode == view->mode_edit)
		session->set_mode(EditMode::Default);
	else
		session->set_mode(EditMode::EditTrack);
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
		hui::fly(new MarkerDialog(this, view->cur_layer(), view->cur_selection.marker));
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
	if (filename)
		return filename.basename();
	return _("No name");
}

base::future<bool> TsunamiWindow::test_allow_termination() {
	base::promise<bool> promise;
	side_bar->test_allow_close().then([this, promise] (bool allow) mutable {
		if (!allow) {
			promise(false);
			return;
		}
		if (song->action_manager->is_save()) {
			promise(true);
			return;
		}

		hui::question_box(this, _("Question"), format(_("'%s'\nSave file?"), title_filename(song->filename)), true).then([this, promise] (bool answer) mutable {
			if (answer) {
				on_save();
				promise(song->action_manager->is_save());
			} else {
				promise(true);
			}
		}).on_fail([promise] () mutable {
			// cancel
			promise(false);
		});
	});
	return promise.get_future();
}

void TsunamiWindow::on_copy() {
	app->clipboard->copy(view);
	session->status(_("copied"));
}

void TsunamiWindow::on_paste() {
	app->clipboard->paste(view);
	session->status(_("pasted"));
}

void TsunamiWindow::on_paste_as_samples() {
	app->clipboard->paste_as_samples(view);
	session->status(_("pasted (sample)"));
}

void TsunamiWindow::on_paste_insert_time() {
	app->clipboard->paste_insert_time(view);
	session->status(_("pasted (insert time)  EXPERIMENTAL!"));
}

void TsunamiWindow::on_paste_aligned_to_beats() {
	app->clipboard->paste_aligned_to_beats(view);
	session->status(_("pasted (aligned)  EXPERIMENTAL!"));
}

void TsunamiWindow::on_menu_execute_audio_effect(const string &name) {
	auto fx = CreateAudioEffect(session, name);

	configure_module(this, fx).then([this, fx] {
		int n_layers = song_apply_audio_effect(song, fx, view->sel, this);
		if (n_layers == 0)
			session->e(_("no audio tracks selected"));
	});
}

void TsunamiWindow::on_menu_execute_audio_source(const string &name) {
	auto s = CreateAudioSource(session, name);

	configure_module(this, s).then([s, this] {
		int n_layers = song_apply_audio_source(song, s, view->sel, this);
		if (n_layers == 0)
			session->e(_("no audio tracks selected"));
	});
}

void TsunamiWindow::on_menu_execute_midi_effect(const string &name) {
	auto fx = CreateMidiEffect(session, name);

	configure_module(this, fx).then([fx, this] {
		int n_layers = song_apply_midi_effect(song, fx, view->sel, this);
		if (n_layers == 0)
			session->e(_("no midi tracks selected"));
	});
}

void TsunamiWindow::on_menu_execute_midi_source(const string &name) {
	auto s = CreateMidiSource(session, name);

	configure_module(this, s).then([s, this] {
		int n_layers = song_apply_midi_source(song, s, view->sel, this);
		if (n_layers == 0)
			session->e(_("no midi tracks selected"));
	});
}

void TsunamiWindow::on_menu_execute_tsunami_plugin(const string &name) {
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
	song_delete_shift(song, view->sel);
	view->set_cursor_pos(view->sel.range().start());
}

void TsunamiWindow::on_sample_manager() {
	session->set_mode(EditMode::DefaultSamples);
}

void TsunamiWindow::on_mixing_console() {
	bottom_bar->toggle(BottomBar::MIXING_CONSOLE);
}

void TsunamiWindow::on_plugin_console() {
	bottom_bar->toggle(BottomBar::PLUGIN_CONSOLE);
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
	hui::fly(new SettingsDialog(view, this));
}

void TsunamiWindow::on_track_import() {
	session->storage->ask_open_import(this).then([this] (const Path &filename) {
		Track *t = song->add_track(SignalType::AUDIO_STEREO);
		session->storage->load_track(t->layers[0].get(), filename, view->cursor_pos());
	});
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


extern bool _capture_console_force_complex_;

void TsunamiWindow::on_record(bool complex) {
	_capture_console_force_complex_ = complex;
	session->set_mode(EditMode::Capture);
}

void TsunamiWindow::on_add_layer() {
	view->cur_track()->add_layer();
}

void TsunamiWindow::on_delete_layer() {
	song->begin_action_group(_("delete layers"));
	try {
		auto layers = view->sel.layers();
		for (auto l: layers) {
			auto t = l->track;
			if (t->layers.num > 1) {
				t->delete_layer(const_cast<TrackLayer*>(l));
			} else {
				song->delete_track(t);
			}
		}
	} catch(Exception &e) {
		session->e(e.message());
	}
	song->end_action_group();
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
	enable("paste-as-samples", app->clipboard->has_data());
	enable("paste-aligned-to-beats", app->clipboard->has_data());
	enable("paste-insert-time", app->clipboard->has_data());
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
	check("show_signal_chain", bottom_bar->is_active(BottomBar::SIGNAL_CHAIN_CONSOLE));
	check("show-plugin-console", bottom_bar->is_active(BottomBar::PLUGIN_CONSOLE));
	check("sample_manager", session->in_mode(EditMode::DefaultSamples));

	string title = title_filename(song->filename) + " - " + AppName;
	if (!song->action_manager->is_save())
		title = "*" + title;
	if (tsunami->session_manager->is_persistent(session) and !header_bar)
		title += " (session)";
	set_title(title);

	if (header_bar)
		header_bar->update();
}

void TsunamiWindow::on_side_bar_update() {
	if (!side_bar->visible)
		activate(main_view->id);
	update_menu();
}

void TsunamiWindow::on_bottom_bar_update() {
	if (!bottom_bar->visible)
		activate(main_view->id);
	main_view->update_onscreen_displays();
	update_menu();
}

void TsunamiWindow::on_update() {
	// "Clipboard", "AudioFile" or "AudioView"
	update_menu();
	view->force_redraw();
}

void TsunamiWindow::on_exit() {
	test_allow_termination().then([this] (bool allow) {
		if (allow) {
			BackupManager::set_save_state(session);
			//request_destroy();
			hui::run_later(0.01f, [this] { session->win = nullptr; });
		}
	});
}

void TsunamiWindow::on_new() {
	hui::fly(new NewSongDialog(this));
}

void TsunamiWindow::on_open() {
	session->storage->ask_open(this).then([this] (const Path &filename) {
		auto *s = tsunami->session_manager->get_empty_session(session);
		if (s->session_manager->try_restore_session_for_song(session, filename)) {
			BackupManager::set_save_state(s);
		} else {
			s->storage->load(s->song.get(), filename).then([s] {
				BackupManager::set_save_state(s);
			});
		}
		/*} else {
			auto *s = tsunami->session_manager->spawn_new_session();
			s->win->show();
			if (s->storage->load(s->song.get(), filename))
				s->session_manager->try_restore_matching_session(s);
		}*/
	});
}

void TsunamiWindow::on_save() {
	if (song->filename == "") {
		on_save_as();
	} else {
		session->storage->save(song, song->filename).then([this] {
			session->status(_("file saved"));
			BackupManager::set_save_state(session);
		});
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
	if (s->filename)
		return s->filename.basename();
	string base = Date::now().format("%Y-%m-%d");

	string ext = "nami";
	if (song_is_simple_audio(s))
		ext = "ogg";
	//else if (song_is_simple_midi(s))
	//	ext = "midi";

	for (int i=0; i<26; i++) {
		string name = base + "a." + ext;
		name[name.num - ext.num - 2] += i;
		if (!os::fs::exists(dir | name))
			return name;
	}
	return "";
}

void TsunamiWindow::on_save_as() {
	string def;
	if (song->filename == "")
		def = _suggest_filename(song, session->storage->current_directory);

	session->storage->ask_save(this, {"default=" + def}).then([this] (const Path &filename) {
		session->storage->save(song, filename).then([this] {
			session->status(_("file saved"));
		});
	});
}

void TsunamiWindow::on_export() {
	string def;
	if (song->filename == "")
		def = _suggest_filename(song, session->storage->current_directory);

	session->storage->ask_save(this, {"default=" + def}).then([this] (const Path &filename) {
		session->storage->_export(song, filename).then([this] {
			session->status(_("file exported"));
		});
	});
}

void TsunamiWindow::on_export_selection() {
	session->storage->ask_save(this).then([this] (const Path &filename) {
		if (view->get_playable_layers() == view->sel.layers()) {
			export_selection(song, view->sel, filename).then([this] {
				session->status(_("file exported"));
			});
		} else {
			QuestionDialogMultipleChoice::ask(this, _("Question"), _("Which tracks and layers should be exported?"),
					{_("All non-muted"), _("All selected"), _("Only non-muted selected")},
					{_("respecting solo and mute, ignoring selection"), _("respecting selection, ignoring solo and mute"), _("only include if selected AND not muted")}, true).then([this, filename] (int answer) {
						auto sel = view->sel;
						bool force_unmute = false;
						if (answer == 0) {
							// all non-muted
							sel = SongSelection::from_range(song, view->sel.range()).filter(view->get_playable_layers());
							force_unmute = false;
						} else if (answer == 1) {
							// selection
							sel = view->sel;
							force_unmute = true;
						} else if (answer == 2) {
							// non-muted & selected
							sel = view->sel.filter(view->get_playable_layers());
							force_unmute = false;
						} else { // answer < 0
							return;
						}
						//for (auto l: sel.layers())
						//	msg_write(l->track->nice_name() + "  v" + str(l->version_number()));
						export_selection(song, sel, filename, force_unmute).then([this] {
							session->status(_("file exported"));
						});
					});
		}
	});
}

void TsunamiWindow::on_quick_export() {
	auto dir = Path(hui::config.get_str("QuickExportDir", hui::Application::directory.str()));
	session->storage->save(song, dir | _suggest_filename(song, dir)).then([this] {
		session->status(_("file saved"));
	});
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
	hui::fly(new BarAddDialog(win, song, pref_bar_index(view)));
}

void TsunamiWindow::on_add_pause() {
	hui::fly(new PauseAddDialog(win, song, pref_bar_index(view)));
}

void TsunamiWindow::on_delete_bars() {
	hui::fly(new BarDeleteDialog(win, song, view->sel.bar_indices(song)));
}

void TsunamiWindow::on_delete_time_interval() {
	hui::error_box(this, "todo", "todo");
	/*song->action_manager->beginActionGroup();

	for (int i=view->sel.bars.end()-1; i>=view->sel.bars.start(); i--){
	song->deleteBar(i, view->bars_edit_data);
	}
	song->action_manager->endActionGroup();*/
}

void TsunamiWindow::on_insert_time_interval() {
	hui::error_box(this, "todo", "todo");
	/*song->action_manager->beginActionGroup();

	for (int i=view->sel.bars.end()-1; i>=view->sel.bars.start(); i--){
	song->deleteBar(i, view->bars_edit_data);
	}
	song->action_manager->endActionGroup();*/
}

void TsunamiWindow::on_edit_bars_speed() {
	if (view->sel._bars.num == 0)
		return;
	int num_bars = 0;
	int num_pauses = 0;
	for (int i: view->sel.bar_indices(song)) {
		if (song->bars[i]->is_pause())
			num_pauses++;
		else
			num_bars++;
	}
	if (num_bars > 0 and num_pauses == 0) {
		hui::fly(new BarEditSpeedDialog(win, song, view->sel.bar_indices(song)));
	} else if (num_bars == 0 and num_pauses == 1) {
		hui::fly(new PauseEditDialog(win, song, view->sel.bar_indices(song)[0]));
	} else {
		session->e(_("Can only edit bars or a single pause at a time."));
	}
}

void TsunamiWindow::on_replace_bars() {
	if (view->sel._bars.num == 0)
		return;
	int num_bars = 0;
	int num_pauses = 0;
	for (int i: view->sel.bar_indices(song)) {
		if (song->bars[i]->is_pause())
			num_pauses++;
		else
			num_bars++;
	}
	if (num_bars > 0 and num_pauses == 0) {
		hui::fly(new BarReplaceDialog(win, song, view->sel.bar_indices(song)));
	} else if (num_bars == 0 and num_pauses == 1) {
		hui::fly(new PauseEditDialog(win, song, view->sel.bar_indices(song)[0]));
	} else {
		session->e(_("Can only edit bars or a single pause at a time."));
	}
}

void TsunamiWindow::on_scale_bars() {
	session->set_mode(EditMode::ScaleBars);
}

void TsunamiWindow::set_big_panel(ModulePanel* p) {
	big_module_panel = p;
	if (big_module_panel) {
		big_module_panel->set_func_close([this] {
			msg_write("...close");
			remove_control("plugin-grid");
		});
		int w, h;
		get_size(w, h);
		big_module_panel->set_width(w / 2);
		set_target("root-grid");
		//add_paned("!expandx", 0, 0, "plugin-grid");
		add_grid("", 0, 0, "plugin-grid");
		embed(big_module_panel.get(), "plugin-grid", 0, 0);
	}
}
