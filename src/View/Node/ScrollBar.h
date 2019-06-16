/*
 * ScrollBar.h
 *
 *  Created on: 22.08.2018
 *      Author: michi
 */

#ifndef SRC_VIEW_NODE_SCROLLBAR_H_
#define SRC_VIEW_NODE_SCROLLBAR_H_

#include "../../lib/base/base.h"
#include "ViewNode.h"

class Painter;
class AudioView;

class ScrollBar : public ViewNode {
public:
	ScrollBar(AudioView *view);
	//ScrollBar(AudioView *view);
	bool constrained = true;
	float offset = 0;
	float page_size = 0;
	float content_size = 0;
	float mouse_offset = 0;
	bool horizontal = false;
	void drag_update(float mx, float my);
	void draw(Painter *c) override;
	void set_area(const rect &r);
	void update(float page, float content);

	bool on_left_button_down() override;

	AudioView *view;
	std::function<void()> callback;
	void set_callback(std::function<void()> callback);
};

class ScrollBarHorizontal : public ScrollBar {
public:
	ScrollBarHorizontal(AudioView *view);
};

#endif /* SRC_VIEW_NODE_SCROLLBAR_H_ */
