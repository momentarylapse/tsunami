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

class ScrollBar : public ViewNode
{
public:
	ScrollBar(AudioView *view, std::function<void()> callback);//ViewNode *parent);
	bool scrolling = false;
	float offset = 0;
	float page_size = 0;
	float content_size = 0;
	float mouse_offset = 0;
	void drag_start(float mx, float my);
	void drag_update(float mx, float my);
	void draw(Painter *c) override;
	void set_area(const rect &r);
	void update(float page, float content);

	void on_left_button_down() override;
	void on_left_button_up() override;
	void on_mouse_move() override;

	std::function<void()> callback;
};

#endif /* SRC_VIEW_NODE_SCROLLBAR_H_ */
