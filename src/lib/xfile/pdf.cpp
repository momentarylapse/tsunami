/*
 * pdf.cpp
 *
 *  Created on: 22.04.2018
 *      Author: michi
 */

#include "pdf.h"
#include "../image/image.h"
#include "../image/Painter.h"
#include "../math/math.h"
#include "../file/file.h"

namespace pdf
{




PagePainter::PagePainter(Parser* _parser, Page *_page)
{
	parser = _parser;
	page = _page;
	width = page->width;
	height = page->height;
	col = new color(1,0,0,0);
	line_width = 1;
	font_size = 12;
	font_name = "Helvetica";
	filling = true;
	text_x = text_y = 0;
	set_color(Black);
}

PagePainter::~PagePainter()
{
	delete col;
}

void PagePainter::set_color(const color& c)
{
	*col = c;
	page->content += format("     %.2f %.2f %.2f RG\n", c.r, c.g, c.b);
	page->content += format("     %.2f %.2f %.2f rg\n", c.r, c.g, c.b);
	//page->content += format("     %.2f CA\n", c.a);
	//page->content += format("     %.2f ca\n", c.a);
}

void PagePainter::set_font(const string& font, float size, bool bold, bool italic)
{
	font_name = font;
	font_size = size;
}

void PagePainter::set_font_size(float size)
{
	font_size = size;
}

void PagePainter::set_antialiasing(bool enabled)
{
}

void PagePainter::set_line_width(float w)
{
	line_width = w;
	page->content += format("     %.1f w\n", w);
}

void PagePainter::set_line_dash(const Array<float>& dash, float offset)
{
}

void PagePainter::set_fill(bool fill)
{
	filling = fill;
}

void PagePainter::set_clip(const rect& r)
{
}

void PagePainter::draw_point(float x, float y)
{
}

void PagePainter::draw_line(float x1, float y1, float x2, float y2)
{
	page->content += format("     %.1f %.1f m\n", x1, height-y1);
	page->content += format("     %.1f %.1f l\n", x2, height-y2);
	page->content += "     S\n";
}

void PagePainter::draw_lines(const Array<complex>& p)
{
}

void PagePainter::draw_polygon(const Array<complex>& p)
{
}

void PagePainter::draw_rect(float x1, float y1, float w, float h)
{
	draw_rect(rect(x1, x1+w, y1, y1+h));
	//page->content += format("     %.1f %.1f %.1f %.1f re\n", x1, height-y1-h, w, h);
}

void PagePainter::draw_rect(const rect& r)
{
	page->content += format("     %.1f %.1f %.1f %.1f re\n", r.x1, height-r.y2, r.width(), r.height());
	if (filling)
		page->content += "     f\n";
	else
		page->content += "     S\n";
}

void PagePainter::draw_circle(float x, float y, float radius)
{
	complex p[12];
	float rr = radius * 0.6f;
	p[0] = complex(x,        height-y-radius);
	p[1] = complex(x+rr,     height-y-radius);
	p[2] = complex(x+radius, height-y-rr);
	p[3] = complex(x+radius, height-y);
	p[4] = complex(x+radius, height-y+rr);
	p[5] = complex(x+rr,     height-y+radius);
	p[6] = complex(x,        height-y+radius);
	p[7] = complex(x-rr,     height-y+radius);
	p[8] = complex(x-radius, height-y+rr);
	p[9] = complex(x-radius, height-y);
	p[10]= complex(x-radius, height-y-rr);
	p[11]= complex(x-rr,     height-y-radius);
	page->content += format("     %.1f %.1f m\n", p[0].x, p[0].y);
	for (int i=0; i<12; i+=3)
		page->content += format("     %.1f %.1f %.1f %.1f %.1f %.1f c\n", p[i+1].x, p[i+1].y, p[i+2].x, p[i+2].y, p[(i+3)%12].x, p[(i+3)%12].y);
	if (filling)
		page->content += "     f\n";
	else
		page->content += "     S\n";
}

static string _pdf_str_filter(const string &str)
{
	string x = str.replace(u8"\u266f", "#").replace(u8"\u266d", "b");
	x = x.replace(u8"ä", "ae").replace(u8"ö", "oe").replace(u8"ü", "ue").replace(u8"ß", "ss");
	x = x.replace(u8"Ä", "Ae").replace(u8"Ö", "Oe").replace(u8"Ü", "Ue");
	return x;
}

void PagePainter::draw_str(float x, float y, const string& str)
{
	y = height - y - font_size*0.8f;
	float dx = x - text_x;
	float dy = y - text_y;
	int fid = parser->font_id(font_name);
	page->content += format("     /F%d %.1f Tf\n     %.2f %.2f Td\n     (%s) Tj\n", fid+1, font_size, dx, dy, _pdf_str_filter(str));

	text_x = x;
	text_y = y;
}

float PagePainter::get_str_width(const string& str)
{
	return font_size * str.num * 0.5f;
}

void PagePainter::draw_image(float x, float y, const Image *image)
{
}

void PagePainter::draw_mask_image(float x, float y, const Image *image)
{
}

rect PagePainter::area() const
{
	return rect(0, width, 0, height);
}

rect PagePainter::clip() const
{
	return area();
}

Parser::Parser()
{
	f = nullptr;
}

Parser::~Parser()
{
	if (f){
		end();
	}
}

Painter* Parser::add_page(float w, float h)
{
	auto page = new Page;
	page->width = w;
	page->height = h;
	pages.add(page);
	return new PagePainter(this, page);
}

int Parser::font_id(const string &name)
{
	for (int i=0; i<font_names.num; i++)
		if (name == font_names[i])
			return i;
	font_names.add(name);
	return font_names.num - 1;
}

void Parser::end()
{
	Array<int> pos;
	Array<int> page_id;
	for (int i=0; i<pages.num; i++)
		page_id.add(4 + i);
	Array<int> stream_id;
	for (int i=0; i<pages.num; i++)
		stream_id.add(4 + pages.num + i);
	int proc_id = 4 + pages.num * 2;
	int font0_id = proc_id + 1;
	int xref_id = font0_id + font_names.num;


	f->write_buffer("%PDF-1.4\n");
	pos.add(f->get_pos());
	f->write_buffer("1 0 obj\n");
	f->write_buffer("  << /Type /Catalog\n");
	f->write_buffer("     /Outlines 2 0 R\n");
	f->write_buffer("     /Pages 3 0 R\n");
	f->write_buffer("  >>\n");
	f->write_buffer("endobj\n");
	pos.add(f->get_pos());
	f->write_buffer("2 0 obj\n");
	f->write_buffer("  << /Type Outlines\n");
	f->write_buffer("     /Count 0\n");
	f->write_buffer("  >>\n");
	f->write_buffer("endobj\n");
	pos.add(f->get_pos());
	f->write_buffer("3 0 obj\n");
	f->write_buffer("  << /Type /Pages\n");
	f->write_buffer("     /Kids [");
	for (int i=0; i<pages.num; i++){
		if (i > 0)
			f->write_buffer(" ");
		f->write_buffer(format("%d 0 R", page_id[i]));
	}
	f->write_buffer("]\n");
	f->write_buffer("     /Count " + i2s(pages.num) + "\n");
	f->write_buffer("  >>\n");
	f->write_buffer("endobj\n");

	// pages
	foreachi(auto p, pages, i){
		pos.add(f->get_pos());
		f->write_buffer(format("%d 0 obj\n", page_id[i]));
		f->write_buffer("  << /Type /Page\n");
		f->write_buffer("     /Parent 3 0 R\n");
		f->write_buffer(format("     /MediaBox [0 0 %d %d]\n", p->width, p->height));
		f->write_buffer(format("     /Contents %d 0 R\n", stream_id[i]));
		f->write_buffer(format("     /Resources << /ProcSet %d 0 R\n", proc_id));
		string fff;
		for (int i=0; i<font_names.num; i++)
			fff += format("/F%d %d 0 R ", i+1, font0_id + i);
		f->write_buffer("                   /Font << " + fff + ">>\n");
		f->write_buffer("                >>\n");
		f->write_buffer("  >>\n");
	}

	// streams
	foreachi(auto p, pages, i){
		pos.add(f->get_pos());
		f->write_buffer(format("%d 0 obj\n", stream_id[i]));
		f->write_buffer(format("  << /Length %d >>\n", p->content.num));
		f->write_buffer("stream\n");
		f->write_buffer("  BT\n");
		f->write_buffer(p->content);
		f->write_buffer("  ET\n");
		f->write_buffer("endstream\n");
		f->write_buffer("endobj\n");
	}

	// proc
	pos.add(f->get_pos());
	f->write_buffer(format("%d 0 obj\n", proc_id));
	f->write_buffer("  [/PDF]\n");
	f->write_buffer("endobj\n");

	// fonts
	foreachi(string &font, font_names, i){
		pos.add(f->get_pos());
		f->write_buffer(format("%d 0 obj\n", font0_id + i));
		f->write_buffer("  << /Type /Font\n");
		f->write_buffer("     /Subtype /Type1\n");
		f->write_buffer(format("     /Name /F%d\n", i+1));
		f->write_buffer("     /BaseFont /" + font + "\n");
		f->write_buffer("     /Encoding /MacRomanEncoding\n");
		f->write_buffer("  >>\n");
		f->write_buffer("endobj\n");
	}

	int xref_pos = f->get_pos();
	f->write_buffer("xref\n");
	f->write_buffer(format("0 %d\n", xref_id));
	f->write_buffer("0000000000 65535 f\n");
	for (int p: pos)
		f->write_buffer(format("%010d 00000 n\n", p));
	f->write_buffer("trailer\n");
	f->write_buffer(format("  << /Size %d\n", xref_id));
	f->write_buffer("     /Root 1 0 R\n");
	f->write_buffer("  >>\n");
	f->write_buffer("startxref\n");
	f->write_buffer(format("%d\n", xref_pos));

	f->write_buffer("%%EOF\n");
	delete f;
	f = nullptr;
}

Parser* save(const Path &filename) {
	auto p = new Parser;
	p->f = FileCreate(filename);
	return p;
}


}
