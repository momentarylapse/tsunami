/*
 * ViewModeCurve.h
 *
 *  Created on: 14.11.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_MODE_VIEWMODECURVE_H_
#define SRC_VIEW_MODE_VIEWMODECURVE_H_

#include "ViewModeDefault.h"

class Curve;

class ViewModeCurve : public ViewModeDefault
{
public:
	ViewModeCurve(AudioView *view);

	void on_left_button_down() override;
	void on_left_button_up() override;
	void on_mouse_move() override;
	void on_key_down(int k) override;

	void draw_track_data(Painter *c, AudioViewTrack *t) override;

	Selection get_hover() override;


	float value2screen(float value);
	float screen2value(float y);

	Curve *curve;
	AudioViewTrack *cur_track;
	void setCurve(Curve *c);
};

#endif /* SRC_VIEW_MODE_VIEWMODECURVE_H_ */
