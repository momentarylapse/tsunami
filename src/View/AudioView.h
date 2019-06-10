/*
 * AudioView.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef AUDIOVIEW_H_
#define AUDIOVIEW_H_

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
class SceneGraph;
class ViewNode;
class TimeScale;
class Cursor;
class SelectionMarker;
class MouseDelayPlanner;
class MouseDelayAction;
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

class AudioView : public Observable<VirtualBase>
{
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

	void on_song_update();
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
	void draw_selection(Painter *c);
	void draw_background(Painter *c);
	void draw_song(Painter *c);
	int draw_runner_id;

	static rect get_boxed_str_rect(Painter *c, float x, float y, const string &str);
	static void draw_boxed_str(Painter *c, float x, float y, const string &str, const color &col_text, const color &col_bg, int align=1);

	static void draw_cursor_hover(Painter *c, const string &msg, float mx, float my, const rect &area);
	void draw_cursor_hover(Painter *c, const string &msg);

	void optimize_view();
	void update_menu();

	string id;

	Array<ColorSchemeBasic> basic_schemes;
	static ColorSchemeBasic basic_colors;
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

	HoverData hover;
	HoverData hover_before_leave;
	SongSelection sel;
	SongSelection sel_temp;

	HoverData hover_time();
	int get_mouse_pos();
	int get_mouse_pos_snap();

	SelectionMode selection_mode;
	SelectionSnapMode selection_snap_mode;
	void set_selection_snap_mode(SelectionSnapMode mode);
	bool hide_selection;


	MouseDelayPlanner *mdp;
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
	ViewModeDefault *mode_default;
	ViewModeMidi *mode_midi;
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
	bool playback_loop;
	void set_playback_loop(bool loop);
	void play();
	void stop();
	void pause(bool pause);
	void prepare_playback(const Range &range, bool allow_loop);
	bool is_playback_active();
	void playback_click();
	bool is_paused();
	int playback_pos();
	Set<const Track*> get_playable_tracks();
	bool has_any_solo_track();
	Set<const TrackLayer*> get_playable_layers();
	bool has_any_solo_layer(Track *t);

	void set_cur_sample(SampleRef *s);
	//void setCurTrack(Track *t);
	void set_cur_layer(AudioViewLayer *l);
	AudioViewLayer *cur_vlayer;
	Track *_prev_cur_track;
	Track *cur_track();
	SampleRef *cur_sample;
	TrackLayer *cur_layer();

	bool editing_track(Track *t);
	bool editing_layer(AudioViewLayer *l);

	rect area;
	rect song_area;
	rect clip;
	TrackHeightManager thm;
	SceneGraph *scene_graph;
	bool update_scene_graph();
	void rebuild_scene_graph();

	bool playback_range_locked;
	Range playback_wish_range;
	void set_playback_range_locked(bool locked);

	ViewPort cam;
	void cam_changed();

	ScrollBar *scroll_bar_h;
	ScrollBar *scroll_bar_w;
	TimeScale *time_scale;
	ViewNode *background;
	Cursor *cursor_start, *cursor_end;
	SelectionMarker *selection_marker;


	MidiPainter *midi_painter;
	BufferPainter *buffer_painter;
	GridPainter *grid_painter;

	Array<AudioViewTrack*> vtrack;
	Array<AudioViewLayer*> vlayer;
	AudioViewLayer *metronome_overlay_vlayer;
	AudioViewTrack *dummy_vtrack;
	AudioViewLayer *dummy_vlayer;
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

	PeakThread *peak_thread;

	struct ImageData {
		Image *speaker, *x, *solo;
		Image *speaker_bg, *x_bg, *solo_bg;
		Image *track_audio, *track_time, *track_midi;
		Image *track_audio_bg, *track_time_bg, *track_midi_bg;
	} images;

	hui::Menu *menu_track;
	hui::Menu *menu_playback_range;
	hui::Menu *menu_layer;
	hui::Menu *menu_sample;
	hui::Menu *menu_marker;
	hui::Menu *menu_bar;
	hui::Menu *menu_bar_gap;
	hui::Menu *menu_buffer;
	hui::Menu *menu_song;
	void prepare_menu(hui::Menu *menu);
	void open_popup(hui::Menu *menu);

	int perf_channel;


	void set_cursor_pos(int pos);
	void select_under_cursor();
	bool hover_any_object();
	bool hover_selected_object();
	void select_object();
	void toggle_object();
	void exclusively_select_object();
	bool exclusively_select_layer(AudioViewLayer *l);
	void toggle_select_layer(AudioViewLayer *l);
	void toggle_select_layer_with_content_in_cursor(AudioViewLayer *l);
};

#endif /* AUDIOVIEW_H_ */
