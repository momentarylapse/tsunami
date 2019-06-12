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

	void on_start() override;

	void on_left_button_up() override;
	void on_right_button_down() override;
	void on_key_down(int k) override;

	void draw_post(Painter *c) override;
	string get_tip() override;

	void start_scaling(const Array<int> &sel);
	void perform_scale();

	Array<int> scaling_sel;
	Range scaling_range_orig;
};

#endif /* SRC_VIEW_MODE_VIEWMODESCALEBARS_H_ */
