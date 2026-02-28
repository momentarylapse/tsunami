/*
 * HuiPainterGtk.cpp
 *
 *  Created on: 25.06.2013
 *      Author: michi
 */

#include "hui.h"
#include "../math/vec2.h"
#include "../math/rect.h"
#include "Controls/Control.h"
#include "Controls/ControlDrawingArea.h"
#include "Painter.h"
#if __has_include("../image/image.h")
#include "../image/image.h"
#define HAS_IMAGE
#endif

#include <gtk/gtk.h>

namespace hui {

bool color_painter_linear = false;

#ifdef HUI_API_GTK

Painter::Painter(cairo_surface_t *_surf, cairo_t *_cr, int w, int h) {
	win = nullptr;
	target_surface = _surf;
	cr = _cr;
	layout = nullptr;
	if (cr)
		layout = pango_cairo_create_layout(cr);

	font_desc = nullptr;
	width = w;
	height = h;
	mode_fill = true;
	cur_font_bold = false;
	cur_font_italic = false;
	cur_font = "";
	font_size = 16;
	corner_radius = 0;
	_initial_offset_x = 0;
	_initial_offset_y = 0;

	set_font("Sans", 16, false, false);
}

Painter::Painter(Panel *panel, const string &_id) : Painter(nullptr, nullptr, 0, 0) {
	win = panel->win;
	id = _id;
	if (auto c = dynamic_cast<ControlDrawingArea*>(panel->_get_control_(id))) {
		cr = (cairo_t*)c->cur_cairo;
//		cr = gdk_cairo_create(gtk_widget_get_window(c->widget));
		layout = pango_cairo_create_layout(cr);

		//gdk_drawable_get_size(gtk_widget_get_window(c->widget), &hui_drawing_context.width, &hui_drawing_context.height);
#if GTK_CHECK_VERSION(4,0,0)
		width = gtk_widget_get_width(c->widget);
		height = gtk_widget_get_height(c->widget);
#else
		width = gdk_window_get_width(gtk_widget_get_window(c->widget));
		height = gdk_window_get_height(gtk_widget_get_window(c->widget));
#endif

		// might be translated already
		cairo_matrix_t m;
		cairo_get_matrix(cr, &m);
		_initial_offset_x = m.x0;
		_initial_offset_y = m.y0;

		set_font("Sans", 16, false, false);
	}
}

Painter::~Painter() {
	if (!cr)
		return;

	if (layout)
		g_object_unref(layout);
	if (font_desc)
		pango_font_description_free(font_desc);
	if (target_surface)
		cairo_surface_destroy(target_surface);
	if (!win)
		cairo_destroy(cr);
}

void Painter::set_color(const color &c) {
	if (!cr)
		return;
	if (color_painter_linear) {
		auto l = c.linear_to_srgb();
		cairo_set_source_rgba(cr, l.r, l.g, l.b, l.a);
	} else {
		cairo_set_source_rgba(cr, c.r, c.g, c.b, c.a);
	}
}

void Painter::set_line_width(float w) {
	if (!cr)
		return;
	cairo_set_line_width(cr, w);
}

void Painter::set_line_dash(const Array<float> &dash, float offset) {
	if (!cr)
		return;
	Array<double> d;
	for (int i=0; i<dash.num; i++)
		d.add(dash[i]);
	cairo_set_dash(cr, (double*)d.data, d.num, offset);
}

void Painter::set_roundness(float radius) {
	corner_radius = radius;
}

void Painter::set_option(const string &key, const string &value) {
	if (key == "line-join") {
		if (value == "miter")
			cairo_set_line_join(cr, CAIRO_LINE_JOIN_MITER);
		else if (value == "round")
			cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
		else if (value == "bevel")
			cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);
	} else if (key == "line-cap") {
		if (value == "butt")
			cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
		else if (value == "round")
			cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
		else if (value == "square")
			cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
	}
}

void Painter::set_clip(const rect &r) {
	cairo_reset_clip(cr);
	cairo_rectangle(cr, max(r.x1, 0.0f), max(r.y1, 0.0f), min(r.width(), (float)width), min(r.height(), (float)height));
	cairo_clip(cr);
}

rect Painter::clip() const {
	if (!cr)
		return rect::EMPTY;
	double x1, x2, y1, y2;
	cairo_clip_extents(cr, &x1, &y1, &x2, &y2);
	return rect((float)x1, (float)x2, (float)y1, (float)y2);
}

void Painter::draw_point(const vec2 &p) {
	if (!cr)
		return;
}

void Painter::draw_line(const vec2 &a, const vec2 &b) {
	if (!cr)
		return;
	cairo_move_to(cr, a.x + 0.5f, a.y + 0.5f);
	cairo_line_to(cr, b.x + 0.5f, b.y + 0.5f);
	cairo_stroke(cr);
}

void Painter::draw_lines(const Array<vec2> &p) {
	if (!cr)
		return;
	if (p.num == 0)
		return;
	//cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
	//cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
	cairo_move_to(cr, p[0].x, p[0].y);
	for (int i=1; i<p.num; i++)
		cairo_line_to(cr, p[i].x, p[i].y);
	cairo_stroke(cr);
}

