/*
 * Background.h
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_NODE_BACKGROUND_H_
#define SRC_VIEW_NODE_BACKGROUND_H_

#include "ViewNode.h"

class AudioView;

class Background : public ViewNode {
public:
	Background(AudioView *view);

	bool on_left_button_down() override;
	bool on_right_button_down() override;
	bool allow_handle_click_when_gaining_focus() override { return false; }

	void draw(Painter *p) override;

	AudioView *view;
	HoverData get_hover_data(float mx, float my) override;
};

#endif /* SRC_VIEW_NODE_BACKGROUND_H_ */
