/*
 * HuiPainter.h
 *
 *  Created on: 25.06.2013
 *      Author: michi
 */

#ifndef HUIPAINTER_H_
#define HUIPAINTER_H_

#include "../image/Painter.h"

class complex;
class rect;
class color;

namespace hui
{

class Window;

class Painter : public ::Painter
{
	public:
#ifdef HUI_API_GTK
	cairo_t *cr;
	PangoLayout *layout;
	PangoFontDescription *font_desc;
	cairo_surface_t *target_surface;
#endif
	Window *win;
	string id;
	string cur_font;
	bool cur_font_bold, cur_font_italic;
	bool mode_fill;
	float corner_radius;

	Painter();
	Painter(Panel *panel, const string &id);
	~Painter() override;

	void _cdecl set_color(const color &c) override;
	void _cdecl set_font(const string &font, float size, bool bold, bool italic) override;
	void _cdecl set_font_size(float size) override;
	void _cdecl set_antialiasing(bool enabled) override;
	void _cdecl set_line_width(float w) override;
	void _cdecl set_line_dash(const Array<float> &dash, float offset) override;
	void _cdecl set_roundness(float radius) override;
	void _cdecl set_fill(bool fill) override;
	void _cdecl set_clip(const rect &r) override;
	void _cdecl draw_point(float x, float y) override;
	void _cdecl draw_line(float x1, float y1, float x2, float y2) override;
	void _cdecl draw_lines(const Array<complex> &p) override;
	void _cdecl draw_polygon(const Array<complex> &p) override;
	void _cdecl draw_rect(float x1, float y1, float w, float h) override;
	void _cdecl draw_rect(const rect &r) override;
	void _cdecl draw_circle(float x, float y, float radius) override;
	void _cdecl draw_str(float x, float y, const string &str) override;
	float _cdecl get_str_width(const string &str) override;
	void _cdecl draw_image(float x, float y, const Image &image) override;
	void _cdecl draw_mask_image(float x, float y, const Image &image) override;
	rect _cdecl area() override;
	rect _cdecl clip() override;
};

Painter *start_image_paint(Image &im);
void end_image_paint(Image &im, ::Painter *p);

};

#endif /* HUIPAINTER_H_ */
