/*
 * ViewModeEditAudio.h
 *
 *  Created on: 28.08.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_MODE_VIEWMODEEDITAUDIO_H_
#define SRC_VIEW_MODE_VIEWMODEEDITAUDIO_H_

#include "ViewModeDefault.h"

class TrackLayer;

class ViewModeEditAudio : public ViewModeDefault {
public:
	ViewModeEditAudio(AudioView *view);

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
	
	float edit_radius;
	
	Range range_source();
	Range range_target();



	AudioViewLayer *cur_vlayer();
	TrackLayer *cur_layer();
	bool editing(AudioViewLayer *l);

	void apply_stretch();

	bool flag_pitch_compensate = false;


	void left_click_handle_void(AudioViewLayer *vlayer, const vec2 &m) override;
	void on_mouse_move(const vec2& m) override;
	string get_tip() override;
};

#endif /* SRC_VIEW_MODE_VIEWMODEEDITAUDIO_H_ */
