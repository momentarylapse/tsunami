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
class color;

namespace hui
{

class Window;

class Painter : public ::Painter
{
	public:
#ifdef HUI_API_GTK
	cairo_t *cr;
	PangoLayout *layout;
	PangoFontDescription *font_desc;
	cairo_surface_t *target_surface;
#endif
	Window *win;
	string id;
	int cur_font_size;
	string cur_font;
	bool cur_font_bold, cur_font_italic;
	bool mode_fill;
	float corner_radius;

	Painter();
	Painter(Panel *panel, const string &id);
	~Painter() override;

	void _cdecl setColor(const color &c) override;
	void _cdecl setFont(const string &font, float size, bool bold, bool italic) override;
	void _cdecl setFontSize(float size) override;
	void _cdecl setAntialiasing(bool enabled) override;
	void _cdecl setLineWidth(float w) override;
	void _cdecl setLineDash(const Array<float> &dash, float offset) override;
	void _cdecl setRoundness(float radius) override;
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
	rect _cdecl area() override;
	rect _cdecl getClip() override;
};

Painter *start_image_paint(Image &im);
void end_image_paint(Image &im, ::Painter *p);

};

#endif /* HUIPAINTER_H_ */
