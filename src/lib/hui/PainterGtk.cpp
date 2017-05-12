/*
 * HuiPainterGtk.cpp
 *
 *  Created on: 25.06.2013
 *      Author: michi
 */

#include "hui.h"
#include "../math/math.h"
#include "Controls/Control.h"
#include "Controls/ControlDrawingArea.h"
#include "Painter.h"

namespace hui
{

#ifdef HUI_API_GTK

Painter::Painter()
{
	win = NULL;
	cr = NULL;
	width = 0;
	height = 0;
	mode_fill = true;
	cur_font_bold = false;
	cur_font_italic = false;
	cur_font_size = -1;
	cur_font = "";
}

Painter::Painter(Panel *panel, const string &_id)
{
	win = panel->win;
	id = _id;
	cr = NULL;
	width = 0;
	height = 0;
	mode_fill = true;
	Control *c = panel->_get_control_(id);
	if (c){
		cr = (cairo_t*)((ControlDrawingArea*)c)->cur_cairo;
//		cr = gdk_cairo_create(gtk_widget_get_window(c->widget));

		//gdk_drawable_get_size(gtk_widget_get_window(c->widget), &hui_drawing_context.width, &hui_drawing_context.height);
		width = gdk_window_get_width(gtk_widget_get_window(c->widget));
		height = gdk_window_get_height(gtk_widget_get_window(c->widget));
		//hui_drawing_context.setFontSize(16);
		setFont("Sans", 16, false, false);
	}
}

Painter::~Painter()
{
	if (!cr)
		return;

//	cairo_destroy(cr);
	cr = NULL;
}

void Painter::setColor(const color &c)
{
	if (!cr)
		return;
	cairo_set_source_rgba(cr, c.r, c.g, c.b, c.a);
}

void Painter::setLineWidth(float w)
{
	if (!cr)
		return;
	cairo_set_line_width(cr, w);
}

void Painter::setLineDash(const Array<float> &dash, float offset)
{
	if (!cr)
		return;
	Array<double> d;
	for (int i=0; i<dash.num; i++)
		d.add(dash[i]);
	cairo_set_dash(cr, (double*)d.data, d.num, offset);
}

color gdk2color(GdkColor c)
{
	return color(1, (float)c.red / 65535.0f, (float)c.green / 65535.0f, (float)c.blue / 65535.0f);
}

color Painter::getThemeColor(int i)
{
	GtkStyleContext *sc = gtk_widget_get_style_context(win->window);
	int x = (i / 10);
	int y = (i % 10);
	GtkStateFlags state = (y == 1) ? GTK_STATE_FLAG_INSENSITIVE : GTK_STATE_FLAG_NORMAL;
	GdkRGBA c;
	if (x == 0)
		gtk_style_context_get_color(sc, state, &c);
	else if (x == 1)
		gtk_style_context_get_background_color(sc, state, &c);
	else if (x == 2)
		gtk_style_context_get_border_color(sc, state, &c);
	return color((float)c.alpha, (float)c.red, (float)c.green, (float)c.blue);
}

void Painter::clip(const rect &r)
{
	cairo_reset_clip(cr);
	cairo_rectangle(cr, r.x1, r.y1, r.width(), r.height());
	cairo_clip(cr);
}


void Painter::drawPoint(float x, float y)
{
	if (!cr)
		return;
}

void Painter::drawLine(float x1, float y1, float x2, float y2)
{
	if (!cr)
		return;
	cairo_move_to(cr, x1 + 0.5f, y1 + 0.5f);
	cairo_line_to(cr, x2 + 0.5f, y2 + 0.5f);
	cairo_stroke(cr);
}

void Painter::drawLines(const Array<complex> &p)
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

void Painter::drawPolygon(const Array<complex> &p)
{
	if (!cr)
		return;
	if (p.num == 0)
		return;
	cairo_move_to(cr, p[0].x, p[0].y);
	for (int i=1; i<p.num; i++)
		cairo_line_to(cr, p[i].x, p[i].y);
	cairo_close_path(cr);

	if (mode_fill)
		cairo_fill(cr);
	else
		cairo_stroke(cr);
}

void Painter::drawStr(float x, float y, const string &str)
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
	if (mode_fill){
		pango_cairo_show_layout(cr, layout);
	}else{
		pango_cairo_layout_path(cr, layout);
		cairo_stroke(cr);
	}
	g_object_unref(layout);

	//cairo_show_text(cr, str);
}

float Painter::getStrWidth(const string &str)
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

void Painter::drawRect(float x, float y, float w, float h)
{
	if (!cr)
		return;
	cairo_rectangle(cr, x, y, w, h);

	if (mode_fill)
		cairo_fill(cr);
	else
		cairo_stroke(cr);
}

void Painter::drawRect(const rect &r)
{
	if (!cr)
		return;
	cairo_rectangle(cr, r.x1, r.y1, r.width(), r.height());

	if (mode_fill)
		cairo_fill(cr);
	else
		cairo_stroke(cr);
}

void Painter::drawCircle(float x, float y, float radius)
{
	if (!cr)
		return;
	cairo_arc(cr, x, y, radius, 0, 2 * pi);

	if (mode_fill)
		cairo_fill(cr);
	else
		cairo_stroke(cr);
}

void Painter::drawImage(float x, float y, const Image &image)
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

	cairo_set_source_surface(cr, img, x, y);
	cairo_paint(cr);
	cairo_surface_destroy(img);
	cairo_set_source(cr, p);
	cairo_pattern_destroy(p);
#endif
}

void Painter::drawMaskImage(float x, float y, const Image &image)
{
#ifdef _X_USE_IMAGE_
	if (!cr)
		return;
	image.setMode(Image::ModeBGRA);
	cairo_surface_t *img = cairo_image_surface_create_for_data((unsigned char*)image.data.data,
                                                         CAIRO_FORMAT_ARGB32,
                                                         image.width,
                                                         image.height,
	    image.width * 4);

	cairo_mask_surface(cr, img, x, y);
	cairo_surface_destroy(img);
#endif
}

void Painter::setFont(const string &font, float size, bool bold, bool italic)
{
	if (!cr)
		return;
	//cairo_select_font_face(cr, "serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	//cairo_set_font_size(cr, size);
	if (font.num > 0)
		cur_font = font;
	if (size > 0)
		cur_font_size = (int)size;
	cur_font_bold = bold;
	cur_font_italic = italic;
}

void Painter::setFontSize(float size)
{
	if (!cr)
		return;
	//cairo_set_font_size(cr, size);
	cur_font_size = (int)size;
}

void Painter::setAntialiasing(bool enabled)
{
	if (!cr)
		return;
	if (enabled)
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_DEFAULT);
	else
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
}

void Painter::setFill(bool fill)
{
	mode_fill = fill;
}

};

#endif
