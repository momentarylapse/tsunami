/*
 * HuiPainter.h
 *
 *  Created on: 25.06.2013
 *      Author: michi
 */

#ifndef HUIPAINTER_H_
#define HUIPAINTER_H_

#include "../image/Painter.h"

class complex;
class rect;

namespace hui
{

class Window;

class Painter : public ::Painter
{
	public:
#ifdef HUI_API_GTK
	cairo_t *cr;
#endif
	Window *win;
	string id;
	int cur_font_size;
	string cur_font;
	bool cur_font_bold, cur_font_italic;
	bool mode_fill;

	Painter();
	Painter(Panel *panel, const string &id);
	virtual ~Painter();

	color _cdecl getThemeColor(int i);

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
};

};

#endif /* HUIPAINTER_H_ */
