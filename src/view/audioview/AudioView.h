/*
 * AudioView.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef AUDIOVIEW_H_
#define AUDIOVIEW_H_

#include "../../lib/base/pointer.h"
#include "../../lib/image/image.h"
#include "../../data/SongSelection.h"
#include "../../data/midi/Scale.h"
#include "TrackHeightManager.h"
#include "ViewPort.h"
#include "../HoverData.h"
#include "../mainview/MainViewNode.h"

namespace hui{
	class Menu;
}

namespace scenegraph {
	class SceneGraph;
	class Node;
	class MouseDelayPlanner;
	class MouseDelayAction;
}

namespace tsunami {

class Song;
class Track;
class TrackLayer;
class Sample;
class SampleRef;
class AudioBuffer;
class DeviceManager;
class AudioOutput;
class SongRenderer;
class PeakMeter;
class SignalChain;
class TsunamiWindow;
class AudioViewTrack;
class AudioViewLayer;
class ViewMode;
class ViewModeDefault;
class ViewModeEdit;
class ViewModeEditDummy;
class ViewModeEditAudio;
class ViewModeEditBars;
class ViewModeEditMidi;
class ViewModeCurve;
class ViewModeCapture;
class ViewModeScaleMarker;
class ScrollBar;
class Session;
class BufferPainter;
class GridPainter;
class TimeScale;
class Cursor;
class AddTrackButton;
class SelectionMarker;
class CpuDisplay;
class PeakMeterDisplay;
class Dial;
class PeakDatabase;
enum class MidiMode;


enum class SelectionSnapMode {
	None,
	Bar,
	Part,
};

enum class SelectionMode {
	None,
	Time,
	Rect,
	TrackRect,
	Fake,
};

class AudioView : public MainViewNode {
public:
	explicit AudioView(Session *session);
	~AudioView() override;

	void* mvn_data() const override;
	string mvn_description() const override;
	void mvn_on_enter() override;

	void play() override;
	void stop() override;
	void pause() override;
	bool is_playback_active() override;
	bool is_paused() override;
	bool mvn_can_play() override;
	bool mvn_can_stop() override;
	bool mvn_can_pause() override;
	bool mvn_can_record() override;
	bool mvn_can_edit() override;
	bool is_editing() override;
	bool is_recording() override;
	bool mvn_show_undo() override;

	void check_consistency();
	void force_redraw();
	void force_redraw_part(const rect &r);

	void on_draw(Painter *p) override;
	bool on_mouse_move(const vec2 &m) override;
	bool on_left_button_down(const vec2 &m) override;
	bool on_left_button_up(const vec2 &m) override;
	bool on_right_button_up(const vec2 &m) override;
	bool on_left_double_click(const vec2 &m) override;
	void on_mouse_leave();
	bool on_mouse_wheel(const vec2 &d) override;
	bool on_key_down(int k) override;
	bool on_key_up(int k) override;
	bool on_gesture(const string &id, const vec2 &m, const vec2 &param) override;
	void on_command(const string &id);

	void on_song_change();
	void on_song_new();
	void on_song_finished_loading();
	void on_song_tracks_change();
	void on_stream_tick();
	void on_stream_state_change();
	void on_update_sidebar();

	obs::source out_cur_track_changed{this, "cur-track-changed"};
	obs::source out_cur_sample_changed{this, "cur-sample-changed"};
	obs::source out_cur_layer_changed{this, "cur-layer-changed"};
	obs::source out_selection_changed{this, "selection-changed"};
	obs::source out_settings_changed{this, "settings-changed"};
	obs::source out_view_changed{this, "view-changed"};
	obs::source out_vtrack_changed{this, "vtrack-changed"};
	obs::source out_solo_changed{this, "solo-changed"};
	obs::sink in_solo_changed;
	obs::sink in_redraw;

	void zoom_in();
	void zoom_out();

	void draw_time_line(Painter *c, int pos, const color &col, bool hover, bool show_time = false);
	void prepare_draw_song();
	int draw_runner_id;

	void draw_cursor_hover(Painter *c, const string &msg);

	void request_optimize_view();
	void perform_optimize_view();
	bool _optimize_view_requested;
	void update_menu();

	static const int SNAPPING_DIST;

	HoverData &hover();
	HoverData hover_before_leave;
	SongSelection sel;
	SongSelection sel_temp;

	HoverData hover_time(const vec2 &m);
	int get_mouse_pos(const vec2 &m);
	int get_mouse_pos_snap(const vec2 &m);

	SelectionMode selection_mode;
	SelectionSnapMode selection_snap_mode;
	void set_selection_snap_mode(SelectionSnapMode mode);
	bool hide_selection;


	scenegraph::MouseDelayPlanner *mdp();
	void mdp_prepare(scenegraph::MouseDelayAction *action, const vec2 &m);
	void mdp_run(scenegraph::MouseDelayAction *action, const vec2 &m);
	void mdp_prepare(std::function<void(const vec2&)> update, const vec2 &m);


	void snap_to_grid(int &pos);

	void unselect_all_samples();

	bool enabled;
	void enable(bool enabled);

	float scroll_speed;
	float scroll_speed_fast;
	float zoom_speed;

	bool selecting_or() const;
	bool selecting_xor() const;

	void select_none();
	void select_all();
	void select_expand();
	void update_selection();
	void set_selection(const SongSelection &s);
	Range get_playback_selection(bool for_recording);

	int mouse_over_sample(SampleRef *s, const vec2& m);
	void selection_update_pos(HoverData &s, const vec2& m);
	bool mouse_over_time(int pos, const vec2& m);

	void select_sample(SampleRef *s, bool diff);

	int detail_steps;
	int preview_sleep_time;
	bool antialiasing;
	bool high_details;
	void set_antialiasing(bool set);
	void set_high_details(bool set);


	void set_midi_view_mode(MidiMode mode);
	MidiMode midi_view_mode;

	ViewMode *mode;
	void _set_mode(ViewMode *m);
	void set_mode(const string& m);
	bool in_mode(const string& m) const;
	string current_mode_name;

	owned_array<ViewMode> all_modes;
	ViewModeDefault *mode_default;
	ViewModeEdit *mode_edit;
	ViewModeEditDummy *mode_edit_dummy;
	ViewModeEditAudio *mode_edit_audio;
	ViewModeEditBars *mode_edit_bars;
	ViewModeEditMidi *mode_edit_midi;
	ViewModeCurve *mode_curve;
	ViewModeCapture *mode_capture;
	ViewModeScaleMarker *mode_scale_marker;

	Session *session;
	TsunamiWindow *win;

	Song *song;

	void set_playback_loop(bool loop);
	void prepare_playback(const Range &range, bool allow_loop);
	bool is_playback_active() const;
	void playback_click();
	bool is_paused() const;
	int playback_pos() const;
	bool looping() const;
	void set_playback_pos(int pos);
	void update_playback_layers();
	Range playback_range() const;

	base::set<const Track*> get_playable_tracks();
	bool has_any_solo_track();
	base::set<const TrackLayer*> get_playable_layers();
	bool has_any_solo_layer(Track *t);

	void __set_cur_sample(SampleRef *s);
	void __set_cur_layer(AudioViewLayer *l);
	HoverData cur_selection;
	HoverData _prev_selection;
	AudioViewTrack *_prev_track = nullptr;
	void set_current(const HoverData &h);
	AudioViewLayer *cur_vlayer();
	AudioViewTrack *cur_vtrack();
	Track *cur_track();
	SampleRef *cur_sample();
	TrackLayer *cur_layer();

	bool editing_track(Track *t);
	bool editing_layer(AudioViewLayer *l);

	rect song_area();
	rect clip;
	TrackHeightManager thm;
	bool update_scene_graph();

	bool playback_range_locked;
	Range playback_wish_range;
	void set_playback_range_locked(bool locked);

	ViewPort cam;
	void cam_changed();

	ScrollBar *scroll_bar_y;
	ScrollBar *scroll_bar_time;
	TimeScale *time_scale;
	scenegraph::Node *background;
	Cursor *cursor_start, *cursor_end;
	SelectionMarker *selection_marker;
	AddTrackButton *add_track_button;

	owned<BufferPainter> buffer_painter;
	owned<GridPainter> grid_painter;

	Array<AudioViewTrack*> vtracks;
	Array<AudioViewLayer*> vlayers;
	AudioViewLayer *metronome_overlay_vlayer;
	shared<Track> dummy_track;
	shared<TrackLayer> dummy_layer;
	shared<AudioViewTrack> dummy_vtrack;
	shared<AudioViewLayer> dummy_vlayer;
	AudioViewTrack *get_track(Track *track);
	AudioViewLayer *get_layer(TrackLayer *layer);
	void update_tracks();

	void implode_track(Track *t);
	void explode_track(Track *t);

	int prefered_buffer_layer;
	double buffer_zoom_factor;
	void update_buffer_zoom();

	struct ImageData {
		owned<Image> speaker, x, solo, config;
		owned<Image> track_audio, track_time, track_midi, track_group;
	} images;

	owned<hui::Menu> menu_track;
	owned<hui::Menu> menu_playback_range;
	owned<hui::Menu> menu_layer;
	owned<hui::Menu> menu_sample;
	owned<hui::Menu> menu_marker;
	owned<hui::Menu> menu_bar;
	owned<hui::Menu> menu_bar_gap;
	owned<hui::Menu> menu_buffer;
	owned<hui::Menu> menu_song;
	void prepare_menu(hui::Menu *menu);
	void open_popup(hui::Menu *menu);


	void set_cursor_pos(int pos);
	int cursor_pos();
	Range cursor_range();
	void select_under_cursor();
	bool hover_any_object();
	bool hover_selected_object();
	void select_object();
	void toggle_object();
	void exclusively_select_object();
	bool exclusively_select_layer(AudioViewLayer *l);
	bool exclusively_select_track(AudioViewTrack *t);
	void toggle_select_layer(AudioViewLayer *l);
	void toggle_select_layer_with_content_in_cursor(AudioViewLayer *l);
	void toggle_select_track_with_content_in_cursor(AudioViewTrack *t);


	AudioViewLayer *next_layer(AudioViewLayer *vlayer);
	AudioViewLayer *prev_layer(AudioViewLayer *vlayer);
	void move_to_layer(int delta);
	void zoom_y(float zoom);
	void toggle_track_mute();
	void toggle_track_solo();
	void toggle_layer_mute();
	void toggle_layer_solo();
	void toggle_track_exploded();

	owned<PeakDatabase> peak_database;
};

}

#endif /* AUDIOVIEW_H_ */
