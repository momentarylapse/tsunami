/*
 * Scrollable.cpp
 *
 *  Created on: Apr 13, 2021
 *      Author: michi
 */

#include "Scrollable.h"
#include "ScrollBar.h"
#include "../../../lib/hui/Event.h"


ScrollPad::ScrollPad() : scenegraph::NodeRel(0,0,0,0) {
	align.horizontal = AlignData::Mode::FILL;
	align.vertical = AlignData::Mode::FILL;
	set_perf_name("scrollpad");
	scrollbar_h = new ScrollBarHorizontal();
	scrollbar_v = new ScrollBar();
	scrollbar_h->set_callback([&] (float offset) {
		view_pos.x = offset;
		move_view(0,0);
	});
	scrollbar_v->set_callback([&] (float offset) {
		view_pos.y = offset;
		move_view(0,0);
	});
	scrollbar_h->constrained = false;
	scrollbar_v->constrained = false;
	scrollbar_h->auto_hide = true;
	scrollbar_v->auto_hide = true;
	auto hbox = new scenegraph::HBox();
	auto vbox = new scenegraph::VBox();
	vbox->add_child(hbox);
	vbox->add_child(scrollbar_h);
	hbox->add_child(scrollbar_v);
	add_child(vbox);
	_update_scrolling();
}


// coord -> screen
complex ScrollPad::project(const complex &p) {
	return p - view_pos;
}

// screen -> coord
complex ScrollPad::unproject(const complex &p) {
	return p + view_pos;
}

bool ScrollPad::on_mouse_wheel(float dx, float dy) {
	move_view(dx * 10, dy * 10);
	return true;
}

bool ScrollPad::on_key(int key) {
	if (key == hui::KEY_UP)
		move_view(0, -10);
	if (key == hui::KEY_DOWN)
		move_view(0, 10);
	if (key == hui::KEY_LEFT)
		move_view(-10, 0);
	if (key == hui::KEY_RIGHT)
		move_view(10, 0);
	return false;
}

void ScrollPad::move_view(float dx, float dy) {
	view_pos += complex(dx, dy);
	_update_scrolling();
	request_redraw();
}

void ScrollPad::_update_scrolling() {
	scrollbar_h->set_view(view_pos.x, view_pos.x + area.width());
	scrollbar_h->set_content(content_area.x1, content_area.x2);

	scrollbar_v->set_view(view_pos.y, view_pos.y + area.height());
	scrollbar_v->set_content(content_area.y1, content_area.y2);

	//scrollbar_v->set_view_offset(-view_pos.y - content_area.y1);
	//scrollbar_v->update(area.height(), content_area.height());
}

void ScrollPad::update_geometry_recursive(const rect &target_area) {
	Node::update_geometry_recursive(target_area);
	for (auto c: collect_children(false))
		if (c != scrollbar_h and c != scrollbar_v) {
			c->area.x1 -= view_pos.x;
			c->area.x2 -= view_pos.x;
			c->area.y1 -= view_pos.y;
			c->area.y2 -= view_pos.y;
		}
}

void ScrollPad::set_content(const rect &r) {
	content_area = r;
	_update_scrolling();
	request_redraw();
}