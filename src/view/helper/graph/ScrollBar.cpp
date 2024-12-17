/*
 * ScrollBar.cpp
 *
 *  Created on: 22.08.2018
 *      Author: michi
 */

#include "ScrollBar.h"
#include "SceneGraph.h"
#include "../../ColorScheme.h"
#include "../../../lib/image/Painter.h"

namespace tsunami {

// TODO who owns the view_offset? -> callback needed/not?
// => WE (ScrollBar) own the offset!

ScrollBar::ScrollBar() {
	align.vertical = AlignData::Mode::Fill;
	align.horizontal = AlignData::Mode::Left;
	align.w = theme.SCROLLBAR_WIDTH;
	align.dz = 120;
	set_perf_name("scrollbar");
}
ScrollBarHorizontal::ScrollBarHorizontal() : ScrollBar() {
	align.vertical = AlignData::Mode::Bottom;
	align.horizontal = AlignData::Mode::Fill;
	align.h = theme.SCROLLBAR_WIDTH;
	horizontal = true;
}

void ScrollBar::drag_update(const vec2 &m) {
	//unproject(mx) - view_offset = mouse_offset
	if (horizontal)
		set_view_offset(unproject(m.x) - mouse_offset);
	else
		set_view_offset(unproject(m.y) - mouse_offset);
	out_offset(view_offset);
}

void ScrollBar::on_draw(Painter *c) {
	c->set_color(theme.background);
	c->draw_rect(area);
	bool _hover = is_cur_hover();
	c->set_color(_hover ? theme.text_soft1 : theme.text_soft3);
	float f = view_size / content_size;
	if (constrained)
		f = min(f, 1.0f);
	const float D = theme.SCROLLBAR_D;
	if (horizontal) {
		float w = area.width() - 2*D;
		float ww = max(w * f, theme.SCROLLBAR_MINIMUM_HANDLE_SIZE);
		c->set_roundness(area.height()/2 - D);
		float x0 = project(view_offset);
		c->draw_rect(rect(x0, x0 + ww, area.y1 + D, area.y2 - D));
	} else {
		float h = area.height() - 2*D;
		float hh = max(h * f, theme.SCROLLBAR_MINIMUM_HANDLE_SIZE);
		c->set_roundness(area.width()/2 - D);
		float y0 = project(view_offset);
		c->draw_rect(rect(area.x1 + D, area.x2 - D, y0, y0 + hh));
	}
	c->set_roundness(0);
}

void ScrollBar::set_view(float start, float end) {
	float size = end - start;
	if (constrained)
		start = max(min(start, content_offset + content_size - size), content_offset);
	if ((start != view_offset) or (view_size != size)) {
		view_offset = start;
		view_size = size;
		request_redraw();
	}
}

void ScrollBar::set_view(const Range &r) {
	set_view(r.start(), r.end());
}

void ScrollBar::set_view_offset(float offset) {
	if (constrained)
		offset = max(min(offset, content_offset + content_size - view_size), content_offset);
	if (offset != view_offset) {
		view_offset = offset;
		request_redraw();
		out_offset(view_offset);
	}
}

void ScrollBar::move_view(float d) {
	set_view_offset(view_offset + d);
}

void ScrollBar::set_view_size(float size) {
	if (size != view_size) {
		view_size = size;
		request_redraw();
	}
}

void ScrollBar::set_content(float start, float end) {
	int size = max(end - start, view_size/100);
	if ((start != content_offset) or (size != content_size)) {
		content_offset = start;
		content_size = size;
		request_redraw();
	}
}

void ScrollBar::set_content(const Range &r) {
	set_content(r.start(), r.end());
}

float ScrollBar::get_view_offset() const {
	return view_offset;
}

bool ScrollBar::on_left_button_down(const vec2 &m) {
	if (horizontal)
		mouse_offset = unproject(m.x) - view_offset;
	else
		mouse_offset = unproject(m.y) - view_offset;

	// outside?!?
	if (mouse_offset < 0 or mouse_offset > view_size) {
		// make the cursor the middle of the scroller
		mouse_offset = view_size / 2;
		drag_update(m);
	}

	if (auto g = graph()) {
		g->mdp_prepare([this] (const vec2& m) {
			drag_update(m);
		}, m);
	}
	return true;
}

float ScrollBar::project(float x) const {
	const float D = theme.SCROLLBAR_D;
	if (horizontal)
		return area.x1 + D + (x - content_offset) / content_size * (area.width() - 2*D);
	else
		return area.y1 + D + (x - content_offset) / content_size * (area.height() - 2*D);
}

float ScrollBar::unproject(float x) const {
	const float D = theme.SCROLLBAR_D;
	if (horizontal)
		return (x - area.x1 - D) * content_size / (area.width() - 2*D) + content_offset;
	else
		return (x - area.y1 - D) * content_size / (area.height() - 2*D) + content_offset;
}

void ScrollBar::update_geometry(const rect& target_area) {
	Node::update_geometry(target_area);
	if (auto_hide)
		set_hidden(content_covered_by_view());
}


bool ScrollBar::content_covered_by_view() const {
	return (view_offset <= content_offset) and (view_offset + view_size >= content_offset + content_size);
}

}
