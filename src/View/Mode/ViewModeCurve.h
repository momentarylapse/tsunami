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
	virtual ~ViewModeCurve();

	virtual void on_left_button_down();
	virtual void on_left_button_up();
	virtual void on_mouse_move();
	virtual void on_key_down(int k);

	virtual void draw_track_data(Painter *c, AudioViewTrack *t);

	virtual Selection get_hover();


	float value2screen(float value);
	float screen2value(float y);

	Curve *curve;
	AudioViewTrack *cur_track;
	void setCurve(Curve *c);
};

#endif /* SRC_VIEW_MODE_VIEWMODECURVE_H_ */
