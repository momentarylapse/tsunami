/*
 * Scrollable.cpp
 *
 *  Created on: Apr 13, 2021
 *      Author: michi
 */

#include "Scrollable.h"
#include "ScrollBar.h"

ScrollPad::ScrollPad() : scenegraph::NodeRel(0,0,0,0) {
	align.horizontal = AlignData::Mode::FILL;
	align.vertical = AlignData::Mode::FILL;
	scrollbar_h = new ScrollBarHorizontal();
	scrollbar_v = new ScrollBar();
	//auto hbox = new scenegraph::HBox();
}


complex ScrollPad::project(const complex &p) {
	return p + view_pos;
}
complex ScrollPad::unproject(const complex &p) {
	return p - view_pos;
}

bool ScrollPad::on_mouse_wheel(float dx, float dy) {
	move_view(dx, dy);
	return true;
}

void ScrollPad::move_view(float dx, float dy) {
	view_pos += complex(dx, dy);
	//graph()->redraw();
}

void ScrollPad::update_geometry_recursive(const rect &target_area) {
	Node::update_geometry_recursive(target_area);
	for (auto c: collect_children(false))
		if (c != scrollbar_h and c != scrollbar_v) {
			c->area.x1 += view_pos.x;
			c->area.x2 += view_pos.x;
			c->area.y1 += view_pos.y;
			c->area.y2 += view_pos.y;
		}
}
