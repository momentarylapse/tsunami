/*
 * ViewModeEditBars.h
 *
 *  Created on: 22.10.2022
 *      Author: michi
 */

#ifndef SRC_VIEW_MODE_VIEWMODEEDITBARS_H_
#define SRC_VIEW_MODE_VIEWMODEEDITBARS_H_

#include "ViewModeDefault.h"

namespace tsunami {

class TrackLayer;

class ViewModeEditBars : public ViewModeDefault {
public:
	explicit ViewModeEditBars(AudioView *view);

	void on_start() override;
	void on_end() override;

	void on_key_down(int k) override;
	float layer_suggested_height(AudioViewLayer *l) override;

	void draw_post(Painter *c) override;

	void add_bar_at_cursor(const vec2 &m);

	enum class EditMode {
		Select,
		AddAndSplit,
		Rubber,
	};
	void set_edit_mode(EditMode mode);
	EditMode edit_mode;
	
	AudioViewLayer *cur_vlayer();
	TrackLayer *cur_layer();
	bool editing(AudioViewLayer *l);

	void left_click_handle_void(AudioViewLayer *vlayer, const vec2 &m) override;
	void on_mouse_move(const vec2& m) override;
	string get_tip() override;

	Range selected_bar_range() const;
	int rubber_end_target = 0;
	bool rubber_hover = false;
};

}

#endif /* SRC_VIEW_MODE_VIEWMODEEDITBARS_H_ */
