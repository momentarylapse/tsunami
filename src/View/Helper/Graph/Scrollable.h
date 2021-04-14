/*
 * Scrollable.h
 *
 *  Created on: Apr 13, 2021
 *      Author: michi
 */

#pragma once

#include "Node.h"
#include "../../../lib/math/complex.h"

//namespace scenegraph {
	class ScrollBar;
	class ScrollBarHorizontal;
//}

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

	complex view_pos = complex(0,0);
	rect content_area = rect::EMPTY;
	void set_content(const rect &r);

	ScrollPad();

	bool on_mouse_wheel(float dx, float dy) override;
	void update_geometry_recursive(const rect &target_area) override;

	void move_view(float dx, float dy);

	complex project(const complex &p);
	complex unproject(const complex &p);

	void _update_scrolling();

	template<class T>
	void connect_scrollable(Scrollable<T> *s) {
		s->pad = this;
	}
};
