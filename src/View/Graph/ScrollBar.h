/*
 * ScrollBar.h
 *
 *  Created on: 22.08.2018
 *      Author: michi
 */

#ifndef SRC_VIEW_GRAPH_SCROLLBAR_H_
#define SRC_VIEW_GRAPH_SCROLLBAR_H_

#include "../../lib/base/base.h"
#include "../Helper/Graph/Node.h"

class Painter;
class AudioView;

class ScrollBar : public scenegraph::Node {
public:
	ScrollBar(AudioView *view);
	//ScrollBar(AudioView *view);
	bool constrained = true;
	float offset = 0;
	float page_size = 0;
	float content_size = 0;
	float mouse_offset = 0;
	bool horizontal = false;
	bool auto_hide = false;
	void drag_update(float mx, float my);
	void set_offset(float offset);
	void draw(Painter *c) override;
	void set_area(const rect &r);
	void update(float page, float content);

	bool on_left_button_down() override;

	void update_geometry(const rect &target_area) override;

	AudioView *view;
	std::function<void()> callback;
	void set_callback(std::function<void()> callback);
};

class ScrollBarHorizontal : public ScrollBar {
public:
	ScrollBarHorizontal(AudioView *view);
};

#endif /* SRC_VIEW_GRAPH_SCROLLBAR_H_ */
