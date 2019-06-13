/*
 * TimeScale.h
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_NODE_TIMESCALE_H_
#define SRC_VIEW_NODE_TIMESCALE_H_

#include "ViewNode.h"

class TimeScale : public ViewNode {
public:
	TimeScale(AudioView *view);
	void draw(Painter *p) override;

	bool allow_handle_click_when_gaining_focus() override { return false; }

	bool on_left_button_down() override;
	bool on_right_button_down() override;

	HoverData get_hover_data(float mx, float my) override;
};

#endif /* SRC_VIEW_NODE_TIMESCALE_H_ */
