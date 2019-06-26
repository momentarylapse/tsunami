/*
 * ImagePainter.h
 *
 *  Created on: 02.04.2016
 *      Author: michi
 */

#ifndef SRC_LIB_IMAGE_PAINTER_H_
#define SRC_LIB_IMAGE_PAINTER_H_

#include "../base/base.h"

class Image;
class color;
class rect;
class complex;

class Painter {
public:
	Painter() {}
	virtual ~Painter() {}

	virtual void _cdecl set_color(const color &c) = 0;
	virtual void _cdecl set_font(const string &font, float size, bool bold, bool italic) = 0;
	virtual void _cdecl set_font_size(float size) = 0;
	virtual void _cdecl set_antialiasing(bool enabled) = 0;
	virtual void _cdecl set_line_width(float w) = 0;
	virtual void _cdecl set_roundness(float radius){}
	virtual void _cdecl set_line_dash(const Array<float> &dash, float offset) = 0;
	virtual void _cdecl set_fill(bool fill) = 0;
	virtual void _cdecl set_clip(const rect &r) = 0;
	virtual void _cdecl draw_point(float x, float y) = 0;
	virtual void _cdecl draw_line(float x1, float y1, float x2, float y2) = 0;
	virtual void _cdecl draw_lines(const Array<complex> &p) = 0;
	virtual void _cdecl draw_polygon(const Array<complex> &p) = 0;
	virtual void _cdecl draw_rect(float x1, float y1, float w, float h) = 0;
	virtual void _cdecl draw_rect(const rect &r) = 0;
	virtual void _cdecl draw_circle(float x, float y, float radius) = 0;
	virtual void _cdecl draw_str(float x, float y, const string &str) = 0;
	virtual float _cdecl get_str_width(const string &str) = 0;
	virtual void _cdecl draw_image(float x, float y, const Image &image) = 0;
	virtual void _cdecl draw_mask_image(float x, float y, const Image &image) = 0;
	int width = 0;
	int height = 0;
	float font_size = 12;
	virtual rect area() = 0;
	virtual rect clip() = 0;
};


#endif /* SRC_LIB_IMAGE_PAINTER_H_ */
