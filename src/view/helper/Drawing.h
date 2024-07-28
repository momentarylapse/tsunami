/*
 * Drawing.h
 *
 *  Created on: May 15, 2021
 *      Author: michi
 */

#pragma once

class Painter;
class rect;
class vec2;
class string;
class color;

color color_heat_map(float f);

enum class TextAlign {
	Left = 1,
	Center = 0,
	Right = -1
};


rect get_boxed_str_rect(Painter *c, const vec2 &pos, const string &str);
void draw_boxed_str(Painter *c, const vec2 &pos, const string &str, const color &col_text, const color &col_bg, TextAlign align=TextAlign::Left);
void draw_box(Painter *p, const rect &r, const color &bg);
void draw_framed_box(Painter *p, const rect &r, const color &bg, const color &frame, float frame_width);
float draw_str_constrained(Painter *p, const vec2 &pos, float w_max, const string &str, TextAlign align=TextAlign::Left);
void draw_str_centered(Painter* c, const vec2& pos, const string& str);
void draw_cursor_hover(Painter *c, const string &msg, const vec2 &m, const rect &area);
void draw_arrow(Painter *p, const vec2 &a, const vec2 &b);
