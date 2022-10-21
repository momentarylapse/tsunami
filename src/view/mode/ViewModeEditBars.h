/*
 * ViewModeEditBars.h
 *
 *  Created on: 22.10.2022
 *      Author: michi
 */

#ifndef SRC_VIEW_MODE_VIEWMODEEDITBARS_H_
#define SRC_VIEW_MODE_VIEWMODEEDITBARS_H_

#include "ViewModeDefault.h"

class TrackLayer;

class ViewModeEditBars : public ViewModeDefault {
public:
	ViewModeEditBars(AudioView *view);

	void on_start() override;
	void on_end() override;

	void on_key_down(int k) override;
	float layer_suggested_height(AudioViewLayer *l) override;

	void draw_post(Painter *c) override;


	enum class EditMode {
		SELECT,
		REMOVE_CLICKS,
		SMOOTHEN,
		CLONE,
		RUBBER,
	};
	void set_edit_mode(EditMode mode);
	EditMode edit_mode;
	
	AudioViewLayer *cur_vlayer();
	TrackLayer *cur_layer();
	bool editing(AudioViewLayer *l);

	void left_click_handle_void(AudioViewLayer *vlayer) override;
	void on_mouse_move() override;
	string get_tip() override;
};

#endif /* SRC_VIEW_MODE_VIEWMODEEDITBARS_H_ */
