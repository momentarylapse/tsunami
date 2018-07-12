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

	void _cdecl end();
	void _cdecl setColor(const color &c) override;
	void _cdecl setFont(const string &font, float size, bool bold, bool italic) override;
	void _cdecl setFontSize(float size) override;
	void _cdecl setAntialiasing(bool enabled) override;
	void _cdecl setLineWidth(float w) override;
	void _cdecl setLineDash(const Array<float> &dash, float offset) override;
	void _cdecl setFill(bool fill) override;
	void _cdecl clip(const rect &r) override;
	void _cdecl drawPoint(float x, float y) override;
	void _cdecl drawLine(float x1, float y1, float x2, float y2) override;
	void _cdecl drawLines(const Array<complex> &p) override;
	void _cdecl drawPolygon(const Array<complex> &p) override;
	void _cdecl drawRect(float x1, float y1, float w, float h) override;
	void _cdecl drawRect(const rect &r) override;
	void _cdecl drawCircle(float x, float y, float radius) override;
	void _cdecl drawStr(float x, float y, const string &str) override;
	float _cdecl getStrWidth(const string &str) override;
	void _cdecl drawImage(float x, float y, const Image &image) override;
	void _cdecl drawMaskImage(float x, float y, const Image &image) override;

	color _color;
	rect _clip;
	Array<float> dash;
	float dash_offset;
	float line_width;
	bool anti_aliasing;
	bool fill;
};

#endif /* SRC_LIB_IMAGE_IMAGEPAINTER_H_ */
