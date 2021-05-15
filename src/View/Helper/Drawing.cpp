/*
 * Drawing.cpp
 *
 *  Created on: May 15, 2021
 *      Author: michi
 */

#include "Drawing.h"
#include "../ColorScheme.h"
#include "../../lib/image/Painter.h"
#include "../../lib/math/rect.h"
#include "../../lib/math/math.h"




rect get_boxed_str_rect(Painter *c, float x, float y, const string &str) {
	float w = c->get_str_width(str);
	return rect(x-theme.CORNER_RADIUS, x + w + theme.CORNER_RADIUS, y-theme.CORNER_RADIUS, y + c->font_size + theme.CORNER_RADIUS);
}

void draw_boxed_str(Painter *c, float x, float y, const string &str, const color &col_text, const color &col_bg, TextAlign align) {
	rect r = get_boxed_str_rect(c, x, y, str);
	float dx = r.width() / 2 - theme.CORNER_RADIUS;
	dx *= ((int)align - 1);
	r.x1 += dx;
	r.x2 += dx;
	c->set_color(col_bg);
	c->set_roundness(theme.CORNER_RADIUS);
	c->draw_rect(r);
	c->set_roundness(0);
	c->set_color(col_text);
	c->draw_str(x + dx, y, str);
}

void draw_framed_box(Painter *p, const rect &r, const color &bg, const color &frame, float frame_width) {
	p->set_color(bg);
	p->set_roundness(theme.CORNER_RADIUS);
	p->draw_rect(r);

	p->set_fill(false);
	p->set_line_width(frame_width);
	p->set_color(frame);
	p->draw_rect(r);
	p->set_line_width(1);
	p->set_fill(true);
	p->set_roundness(0);
}

float draw_str_constrained(Painter *p, float x, float y, float w_max, const string &_str, TextAlign align) {
	string str = _str;
	float w = p->get_str_width(str);
	//float fs0 = p->font_size;
	if (w > w_max) {
		for (int i=str.num/2; i>=1; i--) {
			str = _str.head(i) + ".." + _str.tail(i);
			w = p->get_str_width(str);
			if (w < w_max)
				break;
		}
	}
	float dx = w/2 * ((int)align-1);
	p->draw_str(x + dx, y, str);
	//p->set_font_size(fs0);
	return w;
}


void draw_cursor_hover(Painter *c, const string &msg, float mx, float my, const rect &area) {
	//c->set_font("", -1, true, false);
	float w = c->get_str_width(msg);
	float x = clamp(mx - 20.0f, area.x1 + 2.0f, area.x2 - w);
	float y = clamp(my + 30, area.y1 + 2.0f, area.y2 - theme.FONT_SIZE - 5);
	draw_boxed_str(c, x, y, msg, theme.background, theme.text_soft1);
	//c->set_font("", -1, false, false);
}


