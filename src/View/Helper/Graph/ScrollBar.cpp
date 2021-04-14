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

void ScrollBar::set_callback(std::function<void()> cb) {
	callback = cb;
}

void ScrollBar::drag_update(float mx, float my) {
	if (horizontal)
		set_offset((mx - area.x1) * content_size / area.width() - mouse_offset);
	else
		set_offset((my - area.y1) * content_size / area.height() - mouse_offset);
}

void ScrollBar::set_offset(float _offset) {
	offset = _offset;
	if (constrained)
		offset = max(min(offset, content_size - page_size), 0.0f);
	if (callback)
		callback();
}

void ScrollBar::on_draw(Painter *c) {
	c->set_color(AudioView::colors.background);
	c->draw_rect(area);
	bool _hover = is_cur_hover();
	c->set_color(_hover ? AudioView::colors.text_soft1 : AudioView::colors.text_soft3);
	float d = 5;
	float f = page_size / content_size;
	if (constrained)
		f = min(f, 1.0f);
	if (horizontal) {
		float w = area.width() - 2*d;
		float ww = max(w * f, SCROLLBAR_MINIMUM_HANDLE_SIZE);
		c->set_roundness(area.height()/2 - d);
		c->draw_rect(area.x1 + d +  offset / content_size * w, area.y1 + d, ww, area.height() - 2*d);
	} else {
		float h = area.height() - 2*d;
		float hh = max(h * f, SCROLLBAR_MINIMUM_HANDLE_SIZE);
		c->set_roundness(area.width()/2 - d);
		c->draw_rect(area.x1 + d, area.y1 + d +  offset / content_size * h, area.width() - 2*d, hh);
	}
	c->set_roundness(0);
}


void ScrollBar::set_area(const rect &r) {
	area = r;
}

void ScrollBar::update(float page, float content) {
	page_size = page;
	content_size = content;
	if (constrained)
		offset = max(min(offset, content_size - page_size), 0.0f);
}

bool ScrollBar::on_left_button_down(float mx, float my) {
	if (horizontal)
		mouse_offset = (mx - area.x1) * content_size / area.width() - offset;
	else
		mouse_offset = (my - area.y1) * content_size / area.height() - offset;

	// outside?!?
	if (mouse_offset < 0 or mouse_offset > page_size) {
		mouse_offset = page_size / 2;
		drag_update(mx, my);
	}

	if (auto g = graph()) {
		g->mdp_prepare([=] {
			drag_update(g->mx, g->my);
		});
	}
	return true;
}

void ScrollBar::update_geometry(const rect& target_area) {
	Node::update_geometry(target_area);
	if (auto_hide)
		hidden = !(page_size < content_size);
}
