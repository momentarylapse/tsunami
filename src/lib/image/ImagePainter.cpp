/*
 * ImagePainter.cpp
 *
 *  Created on: 02.04.2016
 *      Author: michi
 */

#include "ImagePainter.h"
#include "image.h"
#include "../math/vec2.h"
#if __has_include("../hui/Painter.h") && (HAS_LIB_GTK3 || HAS_LIB_GTK4)
#include "../hui/Painter.h"
#include <cairo/cairo.h>
#include <gtk/gtk.h>
#define HAS_HUI_PAINTER 1
#endif
#include "../os/msg.h"
#include <math.h>

#ifdef HAS_HUI_PAINTER
#endif

ImagePainter::ImagePainter() {
}

ImagePainter::ImagePainter(Image *im) {
	image = im;
	width = image->width;
	height = image->height;
	set_clip(rect(0, (float)width, 0, (float)height));
	_color = Black;
	dash_offset = 0;
	line_width = 1.0f;
	anti_aliasing = false;
	fill = true;
	font_size = 12;
	font_name = "Sans";
}

ImagePainter::~ImagePainter() {
	end();
}

void ImagePainter::end() {
}

void ImagePainter::set_color(const color& c) {
	_color = c;
}

void ImagePainter::set_font(const string& font, float size, bool bold, bool italic) {
	font_name = font;
	font_size = size;
}

void ImagePainter::set_font_size(float size) {
	font_size = size;
}

void ImagePainter::set_antialiasing(bool enabled) {
	anti_aliasing = enabled;
}

void ImagePainter::set_line_width(float w) {
	line_width = w;
}

void ImagePainter::set_line_dash(const Array<float>& _dash, float offset) {
	dash = _dash;
	dash_offset = offset;
}

void ImagePainter::set_fill(bool _fill) {
	fill = _fill;
}

void ImagePainter::set_clip(const rect& r) {
	_clip = r;
}

rect Painter::area() const {
	return rect(0, (float)width, 0, (float)height);
}

rect ImagePainter::clip() const {
	return _clip;
}

void ImagePainter::draw_point(const vec2 &p) {
	image->draw_pixel(p.x, p.y, _color);
}

void ImagePainter::draw_line(const vec2 &p0, const vec2 &p1) {
	vec2 dir = p1 - p0;
	float length = dir.length();
	dir /= length;

	vec2 e = vec2(dir.y, -dir.x);

	int _x0 = (int)max(min(p0.x, p1.x) - line_width/2 - 1, _clip.x1);
	int _x1 = (int)min(max(p0.x, p1.x) + line_width/2 + 1, _clip.x2);
	int _y0 = (int)max(min(p0.y, p1.y) - line_width/2 - 1, _clip.y1);
	int _y1 = (int)min(max(p0.y, p1.y) + line_width/2 + 1, _clip.y2);

	color cc = _color;

	for (int x=_x0; x<=_x1; x++)
		for (int y=_y0; y<=_y1; y++) {
			float d2 = vec2::dot(vec2(x,y)-p0, dir);  //(x - p0.x) * dir.x + (y - p0.y) * dir.y;
			float alpha2 = min(d2, length - d2) + 0.5f;
			float d1 = vec2::dot(vec2(x,y)-p0, e); //(x - p0.x) * e.x + (y - p0.y) * e.y;
			float alpha1 = line_width/2 + 0.5f - (float)fabs(d1);
			float alpha = min(alpha1, alpha2);
			if (anti_aliasing) {
				cc.a = _color.a * alpha;
			} else {
				if (alpha < 0.5f)
					continue;
			}
			image->draw_pixel(x, y, cc);
		}
}

void ImagePainter::draw_lines(const Array<vec2>& p) {
	for (int i=1; i<p.num; i++)
		draw_line(p[i-1], p[i]);
}

void ImagePainter::draw_polygon(const Array<vec2>& p) {
	draw_lines(p);
}

void ImagePainter::draw_rect(const rect& r) {
	int x0 = (int)max(r.x1, _clip.x1);
	int x1 = (int)min(r.x2, _clip.x2);
	int y0 = (int)max(r.y1, _clip.y1);
	int y1 = (int)min(r.y2, _clip.y2);

	for (int x=x0; x<x1; x++)
		for (int y=y0; y<y1; y++)
			image->draw_pixel(x, y, _color);
}

