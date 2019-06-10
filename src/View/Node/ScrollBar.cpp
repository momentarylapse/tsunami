/*
 * ScrollBar.cpp
 *
 *  Created on: 22.08.2018
 *      Author: michi
 */

#include "ScrollBar.h"
#include "../AudioView.h"

ScrollBar::ScrollBar(ViewNode *parent) : ViewNode(parent, 0, 0, AudioView::SCROLLBAR_WIDTH, 100) {
	//node_width = AudioView::SCROLLBAR_WIDTH;
	//node_height = 100;
	//node_align_right = true;
	z += 10;
}
ScrollBarHorizontal::ScrollBarHorizontal(ViewNode *parent) : ScrollBar(parent) {
	align.h = AudioView::SCROLLBAR_WIDTH;
	align.w = 100;
	//node_align_right = true;
	horizontal = true;
}

void ScrollBar::set_callback(std::function<void()> cb) {
	callback = cb;
}

void ScrollBar::drag_update(float mx, float my) {
	if (horizontal)
		offset = (mx - area.x1) * content_size / area.width() - mouse_offset;
	else
		offset = (my - area.y1) * content_size / area.height() - mouse_offset;
	if (constrained)
		offset = max(min(offset, content_size - page_size), 0.0f);
	if (callback)
		callback();
}

void ScrollBar::draw(Painter *c) {
	c->set_color(AudioView::colors.background);
	c->draw_rect(area);
	bool _hover = view_hover();
	c->set_color(_hover ? AudioView::colors.text_soft1 : AudioView::colors.text_soft3);
	float d = 5;
	float f = page_size / content_size;
	if (constrained)
		f = min(f, 1.0f);
	if (horizontal) {
		float w = area.width() - 2*d;
		c->set_roundness(area.height()/2 - d);
		c->draw_rect(area.x1 + d +  offset / content_size * w, area.y1 + d, w * f, area.height() - 2*d);
	} else {
		float h = area.height() - 2*d;
		c->set_roundness(area.width()/2 - d);
		c->draw_rect(area.x1 + d, area.y1 + d +  offset / content_size * h, area.width() - 2*d, h * f);
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

bool ScrollBar::on_left_button_down() {
	if (horizontal)
		mouse_offset = (view->mx - area.x1) * content_size / area.width() - offset;
	else
		mouse_offset = (view->my - area.y1) * content_size / area.height() - offset;
	view->mdp_prepare([=]{
		drag_update(view->mx, view->my);
	});
	return true;
}
