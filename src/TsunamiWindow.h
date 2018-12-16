/*
 * TsunamiWindow.h
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#ifndef TSUNAMIWINDOW_H_
#define TSUNAMIWINDOW_H_

#include "lib/hui/hui.h"

class Song;
class AudioView;
class SideBar;
class BottomBar;
class MiniBar;
class Session;
class Tsunami;

class TsunamiWindow : public hui::Window
{
public:
	TsunamiWindow(Session *session);
	virtual ~TsunamiWindow();
	virtual void on_destroy();

	void on_about();
	void on_send_bug_report();

	void on_command(const string &id);

	void on_event();

	void on_new();
	void on_open();
	void on_save();
	void on_save_as();


	void on_copy();
	void on_paste();
	void on_paste_as_samples();
	void on_paste_time();
	void on_delete();
	void on_render_export_selection();
	void on_quick_export();
	void on_undo();
	void on_redo();
	void on_add_audio_track_mono();
	void on_add_audio_track_stereo();
	void on_add_time_track();
	void on_add_midi_track();
	void on_delete_track();
	void on_track_render();
	void on_track_edit_midi();
	void on_track_edit_fx();
	void on_track_add_marker();
	void on_track_convert_mono();
	void on_track_convert_stereo();
	void on_buffer_delete();
	void on_buffer_make_movable();
	void on_layer_midi_mode_linear();
	void on_layer_midi_mode_tab();
	void on_layer_midi_mode_classical();
	void on_sample_from_selection();
	void on_insert_sample();
	void on_remove_sample();
	void on_track_import();
	void on_add_layer();
	void on_delete_layer();
	void on_layer_make_track();
	void on_layer_merge();
	void on_layer_mark_selection_dominant();
	void on_sample_manager();
	void on_mixing_console();
	void on_fx_console();
	void on_sample_import();
	void on_song_properties();
	void on_track_properties();
	void on_sample_properties();
	void on_edit_marker();
	void on_delete_marker();
	void on_settings();
	void on_play();
	void on_play_loop();
	void on_pause();
	void on_stop();
	void on_record();
	void on_add_bars();
	void on_add_pause();
	void on_delete_bars();
	void on_insert_time_interval();
	void on_delete_time_interval();
	void on_edit_bars();
	void on_scale_bars();
	void on_view_midi_default();
	void on_view_midi_tab();
	void on_view_midi_score();
	void on_zoom_in();
	void on_zoom_out();
	void on_view_optimal();
	void on_select_none();
	void on_select_all();
	void on_select_expand();
	void on_show_log();
	void on_import_backup();
	void on_delete_backup();
	void on_menu_execute_audio_effect();
	void on_menu_execute_audio_source();
	void on_menu_execute_midi_effect();
	void on_menu_execute_midi_source();
	void on_menu_execute_song_plugin();
	void on_menu_execute_tsunami_plugin();
	void on_exit();

	//bool FileDialog(int kind, bool save, bool force_in_root_dir);
	bool allow_termination();
	bool save();

	void update_menu();

	void on_side_bar_update();
	void on_bottom_bar_update();
	void on_update();

	AudioView *view;

	Array<string> menu_layer_names;

	Song *song;

	Session *session;

	SideBar *side_bar;
	BottomBar *bottom_bar;
	MiniBar *mini_bar;

	bool auto_delete;

	Tsunami *app;
};

#endif /* TSUNAMIWINDOW_H_ */
