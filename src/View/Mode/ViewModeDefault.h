/*
 * ViewModeDefault.h
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_MODE_VIEWMODEDEFAULT_H_
#define SRC_VIEW_MODE_VIEWMODEDEFAULT_H_

#include "ViewMode.h"

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
	void on_mouse_wheel() override;
	void on_mouse_move() override;
	void on_key_down(int k) override;
	float layer_min_height(AudioViewLayer *l) override;
	float layer_suggested_height(AudioViewLayer *l) override;

	void draw_layer_background(Painter *c, AudioViewLayer *l) override;
	void draw_post(Painter *c) override;

	MidiMode which_midi_mode(Track *t) override;

	SongSelection get_selection_for_range(const Range &r) override;
	SongSelection get_selection_for_rect(const Range &r, int y0, int y1) override;
	SongSelection get_selection_for_track_rect(const Range &r, int y0, int y1) override;
	void start_selection() override;

	void select_hover();
	void set_cursor_pos(int pos, bool keep_track_selection);
	HoverData get_hover_data(AudioViewLayer *vlayer) override;


	void left_click_handle(AudioViewLayer *vlayer) override;
	void left_click_handle_object(AudioViewLayer *vlayer) override;
	void left_click_handle_void(AudioViewLayer *vlayer) override;
	void left_click_handle_xor(AudioViewLayer *vlayer) override;
	void left_click_handle_object_xor(AudioViewLayer *vlayer) override;

	void start_selection_rect(AudioViewLayer *vlayer, SelectionMode mode);
};

#endif /* SRC_VIEW_MODE_VIEWMODEDEFAULT_H_ */
