/*
 * ScrollBar.h
 *
 *  Created on: 22.08.2018
 *      Author: michi
 */

#pragma once

#include "Node.h"

class Painter;

namespace tsunami {

class Range;

class ScrollBar : public scenegraph::Node {
public:
	ScrollBar();
	bool constrained = true;

	obs::xsource<float> out_offset{this, "offset"};

	float get_view_offset() const;

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

	void drag_update(const vec2 &m);

	void on_draw(Painter *c) override;

	bool on_left_button_down(const vec2 &m) override;

	void update_geometry(const rect &target_area) override;
};

class ScrollBarHorizontal : public ScrollBar {
public:
	ScrollBarHorizontal();
};

}
