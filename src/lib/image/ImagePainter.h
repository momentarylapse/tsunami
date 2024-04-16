/*
 * ImagePainter.h
 *
 *  Created on: 02.04.2016
 *      Author: michi
 */

#ifndef SRC_LIB_IMAGE_IMAGEPAINTER_H_
#define SRC_LIB_IMAGE_IMAGEPAINTER_H_

#include "Painter.h"
#include "color.h"
#include "../math/rect.h"


class ImagePainter : public Painter {
public:
	ImagePainter();
	ImagePainter(Image *image);
	virtual ~ImagePainter();

	Image *image;
#if HAS_LIB_GTK3
	//void *cairo_context;
#endif

	void _cdecl end();
	void _cdecl set_color(const color &c) override;
	void _cdecl set_font(const string &font, float size, bool bold, bool italic) override;
	void _cdecl set_font_size(float size) override;
	void _cdecl set_antialiasing(bool enabled) override;
	void _cdecl set_line_width(float w) override;
	void _cdecl set_line_dash(const Array<float> &dash, float offset) override;
	void _cdecl set_fill(bool fill) override;
	void _cdecl set_clip(const rect &r) override;
	void _cdecl draw_point(const vec2 &p) override;
	void _cdecl draw_line(const vec2 &a, const vec2 &b) override;
	void _cdecl draw_lines(const Array<vec2> &p) override;
	void _cdecl draw_polygon(const Array<vec2> &p) override;
	void _cdecl draw_rect(const rect &r) override;
	void _cdecl draw_circle(const vec2 &p, float radius) override;
	void _cdecl draw_str(const vec2 &p, const string &str) override;
	vec2 _cdecl get_str_size(const string &str) override;
	void _cdecl draw_image(const vec2 &p, const Image *image) override;
	void _cdecl draw_mask_image(const vec2 &p, const Image *image) override;

	rect clip() const override;

	color _color;
	string font_name;
	rect _clip;
	Array<float> dash;
	float dash_offset;
	float line_width;
	bool anti_aliasing;
	bool fill;
};

#endif /* SRC_LIB_IMAGE_IMAGEPAINTER_H_ */
