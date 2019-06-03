/*
 * ImagePainter.cpp
 *
 *  Created on: 02.04.2016
 *      Author: michi
 */

#include "ImagePainter.h"
#include "image.h"
#include "../math/complex.h"
#include <math.h>

ImagePainter::ImagePainter(Image *im)
{
	image = im;
	width = image->width;
	height = image->height;
	set_clip(rect(0, (float)width, 0, (float)height));
	_color = Black;
	dash_offset = 0;
	line_width = 1.0f;
	anti_aliasing = false;
	fill = true;
}

ImagePainter::~ImagePainter()
{
	end();
}

void ImagePainter::end()
{
}

void ImagePainter::set_color(const color& c)
{
	_color = c;
}

/*void ImagePainter::set_font(const string& font, float size, bool bold, bool italic)
{
}*/

void ImagePainter::set_font_size(float size)
{
}

void ImagePainter::set_antialiasing(bool enabled)
{
	anti_aliasing = enabled;
}

void ImagePainter::set_line_width(float w)
{
	line_width = w;
}

void ImagePainter::set_line_dash(const Array<float>& _dash, float offset)
{
	dash = _dash;
	dash_offset = offset;
}

void ImagePainter::set_fill(bool _fill)
{
	fill = _fill;
}

void ImagePainter::set_clip(const rect& r)
{
	_clip = r;
}

void ImagePainter::draw_point(float x, float y)
{
	image->draw_pixel(x, y, _color);
}

void ImagePainter::draw_line(float x1, float y1, float x2, float y2)
{
	complex p0 = complex(x1, y1);
	complex p1 = complex(x2, y2);
	complex dir = p1 - p0;
	float length = dir.abs();
	dir /= length;

	complex e = complex(dir.y, -dir.x);

	int _x0 = (int)max(min(x1, x2) - line_width/2 - 1, _clip.x1);
	int _x1 = (int)min(max(x1, x2) + line_width/2 + 1, _clip.x2);
	int _y0 = (int)max(min(y1, y2) - line_width/2 - 1, _clip.y1);
	int _y1 = (int)min(max(y1, y2) + line_width/2 + 1, _clip.y2);

	color cc = _color;

	for (int x=_x0; x<_x1; x++)
		for (int y=_y0; y<_y1; y++){
			float d2 = (x - x1) * dir.x + (y - y1) * dir.y;
			float alpha2 = min(d2, length - d2);
			float d1 = (x - x1) * e.x + (y - y1) * e.y;
			float alpha1 = line_width/2 + 0.5f - (float)fabs(d1);
			float alpha = min(alpha1, alpha2);
			if (anti_aliasing){
				cc.a = _color.a * alpha;
			}else{
				if (alpha < 0.5f)
					continue;
			}
			image->draw_pixel(x, y, cc);
		}
}

void ImagePainter::draw_lines(const Array<complex>& p)
{
	for (int i=1; i<p.num; i++)
		draw_line(p[i-1].x, p[i-1].y, p[i].x, p[i].y);
}

void ImagePainter::draw_polygon(const Array<complex>& p)
{
	draw_lines(p);
}

void ImagePainter::draw_rect(float xx, float yy, float w, float h)
{
	int x0 = (int)max(xx, _clip.x1);
	int x1 = (int)min(xx + w, _clip.x2);
	int y0 = (int)max(yy, _clip.y1);
	int y1 = (int)min(yy + h, _clip.y2);

	for (int x=x0; x<x1; x++)
		for (int y=y0; y<y1; y++)
			image->draw_pixel(x, y, _color);
}

void ImagePainter::draw_rect(const rect& r)
{
	draw_rect(r.x1, r.y1, r.width(), r.height());
}

void ImagePainter::draw_circle(float cx, float cy, float radius)
{
	int x0 = (int)max(cx - radius - line_width/2 - 1, _clip.x1);
	int x1 = (int)min(cx + radius + line_width/2 + 1, _clip.x2);
	int y0 = (int)max(cy - radius - line_width/2 - 1, _clip.y1);
	int y1 = (int)min(cy + radius + line_width/2 + 1, _clip.y2);

	color cc = _color;

	if (fill){
		for (int x=x0; x<x1; x++)
			for (int y=y0; y<y1; y++){
				float r = (float)sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));
				float alpha = radius - r;
				if (anti_aliasing){
					cc.a = _color.a * alpha;
				}else{
					if (alpha < 0.5f)
						continue;
				}
				image->draw_pixel(x, y, cc);
			}
	}else{
		for (int x=x0; x<x1; x++)
			for (int y=y0; y<y1; y++){
				float r = (float)sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));
				float alpha = line_width/2 + 0.5f - (float)fabs(radius - r);
				if (anti_aliasing){
					cc.a = _color.a * alpha;
				}else{
					if (alpha < 0.5f)
						continue;
				}
				image->draw_pixel(x, y, cc);
			}
	}
}

void ImagePainter::draw_str(float x, float y, const string& str)
{
}

float ImagePainter::get_str_width(const string& str)
{
	return 0;
}

void ImagePainter::draw_image(float dx, float dy, const Image& im)
{
	int _x0 = (int)max(dx, _clip.x1);
	int _x1 = (int)min(dx + im.width, _clip.x2);
	int _y0 = (int)max(dy, _clip.y1);
	int _y1 = (int)min(dy + im.height, _clip.y2);

	for (int x=_x0; x<_x1; x++)
		for (int y=_y0; y<_y1; y++)
			image->draw_pixel(x, y, im.get_pixel(x - (int)dx, y - (int)dy));
}

void ImagePainter::draw_mask_image(float x, float y, const Image& image)
{
}
