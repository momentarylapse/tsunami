/*
 * ViewModeDefault.h
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_MODE_VIEWMODEDEFAULT_H_
#define SRC_VIEW_MODE_VIEWMODEDEFAULT_H_

#include "ViewMode.h"

namespace tsunami {

class SongSelection;
class Range;
class Track;

class ViewModeDefault : public ViewMode {
public:
	explicit ViewModeDefault(AudioView *view);

	void on_mouse_wheel(const vec2 &d) override;
	void on_key_down(int k) override;
	void on_command(const string &id) override;
	void on_gesture(const string &id, const vec2 &m, const vec2 &param) override;
	float layer_suggested_height(AudioViewLayer *l) override;
	int suggest_move_cursor(const Range &cursor, bool forward) override;

	void draw_selected_layer_highlight(Painter *p, const rect &area);
	void draw_post(Painter *p) override;
	void draw_layer_background(Painter *p, AudioViewLayer *l) override;

	SongSelection get_selection_for_range(const Range &r) override;
	SongSelection get_selection_for_rect(const Range &r, int y0, int y1) override;
	SongSelection get_selection_for_track_rect(const Range &r, int y0, int y1) override;

	HoverData get_hover_data(AudioViewLayer *vlayer, const vec2 &m) override;


	void left_click_handle(AudioViewLayer *vlayer, const vec2 &m) override;
	void left_click_handle_object(AudioViewLayer *vlayer, const vec2 &m) override;
	void left_click_handle_object_xor(AudioViewLayer *vlayer, const vec2 &m) override;
	void left_click_handle_void(AudioViewLayer *vlayer, const vec2 &m) override;
	void left_click_handle_void_or(AudioViewLayer *vlayer, const vec2 &m) override;
	void left_click_handle_void_xor(AudioViewLayer *vlayer, const vec2 &m) override;

	void start_selection_rect(const vec2 &m, SelectionMode mode, bool keep_start = false);
};

}

#endif /* SRC_VIEW_MODE_VIEWMODEDEFAULT_H_ */
