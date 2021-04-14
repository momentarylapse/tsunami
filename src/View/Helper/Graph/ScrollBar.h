/*
 * ScrollBar.h
 *
 *  Created on: 22.08.2018
 *      Author: michi
 */

#pragma once

#include "Node.h"

class Painter;
class Range;

class ScrollBar : public scenegraph::Node {
public:
	ScrollBar();
	bool constrained = true;


	//float offset = 0;
	float get_view_offset() const;
	//float get_size() const;

	// view/page
	float view_offset = 0;
	float view_size = 1;

	float content_offset = 0;
	float content_size = 1;

	float mouse_offset = 0;
	bool horizontal = false;
	bool auto_hide = false;

	float project(float x) const;
	float unproject(float x) const;

	bool content_covered_by_view() const;
	void set_view(float start, float end);
	void set_view(const Range &r);
	void set_view_size(float size);
	void set_view_offset(float offset);
	void move_view(float d);
	void set_content(float start, float end);
	void set_content(const Range &r);

	void drag_update(float mx, float my);

	//void set_offset(float offset);
	void on_draw(Painter *c) override;
	//void set_area(const rect &r);
	//void update(float page, float content);

	bool on_left_button_down(float mx, float my) override;

	void update_geometry(const rect &target_area) override;

	std::function<void(float)> cb_update_view;
	void set_callback(std::function<void(float)> callback);
};

class ScrollBarHorizontal : public ScrollBar {
public:
	ScrollBarHorizontal();
};
