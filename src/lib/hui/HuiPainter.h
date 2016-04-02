/*
 * HuiPainter.h
 *
 *  Created on: 25.06.2013
 *      Author: michi
 */

#ifndef HUIPAINTER_H_
#define HUIPAINTER_H_

class HuiWindow;
class complex;
class rect;

class HuiPainter
{
	public:
#ifdef HUI_API_GTK
	cairo_t *cr;
#endif
	HuiWindow *win;
	string id;
	int cur_font_size;
	string cur_font;
	bool cur_font_bold, cur_font_italic;
	bool mode_fill;

	HuiPainter();
	virtual ~HuiPainter();

	virtual void _cdecl end();
	color _cdecl getThemeColor(int i);
	virtual void _cdecl setColor(const color &c);
	virtual void _cdecl setFont(const string &font, float size, bool bold, bool italic);
	virtual void _cdecl setFontSize(float size);
	virtual void _cdecl setAntialiasing(bool enabled);
	virtual void _cdecl setLineWidth(float w);
	virtual void _cdecl setLineDash(Array<float> &dash, float offset);
	virtual void _cdecl setFill(bool fill);
	virtual void _cdecl clip(const rect &r);
	virtual void _cdecl drawPoint(float x, float y);
	virtual void _cdecl drawLine(float x1, float y1, float x2, float y2);
	virtual void _cdecl drawLines(Array<complex> &p);
	virtual void _cdecl drawPolygon(Array<complex> &p);
	virtual void _cdecl drawRect(float x1, float y1, float w, float h);
	virtual void _cdecl drawRect(const rect &r);
	virtual void _cdecl drawCircle(float x, float y, float radius);
	virtual void _cdecl drawStr(float x, float y, const string &str);
	virtual float _cdecl getStrWidth(const string &str);
	virtual void _cdecl drawImage(float x, float y, const Image &image);
	virtual void _cdecl drawMaskImage(float x, float y, const Image &image);
	int width, height;
};

#endif /* HUIPAINTER_H_ */
