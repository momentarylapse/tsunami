/*
 * pdf.cpp
 *
 *  Created on: 22.04.2018
 *      Author: michi
 */

#include "pdf.h"
#include "ttf.h"
#include "../base/iter.h"
#include "../image/image.h"
#include "../image/Painter.h"
#include "../math/rect.h"
#include "../math/vec2.h"
#include "../os/file.h"
#include "../os/filesystem.h"
#include "../os/msg.h"

using namespace os::fs;

namespace pdf {



PagePainter::PagePainter(Parser* _parser, Page *_page) {
	parser = _parser;
	page = _page;
	width = page->width;
	height = page->height;
	current_color = Black;
	line_width = 1;
	font_size = 12;
	font_name = "Helvetica";
	filling = true;
	text_x = text_y = 0;
	set_color(Black);
}

PagePainter::~PagePainter() {
}

void PagePainter::set_color(const color& c) {
	page->content += format("     %.2f %.2f %.2f RG\n", c.r, c.g, c.b);
	page->content += format("     %.2f %.2f %.2f rg\n", c.r, c.g, c.b);
#if 0
	if (c.a != current_color.a) {
		page->content += format("     %.2f CA\n", c.a);
		page->content += format("     %.2f ca\n", c.a);
		page->content += "false AIS\n";
		//page->content += "Overlay BM\n";
	}
#endif
	current_color = c;
}

void PagePainter::set_font(const string& font, float size, bool bold, bool italic) {
	font_name = font;
	font_size = size;
}

void PagePainter::set_font_size(float size) {
	font_size = size;
}

void PagePainter::set_antialiasing(bool enabled) {
}

void PagePainter::set_line_width(float w) {
	line_width = w;
	page->content += format("     %.1f w\n", w);
}

void PagePainter::set_line_dash(const Array<float>& dash, float offset) {
	page->content += "     [";
	for (float d: dash)
		page->content += format("%.0f ", d);
	page->content += format("] %.0f d\n", offset);
}

void PagePainter::set_fill(bool fill) {
	filling = fill;
}

void PagePainter::set_clip(const rect& r) {
}

void PagePainter::draw_point(const vec2 &p) {
}

void PagePainter::draw_line(const vec2 &a, const vec2 &b) {
	page->content += format("     %.1f %.1f m\n", a.x, height-a.y);
	page->content += format("     %.1f %.1f l\n", b.x, height-b.y);
	page->content += "     S\n";
}

void PagePainter::draw_lines(const Array<vec2>& p) {
	if (p.num < 2)
		return;
	page->content += format("     %.1f %.1f m\n", p[0].x, height-p[0].y);
	for (int i=1; i<p.num; i++)
		page->content += format("     %.1f %.1f l\n", p[i].x, height-p[i].y);
	page->content += "     S\n";
}

void PagePainter::draw_polygon(const Array<vec2>& p) {
	if (p.num < 2)
		return;
	page->content += format("     %.1f %.1f m\n", p[0].x, height-p[0].y);
	for (int i=1; i<p.num; i++)
		page->content += format("     %.1f %.1f l\n", p[i].x, height-p[i].y);
	if (filling)
		page->content += "     f\n";
	else
		page->content += "     s\n";
}

void PagePainter::draw_rect(const rect& r) {
	page->content += format("     %.1f %.1f %.1f %.1f re\n", r.x1, height-r.y2, r.width(), r.height());
	if (filling)
		page->content += "     f\n";
	else
		page->content += "     S\n";
}


// TODO optimize
void PagePainter::draw_circle(const vec2 &c, float radius) {
	vec2 p[12];
	float rr = radius * 0.6f;
	p[0] = vec2(c.x,        height-c.y-radius);
	p[1] = vec2(c.x+rr,     height-c.y-radius);
	p[2] = vec2(c.x+radius, height-c.y-rr);
	p[3] = vec2(c.x+radius, height-c.y);
	p[4] = vec2(c.x+radius, height-c.y+rr);
	p[5] = vec2(c.x+rr,     height-c.y+radius);
	p[6] = vec2(c.x,        height-c.y+radius);
	p[7] = vec2(c.x-rr,     height-c.y+radius);
	p[8] = vec2(c.x-radius, height-c.y+rr);
	p[9] = vec2(c.x-radius, height-c.y);
	p[10]= vec2(c.x-radius, height-c.y-rr);
	p[11]= vec2(c.x-rr,     height-c.y-radius);
	page->content += format("     %.1f %.1f m\n", p[0].x, p[0].y);
	for (int i=0; i<12; i+=3)
		page->content += format("     %.1f %.1f %.1f %.1f %.1f %.1f c\n", p[i+1].x, p[i+1].y, p[i+2].x, p[i+2].y, p[(i+3)%12].x, p[(i+3)%12].y);
	if (filling)
		page->content += "     f\n";
	else
		page->content += "     S\n";
}

static string _pdf_str_filter(const string &str, FontData *fd) {
	auto xx = str.utf8_to_utf32();
	for (int &c: xx) {
		// ascii
		if (c < 0x80)
			continue;
		// Umlaut
		if (c == 228 or c == 196 or c == 246 or c == 214 or c == 252 or c == 220 or c == 223)
			continue;
		if (c == 0x266f)
			c = '#';
		else if (c == 0x266d)
			c = 'b';
		else
			c = '?';
	}
	return utf32_to_utf8(xx);
}

static string _pdf_str_encode(const string &str) {
	string x = str.escape().replace("(", "\\(").replace(")", "\\)");
	// https://apple.fandom.com/wiki/Mac-Roman_encoding
	x = x.replace(u8"ä", "\\212").replace(u8"ö", "\\232").replace(u8"ü", "\\237").replace(u8"ß", "\\247");
	x = x.replace(u8"Ä", "\\200").replace(u8"Ö", "\\205").replace(u8"Ü", "\\206");
	return x;
}

void PagePainter::draw_str(const vec2 &_p, const string& str) {
	vec2 p = vec2(_p.x, height - _p.y - font_size*0.8f);
	float dx = p.x - text_x;
	float dy = p.y - text_y;
	auto f = parser->font_get(font_name);
	string s = _pdf_str_encode(_pdf_str_filter(str, f));
	page->content += format("     %s %.1f Tf\n     %.2f %.2f Td\n     (%s) Tj\n", f->internal_name, font_size, dx, dy, s);

	text_x = p.x;
	text_y = p.y;
}

float PagePainter::get_str_width(const string& str) {
	auto f = parser->font_get(font_name);
	string s = _pdf_str_filter(str, f);
	if (f->true_type) {
		float dx = 0;
		auto codes = s.utf8_to_utf32();
		float scale = f->ttf->scale();
		for (int c: codes) {
			auto g = f->ttf->get(c);
			if (g)
				dx += g->width / 1000.0f;
		}
		//msg_write(format("www   %f", dx));
		return font_size * dx * scale;
	}
	return font_size * s.num * 0.5f;
}

void PagePainter::draw_image(const vec2 &d, const Image *image) {
}

void PagePainter::draw_mask_image(const vec2 &d, const Image *image) {
}

rect PagePainter::area() const {
	return rect(0, width, 0, height);
}

rect PagePainter::clip() const {
	return area();
}

Parser::Parser() {
	current_painter = nullptr;
	page_width = 595.276f;
	page_height = 841.89f;
}

Parser::~Parser() {
	if (current_painter)
		delete current_painter;
}

void Parser::__init__() {
	new(this) Parser;
}

void Parser::__delete__() {
	this->Parser::~Parser();
}


void Parser::set_page_size(float w, float h) {
	page_width = w;
	page_height = h;
}

Painter* Parser::add_page() {
	auto page = new Page;
	page->width = page_width;
	page->height = page_height;
	pages.add(page);
	if (current_painter)
		delete current_painter;
	return new PagePainter(this, page);
}


const Array<string> PDF_DEFAULT_FONTS = {"Times", "Courier", "Helvetica"};

FontData *Parser::font_get(const string &name) {
	for (auto &f: font_data)
		if (name == f.name)
			return &f;
	FontData fd;
	fd.name = name;
	fd.internal_name = format("/F%d", font_data.num+1);
	fd.true_type = false;
	fd.id = -1;
	fd.id_widths = -1;
	fd.id_descr = -1;
	fd.id_file = -1;

	if (!sa_contains(PDF_DEFAULT_FONTS, name)) {
		auto ff = ttf::find_ttf(fd.name);
		if (!ff.is_empty()) {
			fd.true_type = true;
			fd.file_contents = os::fs::read_binary(ff);
			//load_ttf(name);
			fd.ttf = new ttf::TTF;
			fd.ttf->load(ff);
			fd.widths = fd.ttf->get_widths();
		} else {
			msg_error("font not found: " + name);
		}
	}
	font_data.add(fd);
	return &font_data.back();
}

void Parser::save(const Path &filename) {
	auto f = os::fs::open(filename, "wb");
	Array<int> pos;
	int next_id = 1;
	auto mk_id = [&] {
		return next_id ++;
	};


	int id_catalog = mk_id();
	int id_outlines = mk_id();
	int id_pages = mk_id();
	Array<int> page_id;
	for (int i=0; i<pages.num; i++)
		page_id.add(mk_id());
	Array<int> stream_id;
	for (int i=0; i<pages.num; i++)
		stream_id.add(mk_id());

	int id_proc = mk_id();

	// preparing fonts
	for (auto&&  [i,fd]: enumerate(font_data)) {
		if (fd.true_type) {
			auto ff = ttf::find_ttf(fd.name);
			if (ff) {
				fd.id_file = mk_id();
				fd.file_contents = os::fs::read_binary(ff);
			}
			fd.id_descr = mk_id();
			fd.id_widths = mk_id();
		}
		fd.id = mk_id();
	}
	int id_xref = mk_id();


	pos.resize(id_xref);

	auto write_obj = [&](FileStream *f, int id, const string &contents) {
		pos[id] = f->get_pos();
		f->write(format("%d 0 obj\n", id));
		f->write(contents + "\n");
		f->write("endobj\n");
	};

	auto mk_dict = [](const Array<string> &lines) {
		if (lines.num == 1)
			return "<< " + lines[0] + " >>";
		return "<< " + implode(lines, "\n   ") + "\n>>";
	};

	Array<string> page_ref;
	for (int i=0; i<pages.num; i++)
		page_ref.add(format("%d 0 R", page_id[i]));


	f->write("%PDF-1.4\n");
	write_obj(f, id_catalog, mk_dict({
		"/Type /Catalog",
		format("/Outlines %d 0 R", id_outlines),
		format("/Pages %d 0 R", id_pages)}));
	write_obj(f, id_outlines, mk_dict({
		"/Type Outlines",
		"/Count 0"}));
	write_obj(f, id_pages, mk_dict({"/Type /Pages",
		"/Kids [" + implode(page_ref, " ") + "]",
		format("/Count %d\n>>", pages.num)}));


	// pages
	for (auto&& [i,p]: enumerate(pages)) {
		Array<string> font_refs;
		for (auto &fd: font_data)
			font_refs.add(format("%s %d 0 R", fd.internal_name, fd.id));
		write_obj(f, page_id[i], mk_dict({
			"/Type /Page",
			"/Parent 3 0 R",
			format("/MediaBox [0 0 %.3f %.3f]", p->width, p->height),
			format("/Contents %d 0 R", stream_id[i]),
			format("/Resources << /ProcSet %d 0 R\n                 /Font << ", id_proc) + implode(font_refs, " ") + " >>\n              >>"}));
	}

	// streams
	for (auto&& [i,p]: enumerate(pages)) {
		write_obj(f, stream_id[i], mk_dict({format("/Length %d", p->content.num)}) + format("\nstream\n  BT\n%s  ET\nendstream", p->content));
	}

	// proc
	write_obj(f, id_proc, "[/PDF]");

	// fonts
	for (auto &fd: font_data) {

		if (fd.id_file > 0) {
			write_obj(f, fd.id_file, mk_dict({
				"/Filter /ASCIIHexDecode",
				format("/Length %d", fd.file_contents.num*3)}) + "\nstream\n" + fd.file_contents.hex().replace("."," ") + " >\nendstream");
		}

		if (fd.id_descr > 0) {
			Array<string> dict = {
					"/Type /FontDescriptor",
					format("/FontName %s", fd.internal_name),
					"/Flags 4",
					"/FontBBox [-500 -300 1300 900]",
					"/ItalicAngle 0",
					"/Ascent 900",
					"/Descent -200",
					"/CapHeight 900",
					"/StemV 80"};
			if (fd.id_file > 0)
				dict.add(format("/FontFile2 %d 0 R", fd.id_file));
			write_obj(f, fd.id_descr, mk_dict(dict));
		}
		if (fd.id_widths > 0)
			write_obj(f, fd.id_widths, str(fd.widths).replace(",", ""));

		Array<string> dict = {"/Type /Font"};
		if (fd.true_type) {
			dict.add("/Subtype /TrueType");
			dict.add("/FirstChar 0");
			dict.add("/LastChar 255");
		} else {
			dict.add("/Subtype /Type1");
		}
		dict.add(format("/Name %s", fd.internal_name));
		dict.add(format("/BaseFont /%s", fd.name));
		dict.add("/Encoding /MacRomanEncoding");
		if (fd.id_widths > 0)
			dict.add(format("/Widths %d 0 R", fd.id_widths));
		if (fd.id_descr > 0)
			dict.add(format("/FontDescriptor %d 0 R", fd.id_descr));
		write_obj(f, fd.id, mk_dict(dict));
	}

	int xref_pos = f->get_pos();
	f->write("xref\n");
	f->write(format("0 %d\n", id_xref));
	f->write("0000000000 65535 f\n");
	for (int p: pos)
		f->write(format("%010d 00000 n\n", p));
	f->write("trailer\n");
	f->write(mk_dict({
		format("/Size %d", id_xref),
		format("/Root %d 0 R", id_catalog)}) + "\n");
	f->write("startxref\n");
	f->write(format("%d\n", xref_pos));

	f->write("%%EOF\n");
	delete f;
}

}
