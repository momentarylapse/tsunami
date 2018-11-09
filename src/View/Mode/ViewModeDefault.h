/*
 * ViewModeDefault.h
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_MODE_VIEWMODEDEFAULT_H_
#define SRC_VIEW_MODE_VIEWMODEDEFAULT_H_

#include "ViewMode.h"

class ActionSongMoveSelection;
class SongSelection;
class Range;
class Track;

class ViewModeDefault : public ViewMode
{
public:
	ViewModeDefault(AudioView *view);
	virtual ~ViewModeDefault();

	void on_left_button_down() override;
	void on_left_button_up() override;
	void on_left_double_click() override;
	void on_right_button_down() override;
	void on_right_button_up() override;
	void on_mouse_wheel() override;
	void on_mouse_move() override;
	void on_key_down(int k) override;
	void on_key_up(int k) override;
	float layer_min_height(AudioViewLayer *l) override;
	float layer_suggested_height(AudioViewLayer *l) override;


	void draw_imploded_track_background(Painter *c, AudioViewTrack *t) override;
	void draw_imploded_track_data(Painter *c, AudioViewTrack *t) override;
	void draw_layer_background(Painter *c, AudioViewLayer *l) override;
	void draw_track_data(Painter *c, AudioViewTrack *t) override;
	void draw_layer_data(Painter *c, AudioViewLayer *l) override;
	void draw_midi(Painter *c, AudioViewLayer *l, const MidiNoteBuffer &midi, bool as_reference, int shift) override;
	void draw_post(Painter *c) override;

	MidiMode which_midi_mode(Track *t) override;

	SongSelection get_selection_for_range(const Range &r) override;
	SongSelection get_selection_for_rect(const Range &r, int y0, int y1) override;
	SongSelection get_selection_for_track_rect(const Range &r, int y0, int y1) override;
	void start_selection() override;

	void select_under_mouse();
	void set_cursor_pos(int pos, bool keep_track_selection);
	Selection get_hover() override;
	Selection get_hover_basic(bool editable);

	int get_track_move_target(bool visual);

	// drag and drop
	ActionSongMoveSelection *cur_action;
	SongSelection *dnd_selection;
	int dnd_ref_pos;
	int dnd_mouse_pos0;
	void dnd_start_soon(const SongSelection &sel);
	void dnd_start();
	void dnd_stop();
	void dnd_update();

	Track* moving_track;
};

#endif /* SRC_VIEW_MODE_VIEWMODEDEFAULT_H_ */
