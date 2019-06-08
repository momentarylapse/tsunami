/*
 * ScrollBar.cpp
 *
 *  Created on: 22.08.2018
 *      Author: michi
 */

#include "ScrollBar.h"
#include "../AudioView.h"

//ScrollBar::ScrollBar(ViewNode *parent) : ViewNode(parent, 0, 0, AudioView::SCROLLBAR_WIDTH, 100) {}
ScrollBar::ScrollBar(AudioView *view, std::function<void()> cb) : ViewNode(view) {
	node_width = AudioView::SCROLLBAR_WIDTH;
	node_height = 100;
	z = 20;
	callback = cb;
}

void ScrollBar::drag_start(float mx, float my) {
	mouse_offset = (my - area.y1) * content_size / area.height() - offset;
	scrolling = true;
}

void ScrollBar::drag_update(float mx, float my) {
	offset = (my - area.y1) * content_size / area.height() - mouse_offset;
	offset = max(min(offset, content_size - page_size), 0.0f);
	callback();
}

void ScrollBar::draw(Painter *c) {
	c->set_color(AudioView::colors.background);
	c->draw_rect(area);
	// FIXME ...
	bool _hover = hover();//(view->hover.node == this);
	c->set_color(_hover ? AudioView::colors.text_soft1 : AudioView::colors.text_soft3);
	float d = 5;
	float f = min(page_size / content_size, 1.0f);
	float h = area.height() - 2*d;
	c->set_roundness(area.width()/2 - d);
	c->draw_rect(area.x1 + d, area.y1 + d +  offset / content_size * h, area.width() - 2*d, h * f);
	c->set_roundness(0);
}


void ScrollBar::set_area(const rect &r) {
	area = r;
}

void ScrollBar::update(float page, float content) {
	page_size = page;
	content_size = content;
	offset = max(min(offset, content_size - page_size), 0.0f);
}

void ScrollBar::on_left_button_down() {
	drag_start(view->mx, view->my);
}

void ScrollBar::on_left_button_up() {
	scrolling = false;
}

void ScrollBar::on_mouse_move() {
	if (scrolling)
		drag_update(view->mx, view->my);
}
