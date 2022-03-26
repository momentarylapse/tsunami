/*
 * TimeScale.h
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_GRAPH_TIMESCALE_H_
#define SRC_VIEW_GRAPH_TIMESCALE_H_

#include "../../helper/graph/Node.h"

class AudioView;

class TimeScale : public scenegraph::NodeRel {
public:
	TimeScale(AudioView *view);
	void on_draw(Painter *p) override;

	bool allow_handle_click_when_gaining_focus() const override { return false; }

	bool on_left_button_down(const vec2 &m) override;
	bool on_right_button_down(const vec2 &m) override;

	HoverData get_hover_data(const vec2 &m) override;

	AudioView *view;
};

#endif /* SRC_VIEW_GRAPH_TIMESCALE_H_ */
