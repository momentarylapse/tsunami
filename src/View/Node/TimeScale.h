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
	string get_tip() override;

	bool hover_lock_button();
	bool hover_loop_button();
	bool hover_playback();

	bool on_left_button_down() override;
	bool on_right_button_down() override;

	rect playback_lock_button = rect::EMPTY;
	rect playback_loop_button = rect::EMPTY;
};

#endif /* SRC_VIEW_NODE_TIMESCALE_H_ */
