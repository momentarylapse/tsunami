/*
 * ViewMode.h
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_MODE_VIEWMODE_H_
#define SRC_VIEW_MODE_VIEWMODE_H_

#include "../../Stuff/Observable.h"
#include "../../lib/base/set.h"

enum class MidiMode;
enum class SelectionMode;
class AudioView;
class AudioViewTrack;
class AudioViewLayer;
class HoverData;
class SongSelection;
class Track;
class ViewPort;
class TsunamiWindow;
class Song;
class MidiNoteBuffer;
class Painter;
class rect;
class color;
class Range;
class Session;

class ViewMode : public Observable<VirtualBase> {
public:
	ViewMode(AudioView *view);

	virtual void on_start() {}
	virtual void on_end() {}

	virtual void on_left_button_down() {}
	virtual void on_left_button_up() {}
	virtual void on_left_double_click() {}
	virtual void on_right_button_down() {}
	virtual void on_right_button_up() {}
	virtual void on_mouse_move() {}
	virtual void on_mouse_wheel() {}
	virtual void on_key_down(int k) {}
	virtual void on_key_up(int k) {}
	virtual void on_command(const string &id) {}
	virtual float layer_suggested_height(AudioViewLayer *l) = 0;
	virtual void on_cur_layer_change() {}

	virtual HoverData get_hover_data(AudioViewLayer *vlayer, float mx, float my);

	virtual void draw_track_background(Painter *c, AudioViewTrack *t) {}
	virtual void draw_layer_background(Painter *c, AudioViewLayer *l) {}
	virtual void draw_track_data(Painter *c, AudioViewTrack *t) {}
	virtual void draw_post(Painter *c) {}

	SongSelection get_selection(const Range &r, SelectionMode mode);
	virtual SongSelection get_selection_for_range(const Range &r);
	virtual SongSelection get_selection_for_rect(const Range &r, int y0, int y1);
	virtual SongSelection get_selection_for_track_rect(const Range &r, int y0, int y1);
	virtual void start_selection() {}

	virtual Set<Track*> prevent_playback(){ return {}; }

	virtual int suggest_move_cursor(const Range &cursor, bool forward) = 0;

	virtual void left_click_handle(AudioViewLayer *vlayer) {}
	virtual void left_click_handle_object(AudioViewLayer *vlayer) {}
	virtual void left_click_handle_object_xor(AudioViewLayer *vlayer) {}
	virtual void left_click_handle_void(AudioViewLayer *vlayer) {}
	virtual void left_click_handle_void_xor(AudioViewLayer *vlayer) {}

	virtual string get_tip() { return ""; }

	AudioView *view;
	Session *session;
	//ViewPort &cam();
	ViewPort *cam;
	HoverData &hover();
	TsunamiWindow *win;
	Song *song;

	void set_side_bar(int console);
};

#endif /* SRC_VIEW_MODE_VIEWMODE_H_ */
