/*
 * HuiPainterGtk.cpp
 *
 *  Created on: 25.06.2013
 *      Author: michi
 */

#include "hui.h"
#include "HuiPainter.h"
#include "../math/math.h"

#ifdef HUI_API_GTK

void HuiPainter::End()
{
	if (!cr)
		return;

	cairo_destroy(cr);
}

void HuiPainter::SetColor(const color &c)
{
	if (!cr)
		return;
	cairo_set_source_rgba(cr, c.r, c.g, c.b, c.a);
}

void HuiPainter::SetLineWidth(float w)
{
	if (!cr)
		return;
	cairo_set_line_width(cr, w);
}

void HuiPainter::SetLineDash(Array<float> &dash, float offset)
{
	if (!cr)
		return;
	Array<double> d;
	foreach(float f, dash)
		d.add(f);
	cairo_set_dash(cr, (double*)d.data, d.num, offset);
}

color HuiPainter::GetThemeColor(int i)
{
	GtkStyle *style = gtk_widget_get_style(win->window);
	int x = (i / 5);
	int y = (i % 5);
	GdkColor c;
	if (x == 0)
		c = style->fg[y];
	else if (x == 1)
		c = style->bg[y];
	else if (x == 2)
		c = style->light[y];
	else if (x == 3)
		c = style->mid[y];
	else if (x == 4)
		c = style->dark[y];
	else if (x == 5)
		c = style->base[y];
	else if (x == 6)
		c = style->text[y];
	return color(1, (float)c.red / 65535.0f, (float)c.green / 65535.0f, (float)c.blue / 65535.0f);
}


void HuiPainter::DrawPoint(float x, float y)
{
	if (!cr)
		return;
}

void HuiPainter::DrawLine(float x1, float y1, float x2, float y2)
{
	if (!cr)
		return;
	cairo_move_to(cr, x1 + 0.5f, y1 + 0.5f);
	cairo_line_to(cr, x2 + 0.5f, y2 + 0.5f);
	cairo_stroke(cr);
}

void HuiPainter::DrawLines(Array<complex> &p)
{
	if (!cr)
		return;
	if (p.num == 0)
		return;
	//cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
	//cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
	cairo_move_to(cr, p[0].x, p[0].y);
	for (int i=1; i<p.num; i++)
		cairo_line_to(cr, p[i].x, p[i].y);
	cairo_stroke(cr);
}

void HuiPainter::DrawPolygon(Array<complex> &p)
{
	if (!cr)
		return;
	if (p.num == 0)
		return;
	cairo_move_to(cr, p[0].x, p[0].y);
	for (int i=1; i<p.num; i++)
		cairo_line_to(cr, p[i].x, p[i].y);
	cairo_close_path(cr);
	cairo_fill(cr);
}

void HuiPainter::DrawStr(float x, float y, const string &str)
{
	if (!cr)
		return;
	cairo_move_to(cr, x, y);// + cur_font_size);
	PangoLayout *layout = pango_cairo_create_layout(cr);
	pango_layout_set_text(layout, (char*)str.data, str.num);//.c_str(), -1);
	string f = cur_font;
	if (cur_font_bold)
		f += " Bold";
	if (cur_font_italic)
		f += " Italic";
	f += " " + i2s(cur_font_size);
	PangoFontDescription *desc = pango_font_description_from_string(f.c_str());
	//PangoFontDescription *desc = pango_font_description_new();
	//pango_font_description_set_family(desc, "Sans");//cur_font);
	//pango_font_description_set_size(desc, 10);//cur_font_size);
	pango_layout_set_font_description(layout, desc);
	pango_font_description_free(desc);
	pango_cairo_show_layout(cr, layout);
	g_object_unref(layout);

	//cairo_show_text(cr, str);
}

float HuiPainter::GetStrWidth(const string &str)
{
	if (!cr)
		return 0;
	PangoLayout *layout = pango_cairo_create_layout(cr);
	pango_layout_set_text(layout, (char*)str.data, str.num);//.c_str(), -1);
	string f = cur_font;
	if (cur_font_bold)
		f += " Bold";
	if (cur_font_italic)
		f += " Italic";
	f += " " + i2s(cur_font_size);
	PangoFontDescription *desc = pango_font_description_from_string(f.c_str());
	//PangoFontDescription *desc = pango_font_description_new();
	//pango_font_description_set_family(desc, "Sans");//cur_font);
	//pango_font_description_set_size(desc, 10);//cur_font_size);
	pango_layout_set_font_description(layout, desc);
	pango_font_description_free(desc);
	int w, h;
	pango_layout_get_size(layout, &w, &h);
	g_object_unref(layout);

	return (float)w / 1000.0f;
}

void HuiPainter::DrawRect(float x, float y, float w, float h)
{
	if (!cr)
		return;
	cairo_rectangle(cr, x, y, w, h);
	cairo_fill(cr);
}

void HuiPainter::DrawRect(const rect &r)
{
	if (!cr)
		return;
	cairo_rectangle(cr, r.x1, r.y1, r.width(), r.height());
	cairo_fill(cr);
}

void HuiPainter::DrawCircle(float x, float y, float radius)
{
	if (!cr)
		return;
	cairo_arc(cr, x, y, radius, 0, 2 * pi);
	cairo_fill(cr);
}

void HuiPainter::DrawImage(float x, float y, const Image &image)
{
#ifdef _X_USE_IMAGE_
	if (!cr)
		return;
	image.setMode(Image::ModeBGRA);
	cairo_pattern_t *p = cairo_get_source(cr);
	cairo_pattern_reference(p);
	cairo_surface_t *img = cairo_image_surface_create_for_data((unsigned char*)image.data.data,
                                                         CAIRO_FORMAT_ARGB32,
                                                         image.width,
                                                         image.height,
	    image.width * 4);

	cairo_set_source_surface (cr, img, x, y);
	cairo_paint(cr);
	cairo_surface_destroy(img);
	cairo_set_source(cr, p);
	cairo_pattern_destroy(p);
#endif
}

void HuiPainter::SetFont(const string &font, float size, bool bold, bool italic)
{
	if (!cr)
		return;
	//cairo_select_font_face(cr, "serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	//cairo_set_font_size(cr, size);
	if (font.num > 0)
		cur_font = font;
	if (size > 0)
		cur_font_size = size;
	cur_font_bold = bold;
	cur_font_italic = italic;
}

void HuiPainter::SetFontSize(float size)
{
	if (!cr)
		return;
	//cairo_set_font_size(cr, size);
	cur_font_size = size;
}

void HuiPainter::SetAntialiasing(bool enabled)
{
	if (!cr)
		return;
	if (enabled)
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_DEFAULT);
	else
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
}

#endif
