/*
 * HuiPainter.h
 *
 *  Created on: 25.06.2013
 *      Author: michi
 */

#ifndef HUIPAINTER_H_
#define HUIPAINTER_H_

#include "../image/Painter.h"

typedef struct _PangoLayout PangoLayout;
typedef struct _PangoFontDescription PangoFontDescription;

struct vec2;
struct rect;
struct color;

typedef struct _cairo_surface cairo_surface_t;
typedef struct _cairo cairo_t;

namespace hui {

class Window;
class Panel;

class Painter : public ::Painter {
	public:
#ifdef HUI_API_GTK
	cairo_t *cr;
	PangoLayout *layout;
	PangoFontDescription *font_desc;
	cairo_surface_t *target_surface;
	float _initial_offset_x, _initial_offset_y;
#endif
	Window *win;
	string id;
	string cur_font;
	bool cur_font_bold, cur_font_italic;
	bool mode_fill;
	float corner_radius;

	Painter(cairo_surface_t *surf, cairo_t *cr, int width, int height);
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
	void _cdecl draw_point(const vec2 &p) override;
	void _cdecl draw_line(const vec2 &a, const vec2 &b) override;
	void _cdecl draw_lines(const Array<vec2> &p) override;
	void _cdecl draw_polygon(const Array<vec2> &p) override;
	void _cdecl draw_rect(const rect &r) override;
	void _cdecl draw_circle(const vec2 &c, float radius) override;
	void _cdecl draw_str(const vec2 &p, const string &str) override;
	vec2 _cdecl get_str_size(const string &str) override;
	void _cdecl draw_image(const vec2 &d, const Image *image) override;
	void _cdecl draw_mask_image(const vec2 &d, const Image *image) override;
	void _cdecl set_transform(float rot[], const vec2 &offset) override;
	void _cdecl set_option(const string &key, const string &value) override;
	rect _cdecl clip() const override;
};

};

#endif /* HUIPAINTER_H_ */
