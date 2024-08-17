/*
 * Scrollable.h
 *
 *  Created on: Apr 13, 2021
 *      Author: michi
 */

#pragma once

#include "Node.h"

namespace scenegraph {
	class HBox;
	class VBox;
}

namespace tsunami {
//namespace scenegraph {
	class ScrollBar;
	class ScrollBarHorizontal;

class ScrollPad;

template<class T>
class Scrollable : public T {
public:
	ScrollPad *pad;

	/*complex project(const complex &p) {
		return pad->project(p);
	}
	complex unproject(const complex &p) {
		return pad->unproject(p);
	}*/
};

class ScrollPad : public scenegraph::NodeRel {
public:
	ScrollBar *scrollbar_v;
	ScrollBarHorizontal *scrollbar_h;
	scenegraph::HBox *hbox;
	scenegraph::VBox *vbox;

	vec2 view_pos = vec2::ZERO;
	float scale = 1.0f;
	rect content_area = rect::EMPTY;
	void set_content(const rect &r);

	ScrollPad();

	void draw_recursive(Painter *p) override;

	bool on_mouse_wheel(const vec2 &d) override;
	bool on_key_down(int key) override;
	void update_geometry_recursive(const rect &target_area) override;

	void move_view(const vec2 &d);

	vec2 project(const vec2 &p) const;
	vec2 unproject(const vec2 &p) const;
	rect project(const rect &r) const;
	rect unproject(const rect &r) const;

	void _update_scrolling();

	template<class T>
	void connect_scrollable(Scrollable<T> *s) {
		s->pad = this;
	}
};

}
