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
	void _cdecl end();
	color _cdecl getThemeColor(int i);
	void _cdecl setColor(const color &c);
	void _cdecl setFont(const string &font, float size, bool bold, bool italic);
	void _cdecl setFontSize(float size);
	void _cdecl setAntialiasing(bool enabled);
	void _cdecl setLineWidth(float w);
	void _cdecl setLineDash(Array<float> &dash, float offset);
	void _cdecl clip(const rect &r);
	void _cdecl drawPoint(float x, float y);
	void _cdecl drawLine(float x1, float y1, float x2, float y2);
	void _cdecl drawLines(Array<complex> &p);
	void _cdecl drawPolygon(Array<complex> &p);
	void _cdecl drawRect(float x1, float y1, float w, float h);
	void _cdecl drawRect(const rect &r);
	void _cdecl drawCircle(float x, float y, float radius);
	void _cdecl drawStr(float x, float y, const string &str);
	float _cdecl getStrWidth(const string &str);
	void _cdecl drawImage(float x, float y, const Image &image);
	int width, height;
};

#endif /* HUIPAINTER_H_ */
