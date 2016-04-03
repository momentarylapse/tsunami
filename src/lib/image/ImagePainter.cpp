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
	clip(rect(0, width, 0, height));
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

void ImagePainter::setColor(const color& c)
{
	_color = c;
}

void ImagePainter::setFont(const string& font, float size, bool bold, bool italic)
{
}

void ImagePainter::setFontSize(float size)
{
}

void ImagePainter::setAntialiasing(bool enabled)
{
	anti_aliasing = enabled;
}

void ImagePainter::setLineWidth(float w)
{
	line_width = w;
}

void ImagePainter::setLineDash(const Array<float>& _dash, float offset)
{
	dash = _dash;
	dash_offset = offset;
}

void ImagePainter::setFill(bool _fill)
{
	fill = _fill;
}

void ImagePainter::clip(const rect& r)
{
	_clip = r;
}

void ImagePainter::drawPoint(float x, float y)
{
	image->drawPixel(x, y, _color);
}

void ImagePainter::drawLine(float x1, float y1, float x2, float y2)
{
	complex p0 = complex(x1, y1);
	complex p1 = complex(x2, y2);
	complex dir = p1 - p0;
	float length = dir.abs();
	dir /= length;

	complex e = complex(dir.y, -dir.x);

	int _x0 = max(min(x1, x2) - line_width/2 - 1, _clip.x1);
	int _x1 = min(max(x1, x2) + line_width/2 + 1, _clip.x2);
	int _y0 = max(min(y1, y2) - line_width/2 - 1, _clip.y1);
	int _y1 = min(max(y1, y2) + line_width/2 + 1, _clip.y2);

	color cc = _color;

	for (int x=_x0; x<_x1; x++)
		for (int y=_y0; y<_y1; y++){
			float d2 = (x - x1) * dir.x + (y - y1) * dir.y;
			float alpha2 = min(d2, length - d2);
			float d1 = (x - x1) * e.x + (y - y1) * e.y;
			float alpha1 = line_width/2 + 0.5f - fabs(d1);
			float alpha = min(alpha1, alpha2);
			if (anti_aliasing){
				cc.a = _color.a * alpha;
			}else{
				if (alpha < 0.5f)
					continue;
			}
			image->drawPixel(x, y, cc);
		}
}

void ImagePainter::drawLines(const Array<complex>& p)
{
	for (int i=1; i<p.num; i++)
		drawLine(p[i-1].x, p[i-1].y, p[i].x, p[i].y);
}

void ImagePainter::drawPolygon(const Array<complex>& p)
{
	drawLines(p);
}

void ImagePainter::drawRect(float xx, float yy, float w, float h)
{
	int x0 = max(xx, _clip.x1);
	int x1 = min(xx + w, _clip.x2);
	int y0 = max(yy, _clip.y1);
	int y1 = min(yy + h, _clip.y2);

	for (int x=x0; x<x1; x++)
		for (int y=y0; y<y1; y++)
			image->drawPixel(x, y, _color);
}

void ImagePainter::drawRect(const rect& r)
{
	drawRect(r.x1, r.y1, r.width(), r.height());
}

void ImagePainter::drawCircle(float cx, float cy, float radius)
{
	int x0 = max(cx - radius - line_width/2 - 1, _clip.x1);
	int x1 = min(cx + radius + line_width/2 + 1, _clip.x2);
	int y0 = max(cy - radius - line_width/2 - 1, _clip.y1);
	int y1 = min(cy + radius + line_width/2 + 1, _clip.y2);

	color cc = _color;

	if (fill){
		for (int x=x0; x<x1; x++)
			for (int y=y0; y<y1; y++){
				float r = sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));
				float alpha = radius - r;
				if (anti_aliasing){
					cc.a = _color.a * alpha;
				}else{
					if (alpha < 0.5f)
						continue;
				}
				image->drawPixel(x, y, cc);
			}
	}else{
		for (int x=x0; x<x1; x++)
			for (int y=y0; y<y1; y++){
				float r = sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));
				float alpha = line_width/2 + 0.5f - fabs(radius - r);
				if (anti_aliasing){
					cc.a = _color.a * alpha;
				}else{
					if (alpha < 0.5f)
						continue;
				}
				image->drawPixel(x, y, cc);
			}
	}
}

void ImagePainter::drawStr(float x, float y, const string& str)
{
}

float ImagePainter::getStrWidth(const string& str)
{
	return 0;
}

void ImagePainter::drawImage(float dx, float dy, const Image& im)
{
	int _x0 = max(dx, _clip.x1);
	int _x1 = min(dx + im.width, _clip.x2);
	int _y0 = max(dy, _clip.y1);
	int _y1 = min(dy + im.height, _clip.y2);

	for (int x=_x0; x<_x1; x++)
		for (int y=_y0; y<_y1; y++)
			image->drawPixel(x, y, im.getPixel(x - dx, y - dy));
}

void ImagePainter::drawMaskImage(float x, float y, const Image& image)
{
}
