/*
 * ScrollBar.cpp
 *
 *  Created on: 22.08.2018
 *      Author: michi
 */

#include "ScrollBar.h"
#include "SceneGraph.h"

#include "../../AudioView.h"

const float SCROLLBAR_MINIMUM_HANDLE_SIZE = 15.0f;
const float D = 5;

// TODO who owns the view_offset? -> callback needed/not?

ScrollBar::ScrollBar() {
	align.vertical = AlignData::Mode::FILL;
	align.horizontal = AlignData::Mode::LEFT;
	align.w = AudioView::SCROLLBAR_WIDTH;
	align.dz = 120;
}
ScrollBarHorizontal::ScrollBarHorizontal() : ScrollBar() {
	align.vertical = AlignData::Mode::BOTTOM;
	align.horizontal = AlignData::Mode::FILL;
	align.h = AudioView::SCROLLBAR_WIDTH;
	horizontal = true;
}

void ScrollBar::set_callback(std::function<void(float)> f) {
	cb_update_view = f;
}

void ScrollBar::drag_update(float mx, float my) {
	//unproject(mx) - view_offset = mouse_offset
	if (horizontal)
		set_view_offset(unproject(mx) - mouse_offset);
	else
		set_view_offset(unproject(my) - mouse_offset);
	if (cb_update_view)
		cb_update_view(view_offset);
}

/*void ScrollBar::set_offset(float _offset) {
	offset = _offset;
	if (constrained)
		offset = max(min(offset, content_size - page_size), 0.0f);
	if (callback)
		callback();
}*/

void ScrollBar::on_draw(Painter *c) {
	c->set_color(AudioView::colors.background);
	c->draw_rect(area);
	bool _hover = is_cur_hover();
	c->set_color(_hover ? AudioView::colors.text_soft1 : AudioView::colors.text_soft3);
	float f = view_size / content_size;
	if (constrained)
		f = min(f, 1.0f);
	if (horizontal) {
		float w = area.width() - 2*D;
		float ww = max(w * f, SCROLLBAR_MINIMUM_HANDLE_SIZE);
		c->set_roundness(area.height()/2 - D);
		float x0 = project(view_offset);
		c->draw_rect(rect(x0, x0 + ww, area.y1 + D, area.y2 - D));
	} else {
		float h = area.height() - 2*D;
		float hh = max(h * f, SCROLLBAR_MINIMUM_HANDLE_SIZE);
		c->set_roundness(area.width()/2 - D);
		float y0 = project(view_offset);
		c->draw_rect(rect(area.x1 + D, area.x2 - D, y0, y0 + hh));
	}
	c->set_roundness(0);
}


/*void ScrollBar::set_area(const rect &r) {
	area = r;
}*/


void ScrollBar::set_view(float start, float end) {
	view_offset = start;
	view_size = end - start;
	if (constrained)
		view_offset = max(min(view_offset, content_offset + content_size - view_size), content_offset);
	request_redraw();
}

void ScrollBar::set_view(const Range &r) {
	set_view(r.start(), r.end());
}

void ScrollBar::set_view_offset(float offset) {
	view_offset = offset;
	if (constrained)
		view_offset = max(min(view_offset, content_offset + content_size - view_size), content_offset);
	request_redraw();
}

void ScrollBar::move_view(float d) {
	set_view_offset(view_offset + d);
	if (cb_update_view)
		cb_update_view(view_offset);
}

void ScrollBar::set_view_size(float size) {
	view_size = size;
	request_redraw();
}

void ScrollBar::set_content(float start, float end) {
	content_offset = start;
	content_size = max(end - start, view_size/100);
	request_redraw();
}

void ScrollBar::set_content(const Range &r) {
	set_content(r.start(), r.end());
}

/*void ScrollBar::update(float page, float content) {
	page_size = page;
	content_size = max(content, page/100);
	if (constrained)
		offset = max(min(offset, content_size - page_size), 0.0f);
}*/

float ScrollBar::get_view_offset() const {
	return view_offset;
}

/*float ScrollBar::get_size() const {
	return view_size / content_size;
}*/

bool ScrollBar::on_left_button_down(float mx, float my) {
	if (horizontal)
		mouse_offset = unproject(mx) - view_offset;
	else
		mouse_offset = unproject(my) - view_offset;

	// outside?!?
	if (mouse_offset < 0 or mouse_offset > view_size) {
		// make the cursor the middle of the scroller
		mouse_offset = view_size / 2;
		drag_update(mx, my);
	}

	if (auto g = graph()) {
		g->mdp_prepare([=] {
			drag_update(g->mx, g->my);
		});
	}
	return true;
}

float ScrollBar::project(float x) const {
	if (horizontal)
		return area.x1 + D + (x - content_offset) / content_size * (area.width() - 2*D);
	else
		return area.y1 + D + (x - content_offset) / content_size * (area.height() - 2*D);
}

float ScrollBar::unproject(float x) const {
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
