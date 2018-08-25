/*
 * ViewModeScaleBars.h
 *
 *  Created on: 13.11.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_MODE_VIEWMODESCALEBARS_H_
#define SRC_VIEW_MODE_VIEWMODESCALEBARS_H_

#include "ViewModeDefault.h"
#include "../../Data/Range.h"
#include "../../lib/base/base.h"

class ViewModeScaleBars : public ViewModeDefault
{
public:
	ViewModeScaleBars(AudioView *view);
	virtual ~ViewModeScaleBars();

	virtual void on_left_button_up();
	virtual void on_right_button_down();
	virtual void on_mouse_move();
	virtual void on_key_down(int k);

	virtual void draw_post(Painter *c);

	void start_scaling(const Array<int> &sel);
	void perform_scale();

	bool scaling_change;
	Array<int> scaling_sel;
	Range scaling_range_orig;
};

#endif /* SRC_VIEW_MODE_VIEWMODESCALEBARS_H_ */
