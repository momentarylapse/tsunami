/*
 * Scrollable.cpp
 *
 *  Created on: Apr 13, 2021
 *      Author: michi
 */

#include "Scrollable.h"
#include "ScrollBar.h"
#include "../../../lib/hui/Event.h"


ScrollPad::ScrollPad() : scenegraph::NodeRel({0,0},0,0) {
	align.horizontal = AlignData::Mode::FILL;
	align.vertical = AlignData::Mode::FILL;
	set_perf_name("scrollpad");
	scrollbar_h = new ScrollBarHorizontal();
	scrollbar_v = new ScrollBar();
	scrollbar_h->out_offset >> create_data_sink<float>([&] (float offset) {
		view_pos.x = offset;
		move_view({0,0});
	});
	scrollbar_v->out_offset >> create_data_sink<float>([&] (float offset) {
		view_pos.y = offset;
		move_view({0,0});
	});
	scrollbar_h->constrained = false;
	scrollbar_v->constrained = false;
	scrollbar_h->auto_hide = true;
	scrollbar_v->auto_hide = true;
	hbox = new scenegraph::HBox();
	vbox = new scenegraph::VBox();
	vbox->add_child(hbox);
	vbox->add_child(scrollbar_h);
	hbox->add_child(scrollbar_v);
	add_child(vbox);
	_update_scrolling();
}


// coord -> screen
vec2 ScrollPad::project(const vec2 &p) {
	return p - view_pos;
}

// screen -> coord
vec2 ScrollPad::unproject(const vec2 &p) {
	return p + view_pos;
}

bool ScrollPad::on_mouse_wheel(const vec2 &d) {
	move_view(d * 10);
	return true;
}

bool ScrollPad::on_key(int key) {
	if (key == hui::KEY_UP)
		move_view({0, -10});
	if (key == hui::KEY_DOWN)
		move_view({0, 10});
	if (key == hui::KEY_LEFT)
		move_view({-10, 0});
	if (key == hui::KEY_RIGHT)
		move_view({10, 0});
	return false;
}

void ScrollPad::move_view(const vec2 &d) {
	view_pos += d;
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
		if (c != scrollbar_h and c != scrollbar_v and c != hbox and c != vbox) {
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
