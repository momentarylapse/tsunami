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


class ImagePainter : public Painter
{
public:
	ImagePainter(Image *image);
	virtual ~ImagePainter();

	Image *image;

	virtual void _cdecl end();
	virtual void _cdecl setColor(const color &c);
	virtual void _cdecl setFont(const string &font, float size, bool bold, bool italic);
	virtual void _cdecl setFontSize(float size);
	virtual void _cdecl setAntialiasing(bool enabled);
	virtual void _cdecl setLineWidth(float w);
	virtual void _cdecl setLineDash(const Array<float> &dash, float offset);
	virtual void _cdecl setFill(bool fill);
	virtual void _cdecl clip(const rect &r);
	virtual void _cdecl drawPoint(float x, float y);
	virtual void _cdecl drawLine(float x1, float y1, float x2, float y2);
	virtual void _cdecl drawLines(const Array<complex> &p);
	virtual void _cdecl drawPolygon(const Array<complex> &p);
	virtual void _cdecl drawRect(float x1, float y1, float w, float h);
	virtual void _cdecl drawRect(const rect &r);
	virtual void _cdecl drawCircle(float x, float y, float radius);
	virtual void _cdecl drawStr(float x, float y, const string &str);
	virtual float _cdecl getStrWidth(const string &str);
	virtual void _cdecl drawImage(float x, float y, const Image &image);
	virtual void _cdecl drawMaskImage(float x, float y, const Image &image);

	color _color;
	rect _clip;
	Array<float> dash;
	float dash_offset;
	float line_width;
	bool anti_aliasing;
	bool fill;
};

#endif /* SRC_LIB_IMAGE_IMAGEPAINTER_H_ */
