/*
 * TsunamiWindow.h
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#ifndef SRC_VIEW_TSUNAMIWINDOW_H_
#define SRC_VIEW_TSUNAMIWINDOW_H_

#include "../lib/base/pointer.h"
#include "../lib/hui/hui.h"
#include "../lib/pattern/Observable.h"

class Song;
class MainView;
class AudioView;
class SideBar;
class BottomBar;
class HeaderBar;
class Session;
class Tsunami;
class Module;
class ModulePanel;
enum class SignalType;

class TsunamiWindow : public obs::Node<hui::Window> {
public:
	TsunamiWindow(Session *session);
	~TsunamiWindow() override;

	obs::sink in_update;

	void on_help();
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
	void on_paste_insert_time();
	void on_paste_aligned_to_beats();
	void on_delete();
	void on_delete_shift();
	void on_export();
	void on_export_selection();
	void on_quick_export();
	void on_undo();
	void on_redo();
	void on_add_new_track(SignalType type);
	void on_track_delete();
	void on_track_render();
	void on_track_edit();
	void on_track_edit_fx();
	void on_track_add_marker();
	void on_track_convert_mono();
	void on_track_convert_stereo();
	void on_track_group();
	void on_track_ungroup();
	void on_buffer_delete();
	void on_buffer_make_movable();
	void on_buffer_compress();
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
	void on_layer_add_selection_dominant();
	void on_sample_manager();
	void on_mixing_console();
	void on_plugin_console();
	void on_fx_console();
	void on_mastering_console();
	void on_sample_import();
	void on_song_properties();
	void on_track_properties();
	void on_sample_properties();
	void on_edit_marker();
	void on_marker_resize();
	void on_delete_marker();
	void on_settings();
	void on_mode_edit_checked();
	void on_play();
	void on_play_toggle();
	void on_play_loop();
	void on_pause();
	void on_stop();
	void on_record(bool complex);
	void on_add_bars();
	void on_add_pause();
	void on_delete_bars();
	void on_insert_time_interval();
	void on_delete_time_interval();
	void on_edit_bars_speed();
	void on_replace_bars();
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
	void on_menu_execute_audio_effect(const string &name);
	void on_menu_execute_audio_source(const string &name);
	void on_menu_execute_midi_effect(const string &name);
	void on_menu_execute_midi_source(const string &name);
	void on_menu_execute_tsunami_plugin(const string &name);
	void on_exit();

	base::future<bool> test_allow_termination();
	bool save();

	void update_menu();

	void on_side_bar_update();
	void on_bottom_bar_update();
	void on_update();

	AudioView* view;

	Song *song;

	Session *session;
	owned<MainView> main_view;

	owned<HeaderBar> header_bar;
	shared<SideBar> side_bar;
	shared<BottomBar> bottom_bar;
	
	shared<ModulePanel> big_module_panel;
	void set_big_panel(ModulePanel *p);

	Tsunami *app;
};

#endif /* SRC_VIEW_TSUNAMIWINDOW_H_ */
