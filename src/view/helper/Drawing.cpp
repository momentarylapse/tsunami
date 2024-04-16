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
#include "../../lib/math/vec2.h"


color color_heat_map(float f) {
	static const color c[5] = {color(0,0,0,0), color(1,0,0,0.5f), color(1,1,0,0), color(1,1,1,0), color(1,1,1,1)};
	static const float stop[5] = {0.0f, 0.2f, 0.6f, 0.8f, 1.0f};

	f = clamp(f, 0.0f, 1.0f);

	for (int i=0; i<4; i++)
		if (f <= stop[i + 1])
			return color::interpolate(c[i], c[i+1], (f - stop[i]) / (stop[i+1] - stop[i]));
	return c[4];
}

void draw_str_centered(Painter* c, const vec2& pos, const string& str) {
	float w = c->get_str_width(str);
	c->draw_str(pos - vec2(w / 2, c->font_size * 0.6f), str);
}

rect get_boxed_str_rect(Painter *c, const vec2 &pos, const string &str) {
	vec2 size = c->get_str_size(str);
	size.y -= c->font_size * 1.0f; // ?!?
	return rect(pos.x-theme.CORNER_RADIUS, pos.x + size.x + theme.CORNER_RADIUS, pos.y-theme.CORNER_RADIUS, pos.y + size.y + theme.CORNER_RADIUS);
}

void draw_boxed_str(Painter *c, const vec2 &pos, const string &str, const color &col_text, const color &col_bg, TextAlign align) {
	rect r = get_boxed_str_rect(c, pos, str);
	float dx = r.width() / 2 - theme.CORNER_RADIUS;
	dx *= ((int)align - 1);
	r.x1 += dx;
	r.x2 += dx;
	c->set_color(col_bg);
	c->set_roundness(theme.CORNER_RADIUS);
	c->draw_rect(r);
	c->set_roundness(0);
	c->set_color(col_text);
	c->draw_str({pos.x + dx, pos.y}, str);
}

void draw_box(Painter *p, const rect &r, const color &bg) {
	p->set_color(bg);
	p->set_roundness(theme.CORNER_RADIUS);
	p->draw_rect(r);
	p->set_roundness(0);
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

float draw_str_constrained(Painter *p, const vec2 &pos, float w_max, const string &_str, TextAlign align) {
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
	p->draw_str({pos.x + dx, pos.y}, str);
	//p->set_font_size(fs0);
	return w;
}

void draw_cursor_hover(Painter *c, const string &msg, const vec2 &m, const rect &area) {
	//c->set_font("", -1, true, false);
	vec2 size = c->get_str_size(msg);
	float x = clamp(m.x - 20.0f, area.x1 + 2.0f, area.x2 - size.x);
	float y = clamp(m.y + 30, area.y1 + 2.0f, area.y2 - size.y - 5);
	draw_boxed_str(c, {x, y}, msg, theme.background, theme.text_soft1);
	//c->set_font("", -1, false, false);
}

void draw_arrow(Painter *p, const vec2 &a, const vec2 &b) {
	float l = (b-a).length();
	if (l < 0.0001f)
		return;
	vec2 dir = (b-a) / l;
	vec2 e = vec2(dir.y, -dir.x);
	float r = min(l, 18.0f);
	p->draw_line(a, b);
	p->draw_polygon({b, b - r * (dir + e*0.4f), b - r * (dir - e*0.4f)});
}


