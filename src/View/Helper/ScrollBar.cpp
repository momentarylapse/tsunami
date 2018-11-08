/*
 * ScrollBar.cpp
 *
 *  Created on: 22.08.2018
 *      Author: michi
 */

#include "ScrollBar.h"
#include "../AudioView.h"

void ScrollBar::drag_start(float mx, float my)
{
	mouse_offset = (my - area.y1) * content_size / area.height() - offset;
}

void ScrollBar::drag_update(float mx, float my)
{
	offset = (my - area.y1) * content_size / area.height() - mouse_offset;
	offset = max(min(offset, content_size - page_size), 0.0f);
}

void ScrollBar::draw(Painter *c, bool hover)
{
	c->set_color(AudioView::colors.background);
	c->draw_rect(area);
	c->set_color(hover ? AudioView::colors.text_soft1 : AudioView::colors.text_soft3);
	float d = 5;
	float f = min(page_size / content_size, 1.0f);
	float h = area.height() - 2*d;
	c->set_roundness(area.width()/2 - d);
	c->draw_rect(area.x1 + d, area.y1 + d +  offset / content_size * h, area.width() - 2*d, h * f);
	c->set_roundness(0);
}


void ScrollBar::set_area(const rect &r)
{
	area = r;
}

void ScrollBar::update(float page, float content)
{
	page_size = page;
	content_size = content;
	offset = max(min(offset, content_size - page_size), 0.0f);
}


