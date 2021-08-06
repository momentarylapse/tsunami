/*
 * ViewModeEdit.h
 *
 *  Created on: May 1, 2020
 *      Author: michi
 */

#ifndef SRC_VIEW_MODE_VIEWMODEEDIT_H_
#define SRC_VIEW_MODE_VIEWMODEEDIT_H_


#include "ViewModeDefault.h"

class ViewModeEdit : public ViewModeDefault {
public:
	ViewModeEdit(AudioView *view);
	virtual ~ViewModeEdit();

	ViewMode *mode;
	void set_mode(ViewMode *m);
	ViewMode *suggest_mode();


	void on_start() override;
	void on_end() override;

	void on_key_down(int k) override;
	void on_command(const string &id) override;
	float layer_suggested_height(AudioViewLayer *l) override;
	void on_cur_layer_change() override;

	void draw_layer_background(Painter *c, AudioViewLayer *l) override;
	void draw_post(Painter *c) override;

	HoverData get_hover_data(AudioViewLayer *vlayer, const vec2 &m) override;
	SongSelection get_selection_for_rect(const Range &r, int y0, int y1) override;
	SongSelection get_selection_for_range(const Range &r) override;


	void left_click_handle_void(AudioViewLayer *vlayer) override;
	string get_tip() override;

	bool editing(AudioViewLayer *l);
};

#endif /* SRC_VIEW_MODE_VIEWMODEEDIT_H_ */
