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

class Painter
{
public:
	Painter(){}
	virtual ~Painter(){}

	virtual void _cdecl setColor(const color &c) = 0;
	virtual void _cdecl setFont(const string &font, float size, bool bold, bool italic) = 0;
	virtual void _cdecl setFontSize(float size) = 0;
	virtual void _cdecl setAntialiasing(bool enabled) = 0;
	virtual void _cdecl setLineWidth(float w) = 0;
	virtual void _cdecl setRoundness(float radius){}
	virtual void _cdecl setLineDash(const Array<float> &dash, float offset) = 0;
	virtual void _cdecl setFill(bool fill) = 0;
	virtual void _cdecl clip(const rect &r) = 0;
	virtual void _cdecl drawPoint(float x, float y) = 0;
	virtual void _cdecl drawLine(float x1, float y1, float x2, float y2) = 0;
	virtual void _cdecl drawLines(const Array<complex> &p) = 0;
	virtual void _cdecl drawPolygon(const Array<complex> &p) = 0;
	virtual void _cdecl drawRect(float x1, float y1, float w, float h) = 0;
	virtual void _cdecl drawRect(const rect &r) = 0;
	virtual void _cdecl drawCircle(float x, float y, float radius) = 0;
	virtual void _cdecl drawStr(float x, float y, const string &str) = 0;
	virtual float _cdecl getStrWidth(const string &str) = 0;
	virtual void _cdecl drawImage(float x, float y, const Image &image) = 0;
	virtual void _cdecl drawMaskImage(float x, float y, const Image &image) = 0;
	int width, height;
};


#endif /* SRC_LIB_IMAGE_PAINTER_H_ */