void Painter::draw_polygon(const Array<vec2> &p) {
	if (!cr)
		return;
	if (p.num == 0)
		return;
	cairo_move_to(cr, p[0].x, p[0].y);
	for (int i=1; i<p.num; i++)
		cairo_line_to(cr, p[i].x, p[i].y);
	cairo_close_path(cr);

	if (mode_fill)
		cairo_fill(cr);
	else
		cairo_stroke(cr);
}

// y = (typically) top of text
void Painter::draw_str(const vec2 &p, const string &str) {
	if (!cr)
		return;
	pango_cairo_update_layout(cr, layout);
	pango_layout_set_text(layout, (char*)str.data, str.num);
	float dy = (float)pango_layout_get_baseline(layout) / 1000.0f;
	cairo_move_to(cr, p.x, p.y - dy + font_size);

	if (mode_fill) {
		pango_cairo_show_layout(cr, layout);
	} else {
		pango_cairo_layout_path(cr, layout);
		cairo_stroke(cr);
	}

	//cairo_show_text(cr, str);
}

vec2 Painter::get_str_size(const string &str) {
	if (!cr)
		return {0,0};
	pango_cairo_update_layout(cr, layout);
	pango_layout_set_text(layout, (char*)str.data, str.num);
	int w, h;
	pango_layout_get_size(layout, &w, &h);

	return {(float)w / 1000.0f, (float)h / 1000.0f};
}

void Painter::draw_rect(const rect &r) {
	if (!cr)
		return;
	if (corner_radius > 0) {
		float R = corner_radius;
		cairo_new_sub_path(cr);
		cairo_arc(cr, r.x2 - R, r.y1 + R, R, -pi/2, 0);
		cairo_arc(cr, r.x2 - R, r.y2 - R, R, 0, pi/2);
		cairo_arc(cr, r.x1 + R, r.y2 - R, R, pi/2, pi);
		cairo_arc(cr, r.x1 + R, r.y1 + R, R, pi, pi*3/2);
		cairo_close_path(cr);
	} else {
		cairo_rectangle(cr, r.x1, r.y1, r.width(), r.height());
	}

	if (mode_fill)
		cairo_fill(cr);
	else
		cairo_stroke(cr);
}

void Painter::draw_circle(const vec2 &c, float radius) {
	if (!cr)
		return;
	cairo_new_sub_path(cr);
	cairo_arc(cr, c.x, c.y, radius, 0, 2 * pi);

	if (mode_fill)
		cairo_fill(cr);
	else
		cairo_stroke(cr);
}

void Painter::draw_image(const vec2 &d, const Image *image) {
#ifdef HAS_IMAGE
	if (!cr)
		return;
	image->set_mode(Image::Mode::BGRA);
	cairo_pattern_t *p = cairo_get_source(cr);
	cairo_pattern_reference(p);
	cairo_surface_t *img = cairo_image_surface_create_for_data((unsigned char*)image->data.data,
                                                         CAIRO_FORMAT_ARGB32,
                                                         image->width,
                                                         image->height,
	    image->width * 4);

	cairo_set_source_surface(cr, img, d.x, d.y);
	cairo_paint(cr);
	cairo_surface_destroy(img);
	cairo_set_source(cr, p);
	cairo_pattern_destroy(p);
#endif
}

void Painter::draw_mask_image(const vec2 &d, const Image *image) {
#ifdef HAS_IMAGE
	if (!cr)
		return;
	image->set_mode(Image::Mode::BGRA);
	cairo_surface_t *img = cairo_image_surface_create_for_data((unsigned char*)image->data.data,
                                                         CAIRO_FORMAT_ARGB32,
                                                         image->width,
                                                         image->height,
	    image->width * 4);

	cairo_mask_surface(cr, img, d.x, d.y);
	cairo_surface_destroy(img);
#endif
}

void Painter::set_font(const string &font, float size, bool bold, bool italic) {
	if (!cr)
		return;
	//cairo_select_font_face(cr, "serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	//cairo_set_font_size(cr, size);
	if (font.num > 0)
		cur_font = font;
	if (size > 0)
		font_size = size;
	cur_font_bold = bold;
	cur_font_italic = italic;



	string f = cur_font;
	if (cur_font_bold)
		f += " Bold";
	if (cur_font_italic)
		f += " Italic";
	f += " " + f2s(font_size, 2);
	if (font_desc)
		pango_font_description_free(font_desc);
	font_desc = pango_font_description_from_string(f.c_str());
	//PangoFontDescription *desc = pango_font_description_new();
	//pango_font_description_set_family(desc, "Sans");//cur_font);
	//pango_font_description_set_size(desc, 10);//cur_font_size);
	pango_layout_set_font_description(layout, font_desc);

}

void Painter::set_font_size(float size) {
	if (!cr)
		return;
	set_font(cur_font, size, cur_font_bold, cur_font_italic);
}

void Painter::set_antialiasing(bool enabled) {
	if (!cr)
		return;
	if (enabled)
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_DEFAULT);
	else
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
}

void Painter::set_fill(bool fill) {
	mode_fill = fill;
}

void Painter::set_transform(float *rot, const vec2 &offset) {
	cairo_matrix_t m;

	if (rot) {
		m.xx = rot[0];
		m.xy = rot[1];
		m.yx = rot[2];
		m.yy = rot[3];
	}
	m.x0 = offset.x + _initial_offset_x;
	m.y0 = offset.y + _initial_offset_y;
	cairo_set_matrix(cr, &m);
}


};

#endif
