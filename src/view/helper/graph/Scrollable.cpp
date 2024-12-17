/*
 * Scrollable.cpp
 *
 *  Created on: Apr 13, 2021
 *      Author: michi
 */

#include "Scrollable.h"

#include <image/Painter.h>

#include "ScrollBar.h"
#include "../../../stuff/PerformanceMonitor.h"
#include "../../../lib/hui/Event.h"

namespace scenegraph {
void sort_nodes_up(Array<Node*> &nodes);
}

namespace tsunami {

ScrollPad::ScrollPad() : scenegraph::NodeRel({0,0},0,0) {
	align.horizontal = AlignData::Mode::Fill;
	align.vertical = AlignData::Mode::Fill;
	set_perf_name("scrollpad");
	f_local_to_parent = [this] (const vec2& p) {
		return project_to_parent(p);
	};
	f_parent_to_local = [this] (const vec2& p) {
		return unproject_to_local(p);
	};
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
	_update_scrollbars();
}


vec2 ScrollPad::project_to_parent(const vec2 &p) const {
	return (p - view_pos) * scale;
}

vec2 ScrollPad::unproject_to_local(const vec2 &p) const {
	return p / scale + view_pos;
}

rect ScrollPad::unproject_to_local(const rect &r) const {
	return rect(r.x1 / scale + view_pos.x, r.x2 / scale + view_pos.x, r.y1 / scale + view_pos.y, r.y2 / scale + view_pos.y);
}

bool ScrollPad::on_mouse_wheel(const vec2 &d) {
	move_view(d * 50);
	return true;
}

void ScrollPad::zoom(float factor) {
	vec2 m = area.center();
	if (is_cur_hover())
		m = cursor();
	vec2 mm = project_to_parent(m);
	scale = clamp(scale * factor, scale_min, scale_max);
	view_pos = m - mm / scale;

	out_changed();
}

bool ScrollPad::on_key_down(int key) {
	if (key == hui::KEY_UP)
		move_view({0, -30});
	if (key == hui::KEY_DOWN)
		move_view({0, 30});
	if (key == hui::KEY_LEFT)
		move_view({-30, 0});
	if (key == hui::KEY_RIGHT)
		move_view({30, 0});
	if (key == hui::KEY_PLUS)
		zoom(1.1f);
	if (key == hui::KEY_MINUS)
		zoom(1.0f / 1.1f);

	return false;
}

void ScrollPad::move_view(const vec2 &d) {
	view_pos += d / scale;
	_update_scrollbars();
	out_changed();
	request_redraw();
}

void ScrollPad::_update_scrollbars() {
	const auto v = unproject_to_local(area);

	scrollbar_h->set_view(v.x1, v.x2);
	scrollbar_h->set_content(content_area.x1, content_area.x2);

	scrollbar_v->set_view(v.y1, v.y2);
	scrollbar_v->set_content(content_area.y1, content_area.y2);

	//scrollbar_v->set_view_offset(-view_pos.y - content_area.y1);
	//scrollbar_v->update(area.height(), content_area.height());
}

void ScrollPad::update_geometry_recursive(const rect &target_area) {
	update_geometry(target_area);
	if (parent)
		z = parent->z + align.dz;

	for (auto *c: weak(children))
		if (c != scrollbar_h and c != scrollbar_v)
			c->update_geometry_recursive(unproject_to_local(area));

	scrollbar_h->update_geometry_recursive(area);
	scrollbar_v->update_geometry_recursive(area);
}

void ScrollPad::set_content(const rect &r) {
	content_area = r;
	_update_scrollbars();
	request_redraw();
}

void ScrollPad::draw_recursive(Painter *p) {
	if (hidden)
		return;
	PerformanceMonitor::start_busy(perf_channel);

	rect clip_before = p->clip();
	if (clip)
		p->set_clip(area);

	on_draw(p);

	float m[4] = {scale, 0, 0, scale};
	p->set_transform(m, -view_pos*scale);

	auto nodes = weak(children);
	sort_nodes_up(nodes);
	for (auto *c: nodes)
		if (!c->hidden and c != vbox)
			c->draw_recursive(p);

	float m_id[4] = {1, 0, 0, 1};
	p->set_transform(m_id, vec2::ZERO);

	vbox->draw_recursive(p);

	if (clip)
		p->set_clip(clip_before);
	PerformanceMonitor::end_busy(perf_channel);
}

}
