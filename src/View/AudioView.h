/*
 * AudioView.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef AUDIOVIEW_H_
#define AUDIOVIEW_H_

#include "../lib/base/pointer.h"
#include "../Data/SongSelection.h"
#include "../Data/Midi/Scale.h"
#include "../Stuff/Observable.h"
#include "TrackHeightManager.h"
#include "ViewPort.h"
#include "ColorScheme.h"
#include <atomic>
#include "HoverData.h"

namespace hui{
	class Menu;
}

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
class PeakThread;
class ViewMode;
class ViewModeDefault;
class ViewModeEdit;
class ViewModeEditDummy;
class ViewModeEditAudio;
class ViewModeMidi;
class ViewModeScaleBars;
class ViewModeCurve;
class ViewModeCapture;
class ViewModeScaleMarker;
class ScrollBar;
class Session;
class MidiPainter;
class BufferPainter;
class GridPainter;
namespace scenegraph {
	class SceneGraph;
	class Node;
}
class TimeScale;
class Cursor;
class SelectionMarker;
class MouseDelayPlanner;
class MouseDelayAction;
class CpuDisplay;
class PeakMeterDisplay;
class Dial;
class BottomBarExpandButton;
enum class MidiMode;


enum class SelectionSnapMode {
	NONE,
	BAR,
	PART,
};

enum class SelectionMode {
	NONE,
	TIME,
	RECT,
	TRACK_RECT,
	FAKE,
};

class AudioView : public Observable<VirtualBase> {
public:
	AudioView(Session *session, const string &id);
	virtual ~AudioView();

	void check_consistency();
	void force_redraw();
	void force_redraw_part(const rect &r);

	void on_draw(Painter *p);
	void on_mouse_move();
	void on_left_button_down();
	void on_left_button_up();
	void on_middle_button_down();
	void on_middle_button_up();
	void on_right_button_down();
	void on_right_button_up();
	void on_left_double_click();
	void on_mouse_leave();
	void on_mouse_wheel();
	void on_key_down();
	void on_key_up();
	void on_command(const string &id);

	void on_song_change();
	void on_song_new();
	void on_song_finished_loading();
	void on_song_tracks_change();
	void on_stream_tick();
	void on_stream_state_change();
	void on_update();
	static const string MESSAGE_CUR_TRACK_CHANGE;
	static const string MESSAGE_CUR_SAMPLE_CHANGE;
	static const string MESSAGE_CUR_LAYER_CHANGE;
	static const string MESSAGE_SELECTION_CHANGE;
	static const string MESSAGE_SETTINGS_CHANGE;
	static const string MESSAGE_VIEW_CHANGE;
	static const string MESSAGE_VTRACK_CHANGE;
	static const string MESSAGE_SOLO_CHANGE;

	void update_peaks();
	void zoom_in();
	void zoom_out();

	void draw_time_line(Painter *c, int pos, const color &col, bool hover, bool show_time = false, bool show_circle = false);
	void draw_song(Painter *c);
	int draw_runner_id;

	static rect get_boxed_str_rect(Painter *c, float x, float y, const string &str);
	static void draw_boxed_str(Painter *c, float x, float y, const string &str, const color &col_text, const color &col_bg, int align=1);
	static void draw_framed_box(Painter *p, const rect &r, const color &bg, const color &frame, float frame_width);
	static float draw_str_constrained(Painter *p, float x, float y, float w_max, const string &str, int align=1);

	static void draw_cursor_hover(Painter *c, const string &msg, float mx, float my, const rect &area);
	void draw_cursor_hover(Painter *c, const string &msg);

	void request_optimize_view();
	void perform_optimize_view();
	bool _optimize_view_requested;
	void update_menu();

	int perf_channel;

	string id;

	Array<ColorScheme> color_schemes;
	static ColorScheme basic_colors;
	static ColorScheme colors;
	void set_color_scheme(const string &name);

	static const int SAMPLE_FRAME_HEIGHT;
	static const int TIME_SCALE_HEIGHT;
	static const float LINE_WIDTH;
	static const float CORNER_RADIUS;
	static const float FONT_SIZE;
	static const int MAX_TRACK_CHANNEL_HEIGHT;
	static const int TRACK_HANDLE_WIDTH;
	static const int LAYER_HANDLE_WIDTH;
	static const int TRACK_HANDLE_HEIGHT;
	static const int TRACK_HANDLE_HEIGHT_SMALL;
	static const int SCROLLBAR_WIDTH;
	static const int SNAPPING_DIST;

	HoverData &hover();
	HoverData hover_before_leave;
	SongSelection sel;
	SongSelection sel_temp;

	HoverData hover_time(float mx, float my);
	int get_mouse_pos();
	int get_mouse_pos_snap();

	SelectionMode selection_mode;
	SelectionSnapMode selection_snap_mode;
	void set_selection_snap_mode(SelectionSnapMode mode);
	bool hide_selection;


	MouseDelayPlanner *mdp();
	void mdp_prepare(MouseDelayAction *action);
	void mdp_run(MouseDelayAction *action);
	void mdp_prepare(hui::Callback update);


	void snap_to_grid(int &pos);

	void _cdecl unselect_all_samples();

	bool enabled;
	void enable(bool enabled);

	float ScrollSpeed;
	float ScrollSpeedFast;
	float ZoomSpeed;
	float mouse_wheel_speed;
	void set_mouse_wheel_speed(float speed);

	int mx, my;
	bool select_xor = false;

	void select_none();
	void select_all();
	void select_expand();
	void update_selection();
	void set_selection(const SongSelection &s);
	Range get_playback_selection(bool for_recording);

	void set_mouse();
	int mouse_over_sample(SampleRef *s);
	void selection_update_pos(HoverData &s);
	bool mouse_over_time(int pos);

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
	void set_mode(ViewMode *m);

	owned_array<ViewMode> all_modes;
	ViewModeDefault *mode_default;
	ViewModeEdit *mode_edit;
	ViewModeEditDummy *mode_edit_dummy;
	ViewModeEditAudio *mode_edit_audio;
	ViewModeMidi *mode_edit_midi;
	ViewModeScaleBars *mode_scale_bars;
	ViewModeCurve *mode_curve;
	ViewModeCapture *mode_capture;
	ViewModeScaleMarker *mode_scale_marker;

	Session *session;
	TsunamiWindow *win;

	Song *song;

	SignalChain *signal_chain;
	SongRenderer *renderer;
	PeakMeter *peak_meter;
	AudioOutput *output_stream;
	void set_playback_loop(bool loop);
	void play();
	void stop();
	void pause(bool pause);
	void prepare_playback(const Range &range, bool allow_loop);
	bool is_playback_active();
	void playback_click();
	bool is_paused();
	int playback_pos();
	bool looping();
	void _sync_playback_pos();
	int _playback_sync_counter = 0;
	int64 _playback_stream_offset = 0;
	void set_playback_pos(int pos);
	Set<const Track*> get_playable_tracks();
	bool has_any_solo_track();
	Set<const TrackLayer*> get_playable_layers();
	bool has_any_solo_layer(Track *t);

	void __set_cur_sample(SampleRef *s);
	void __set_cur_layer(AudioViewLayer *l);
	HoverData cur_selection;
	HoverData _prev_selection;
	void set_current(const HoverData &h);
	AudioViewLayer *cur_vlayer();
	AudioViewTrack *cur_vtrack();
	Track *cur_track();
	SampleRef *cur_sample();
	TrackLayer *cur_layer();

	bool editing_track(Track *t);
	bool editing_layer(AudioViewLayer *l);

	rect area;
	rect song_area();
	rect clip;
	TrackHeightManager thm;
	shared<scenegraph::SceneGraph> scene_graph;
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
	CpuDisplay *cpu_display;
	PeakMeterDisplay *peak_meter_display;
	Dial *output_volume_dial;
	BottomBarExpandButton *bottom_bar_expand_button;
	scenegraph::Node *onscreen_display;
	void update_onscreen_displays();


	owned<MidiPainter> midi_painter;
	owned<BufferPainter> buffer_painter;
	owned<GridPainter> grid_painter;

	Array<AudioViewTrack*> vtracks;
	Array<AudioViewLayer*> vlayers;
	AudioViewLayer *metronome_overlay_vlayer;
	shared<AudioViewTrack> dummy_vtrack;
	shared<AudioViewLayer> dummy_vlayer;
	AudioViewTrack *get_track(Track *track);
	AudioViewLayer *get_layer(TrackLayer *layer);
	void update_tracks();

	void implode_track(Track *t);
	void explode_track(Track *t);

	void update_peaks_now(AudioBuffer &buf);

	int prefered_buffer_layer;
	double buffer_zoom_factor;
	void update_buffer_zoom();

	struct Message {
		string text;
		float ttl;
		float size;
	} message;
	void set_message(const string &text, float size=1.0f);
	void draw_message(Painter *c, Message &m);

	owned<PeakThread> peak_thread;

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
};

#endif /* AUDIOVIEW_H_ */
