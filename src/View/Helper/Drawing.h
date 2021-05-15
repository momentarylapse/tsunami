/*
 * Drawing.h
 *
 *  Created on: May 15, 2021
 *      Author: michi
 */

#pragma once

class Painter;
class rect;
class string;
class color;

enum class TextAlign {
	LEFT = 1,
	CENTER = 0,
	RIGHT = -1
};


rect get_boxed_str_rect(Painter *c, float x, float y, const string &str);
void draw_boxed_str(Painter *c, float x, float y, const string &str, const color &col_text, const color &col_bg, TextAlign align=TextAlign::LEFT);
void draw_framed_box(Painter *p, const rect &r, const color &bg, const color &frame, float frame_width);
float draw_str_constrained(Painter *p, float x, float y, float w_max, const string &str, TextAlign align=TextAlign::LEFT);
void draw_cursor_hover(Painter *c, const string &msg, float mx, float my, const rect &area);
