/*
 * ImagePainter.h
 *
 *  Created on: 02.04.2016
 *      Author: michi
 */

#ifndef SRC_LIB_IMAGE_PAINTER_H_
#define SRC_LIB_IMAGE_PAINTER_H_

#include "../base/base.h"
#include "../math/vec2.h"

class Image;
struct color;
struct rect;
struct vec2;

class Painter : public VirtualBase {
public:
	Painter() {}
	virtual ~Painter() {}

	virtual void _cdecl set_color(const color &c) = 0;
	virtual void _cdecl set_font(const string &font, float size, bool bold, bool italic) = 0;
	virtual void _cdecl set_font_size(float size) = 0;
	virtual void _cdecl set_antialiasing(bool enabled) = 0;
	virtual void _cdecl set_line_width(float w) = 0;
	virtual void _cdecl set_roundness(float radius) {}
	virtual void _cdecl set_line_dash(const Array<float> &dash, float offset) = 0;
	virtual void _cdecl set_fill(bool fill) = 0;
	virtual void _cdecl set_clip(const rect &r) = 0;
	virtual void _cdecl draw_point(const vec2 &p) = 0;
	virtual void _cdecl draw_line(const vec2 &a, const vec2 &b) = 0;
	virtual void _cdecl draw_lines(const Array<vec2> &p) = 0;
	virtual void _cdecl draw_polygon(const Array<vec2> &p) = 0;
	virtual void _cdecl draw_rect(const rect &r) = 0;
	virtual void _cdecl draw_circle(const vec2 &p, float radius) = 0;
	virtual void _cdecl draw_str(const vec2 &p, const string &str) = 0;
	virtual float _cdecl get_str_width(const string &str) {
		return get_str_size(str).x;
	}
	virtual vec2 _cdecl get_str_size(const string &str) = 0;
	virtual void _cdecl draw_image(const vec2 &p, const Image *image) = 0;
	virtual void _cdecl draw_mask_image(const vec2 &p, const Image *image) = 0;
	virtual void _cdecl set_transform(float rot[], const vec2 &offset) {}
	virtual void _cdecl set_option(const string &key, const string &value) {}
	virtual bool _cdecl allow_images() const { return true; }
	int width = 0;
	int height = 0;
	float font_size = 12;
	virtual rect area() const; // ImagePainter.cpp...
	virtual rect clip() const = 0;
};


#endif /* SRC_LIB_IMAGE_PAINTER_H_ */