void ImagePainter::draw_circle(const vec2 &c, float radius) {
	int x0 = (int)max(c.x - radius - line_width/2 - 1, _clip.x1);
	int x1 = (int)min(c.x + radius + line_width/2 + 1, _clip.x2);
	int y0 = (int)max(c.y - radius - line_width/2 - 1, _clip.y1);
	int y1 = (int)min(c.y + radius + line_width/2 + 1, _clip.y2);

	color cc = _color;

	if (fill) {
		for (int x=x0; x<x1; x++)
			for (int y=y0; y<y1; y++) {
				float r = (float)sqrt((x - c.x) * (x - c.x) + (y - c.y) * (y - c.y));
				float alpha = radius - r;
				if (anti_aliasing) {
					cc.a = _color.a * alpha;
				} else {
					if (alpha < 0.5f)
						continue;
				}
				image->draw_pixel(x, y, cc);
			}
	} else {
		for (int x=x0; x<x1; x++)
			for (int y=y0; y<y1; y++) {
				float r = (float)sqrt((x - c.x) * (x - c.x) + (y - c.y) * (y - c.y));
				float alpha = line_width/2 + 0.5f - (float)fabs(radius - r);
				if (anti_aliasing) {
					cc.a = _color.a * alpha;
				} else {
					if (alpha < 0.5f)
						continue;
				}
				image->draw_pixel(x, y, cc);
			}
	}
}

void ImagePainter::draw_str(const vec2 &p, const string& str) {
#if 1
#if HAS_LIB_GTK3


	cairo_surface_t *surface = cairo_image_surface_create_for_data((unsigned char*)image->data.data, CAIRO_FORMAT_ARGB32, width, height, width * 4);
	cairo_t *cr = cairo_create(surface);

	cairo_set_source_rgba(cr, _color.r, _color.g, _color.b, _color.a);

	PangoLayout *layout = pango_cairo_create_layout(cr);
	PangoFontDescription *desc = pango_font_description_from_string((font_name + "," + f2s(font_size, 1)).c_str());
	pango_layout_set_font_description(layout, desc);
	pango_font_description_free(desc);

	pango_layout_set_text(layout, (char*)str.data, str.num);
	//int baseline = pango_layout_get_baseline(layout) / PANGO_SCALE;
	//int w_used, h_used;
	//pango_layout_get_pixel_size(layout, &w_used, &h_used);

	float dy = (float)pango_layout_get_baseline(layout) / 1000.0f;
	cairo_move_to(cr, p.x, p.y - dy + font_size);

	pango_cairo_show_layout(cr, layout);
	g_object_unref(layout);

	cairo_surface_flush(surface);

	cairo_destroy(cr);
	cairo_surface_destroy(surface);
#endif
#endif
}

float ImagePainter::get_str_width(const string& str) {
	return 0;
}

void ImagePainter::draw_image(const vec2 &d, const Image *im) {
	int _x0 = (int)max(d.x, _clip.x1);
	int _x1 = (int)min(d.x + im->width, _clip.x2);
	int _y0 = (int)max(d.y, _clip.y1);
	int _y1 = (int)min(d.y + im->height, _clip.y2);

	for (int x=_x0; x<_x1; x++)
		for (int y=_y0; y<_y1; y++)
			image->draw_pixel(x, y, im->get_pixel(x - (int)d.x, y - (int)d.y));
}

void ImagePainter::draw_mask_image(const vec2 &d, const Image *image) {
}


Painter *Image::start_draw() {
#if 0
#ifdef HAS_HUI_PAINTER
	//set_mode(Mode::BGRA);

	cairo_surface_t *surface = cairo_image_surface_create_for_data((unsigned char*)data.data, CAIRO_FORMAT_ARGB32, width, height, width * 4);

	cairo_t *cr = cairo_create(surface);
	//auto surf = cairo_image_surface_create_for_data((unsigned char*)data.data, CAIRO_FORMAT_ARGB32, width, height, width*4);
	return new hui::Painter(surface, cr, width, height);
#endif
#endif
	return new ImagePainter(this);
}
