/*
 * pdf.cpp
 *
 *  Created on: 22.04.2018
 *      Author: michi
 */

#include "pdf.h"
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

Array<Path> font_paths = {"./"};


class TTF : public Sharable<base::Empty> {
public:


	struct BEUShort {
		unsigned char c[2];
		int _int() const {
			unsigned int a = c[0];
			unsigned int b = c[1];
			return (a << 8) + b;
		}
	};
	struct BESShort {
		unsigned char c[2];
		int _int() const {
			unsigned int a = c[0];
			unsigned int b = c[1];
			unsigned int r = (a << 8) + b;
			if ((r & 0x8000) != 0)
				r -= 1<<16;
			return r;
		}
	};
	struct BELong {
		unsigned char x[4];
		int _int() const {
			unsigned int a = x[0];
			unsigned int b = x[1];
			unsigned int c = x[2];
			unsigned int d = x[3];
			return (a << 24) + (b << 16) + (c << 8) + d;
		}
	};

	struct Fixed {
		BEUShort a, b;
	};


	static int readUS(FileStream *f) {
		BEUShort t;
		f->read(&t, 2);
		return t._int();
	}

	static int readSS(FileStream *f) {
		BESShort t;
		f->read(&t, 2);
		return t._int();
	}

	static int readUB(FileStream *f) {
		int t = 0;
		f->read(&t, 1);
		return t;
	}

	static int readL(FileStream *f) {
		BELong t;
		f->read(&t, 4);
		return t._int();
	}

	struct TableDirectory {
		Fixed version;
		BEUShort num_tables;
		BEUShort seach_range;
		BEUShort entry_selector;
		BEUShort range_shift;
	};
	TableDirectory td;

	struct TableDirectoryEntry {
		BELong tag, chksum, offset, length;
		string name() {
			string s;
			s.add(tag.x[0]);
			s.add(tag.x[1]);
			s.add(tag.x[2]);
			s.add(tag.x[3]);
			return s;
		}
	};
	Array<TableDirectoryEntry> tdentries;

	struct Header {
		Fixed version;
		Fixed revision;
		BELong chksm;
		BELong magic;
		BEUShort flags;
		BEUShort units_per_em;
		// more
	};
	Header head;


	struct MapHeader {
		BEUShort version, num_tables;
	};
	MapHeader mh;

	struct MapTable {
		int platform, encoding;
		int offset;
		int format;

		// format 4
		Array<int> end_code;
		Array<int> start_code;
		Array<int> id_delta;
		Array<int> id_range_offset;

		// format 6
		int first_code;
		Array<int> glyph_id_array;

		int map(int code) {
			if (format == 4) {
				for (int i=0; i<end_code.num; i++)
					if (code >= start_code[i] and code <= end_code[i])
						return (code + id_delta[i]) & 0x0000ffff;
				return -1;
			} else if (format == 6) {
				if (code < first_code or code >= (first_code + glyph_id_array.num))
					return -1;
				return glyph_id_array[code - first_code];
			}
			return -1;
		}
	};
	Array<MapTable> map_tables;

	struct Glyph {
		int code;
		int xmin, ymin, xmax, ymax;
		//Contour[] contours;
		Array<int> codes;
		float lsb, width;
	};
	Array<Glyph> glyphs;

	Array<int> lsb, advance;


	TableDirectoryEntry* get_table(const string &tag) {
		for (auto &ee: tdentries)
			if (tag == ee.name())
				return &ee;
		return nullptr;
	}

	void read_table_directory(FileStream *f) {
		f->read(&td, 12);
		int n = td.num_tables._int();
		if (n > 1000)
			throw Exception("argh");
		for (int i=0; i<n; i++) {
			TableDirectoryEntry e;
			f->read(&e, 16);
			tdentries.add(e);
		}

		//for (auto &ee: tdentries)
		//	msg_write(format("%s  %d", ee.name(), ee.offset._int()));
	}
	bool read_head(FileStream *f) {
		auto te = get_table("head");
		if (!te) {
			msg_error("no head table");
			return false;
		}
		f->set_pos(te->offset._int());
		//msg_write("head-----------------------------");

		f->read(&head, 32);
		//msg_write(head.units_per_em._int());
		return true;
	}



	bool try_read_hhead(FileStream *f) {
		auto te = get_table("hhea");
		if (!te)
			return false;
		f->set_pos(te->offset._int());
		//msg_write("hhead-----------------------------");

		f->seek(34);
		int n = readUS(f);
		//msg_write(n);
		lsb.resize(n);
		advance.resize(n);
		return true;
	}

	bool try_read_hmetrix(FileStream *f) {
		auto te = get_table("hmtx");
		if (!te)
			return false;
		f->set_pos(te->offset._int());
		//msg_write("hmetrix-----------------------------");
		for (int i=0; i<lsb.num; i++) {
			advance[i] = readUS(f);
			lsb[i] = readSS(f);
		}
		//msg_write(ia2s(advance));
		return true;
	}


	bool read_mapping(FileStream *f) {
		auto te = get_table("cmap");
		if (!te) {
			msg_error("no cmap table");
			return false;
		}
		f->set_pos(te->offset._int());

		f->read(&mh, 4);
		int n = mh.num_tables._int();
		//msg_write("map-----------------------------");
		//msg_write(i2h(f->get_pos(), 4));
		//msg_write mh.version.int();
		//msg_write n;
		if (n > 100000) {
			msg_error("too many mapping tables");
			return false;
		}

		for (int i=0; i<n; i++) {
			MapTable t;
			t.platform = readUS(f);
			t.encoding = readUS(f);
			t.offset = readL(f);
			map_tables.add(t);
		}

		for (auto &m: map_tables) {
			/*msg_write("--------------");
			msg_write(m.offset);
			msg_write(m.platform);
			msg_write(m.encoding);*/
			f->set_pos(te->offset._int() + m.offset);
			m.format = readUS(f);

			if (m.format == 0) {
				int length = readUS(f);
				//msg_write("l {{length}}");
				int version = readUS(f);
				for (int i=0; i<256; i++)
					m.glyph_id_array.add(readUB(f));
				//msg_write m.glyph_id_array;
			} else if (m.format == 4) {
				int length = readUS(f);
				//msg_write("l {{length}}");
				int version = readUS(f);
				int seg_count = readUS(f) / 2;
				//msg_write("s {{seg_count}}");
				int search_range = readUS(f);
				int entry_selector = readUS(f);
				int range_shift = readUS(f);
				for (int i=0; i<seg_count; i++)
					m.end_code.add(readUS(f));
				readUS(f);
				for (int i=0; i<seg_count; i++)
					m.start_code.add(readUS(f));
				for (int i=0; i<seg_count; i++)
					m.id_delta.add(readUS(f));
				for (int i=0; i<seg_count; i++)
					m.id_range_offset.add(readUS(f));
				int rs = seg_count * 8 + 16;
				//msg_write("r: {{length - rs}}");
				int na = (length - rs) / 2;
				for (int i=0; i<na; i++)
					m.glyph_id_array.add(readUS(f));
			} else if (m.format == 6) {
				int length = readUS(f);
				//msg_write("l {{length}}");
				int version = readUS(f);
				m.first_code = readUS(f);
				int entry_count = readUS(f);
				for (int i=0; i<entry_count; i++)
					m.glyph_id_array.add(readUS(f));
				//msg_write m.glyph_id_array

			} else if (m.format == 12) {
				readUS(f); // "padding"
				int length = readL(f);
				//msg_write("l {{length}}");
				int version = readL(f);

				int group_count = readL(f) / 2;
				//msg_write("s {{group_count}}");

				for (int i=0; i<group_count; i++) {
					int start_code = readL(f);
					m.start_code.add(start_code);
					m.end_code.add(readL(f));
					m.id_delta.add(readL(f) - start_code);
				}

			} else {
				msg_error(format("unhandled format: %d", m.format));
				return false;
			}
		}


		return true;
	}

	void map_code(int index, int code) {
		if (index < glyphs.num) {
			glyphs[index].code = code;
			if (glyphs[index].codes.find(code) >= 0)
				glyphs[index].codes.add(code);
			//msg_write(format("E: %d => %d", index, code));
		}
	}

	void update_codes() {

		for (auto &m: map_tables) {
			if (m.format == 6) {
				//msg_write("----------------------- 666");
				for (auto&& [i,n]: enumerate(m.glyph_id_array))
					map_code(n, m.first_code + i);
			} else if (m.format == 4) {
				//msg_write("----------------------- 444");
				//msg_write(m.end_code.num);
				for (int i=0; i<m.end_code.num; i++) {
					/*msg_write("----");
					msg_write(m.start_code[i]);
					msg_write(m.end_code[i]);
					msg_write(m.id_delta[i]);*/
					for (int code=m.start_code[i]; code<m.end_code[i]+1; code++) {
						int n;
						if (m.id_range_offset[i] == 0) {
							n = (code + m.id_delta[i]) & 0x0000ffff;
						} else {
							int index = (code - m.start_code[i]) + m.id_range_offset[i]/2 - (m.id_range_offset.num - i);
							n = m.glyph_id_array[index];
						}
						map_code(n, code);
					}
				}
			} else if (m.format == 12) {
				//msg_write("----------------------- 12 12 12");
				//msg_write(m.end_code.num);
				for (int i=0; i<m.end_code.num; i++) {
					/*msg_write("----");
					msg_write(m.start_code[i]);
					msg_write(m.end_code[i]);
					msg_write(m.id_delta[i]);*/
					for (int code=m.start_code[i]; code<m.end_code[i]+1; code++)
						map_code(code + m.id_delta[i], code);
				}
			} else {
				msg_error(format("---------------- UNHANDLED %d", m.format));
			}
		}
	}


	bool read_glyphs(FileStream *f) {
		auto te = get_table("glyf");
		if (!te) {
			msg_error("no glyf table");
			return false;
		}
		f->set_pos(te->offset._int());
		int _max = te->offset._int() + te->length._int() - 32;

		Glyph gg;
		glyphs.add(gg);
		glyphs.add(gg);
		glyphs.add(gg);

		/*while (f->get_pos() < _max) {
			Glyph g;
			if (!read_glyph(g))
				return false;
			glyphs.add(g);
		}*/
		glyphs.resize(_max + 3);
		//print("Glyphs: {{len(glyphs)}}");


		return true;
	}

	void update_size() {
		float scale = 1.0 / head.units_per_em._int();
		float w = 0;
		for (auto&& [i,g]: enumerate(glyphs)) {
			g.lsb = 0;
			g.width = g.xmax - g.xmin;
			if (i < lsb.num) {
				g.lsb = lsb[i];
				g.width = advance[i];
				w = g.width;
			} else if (lsb.num > 0) {
				g.width = w;
			}
		}
	}

	void load(const Path &filename) {
		auto f = os::fs::open(filename, "rb");

		read_table_directory(f);
		read_head(f);
		read_mapping(f);
		try_read_hhead(f);
		try_read_hmetrix(f);
		read_glyphs(f);

		update_codes();
		update_size();

		delete f;
	}

	int map(int code) {
		for (auto &m: map_tables) {
			int n = m.map(code);
			if (n >= glyphs.num) {
				msg_error("mapping error: {{code}} => {{n}}");
				return -1;
			}
			if (n >= 0)
				return n;
		}
		return -1;
	}

	Glyph* get(int code) {
		//print("get {{code}}")
		int n = map(code);
		//print(n);
		if (n < 0)
			return &glyphs[0];
		return &glyphs[n];
	}

	float scale() const {
		return 1000.0f / (float)head.units_per_em._int();
	}

	Array<int> get_widths() {
		float _scale = scale();
		Array<int> widths;
		for (int code=0; code<256; code++) {
			auto g = get(code);
			widths.add((int)(g->width * _scale));
		}
		return widths;
	}
};

Path find_ttf(const string &name) {
	string fname = name + ".ttf";
	for (auto &dir: font_paths)
		if (os::fs::exists(dir << fname)) {
			return dir << fname;
		}
	return "";
}

void load_ttf(const string &name) {
	auto filename = find_ttf(name);
	if (!filename.is_empty()) {
		//msg_write(filename.str());
		TTF ttf;
		ttf.load(filename);
	} else {
		msg_error(format("ttf font not found: %s", name));
	}
}



PagePainter::PagePainter(Parser* _parser, Page *_page) {
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

PagePainter::~PagePainter() {
	delete col;
}

void PagePainter::set_color(const color& c) {
	*col = c;
	page->content += format("     %.2f %.2f %.2f RG\n", c.r, c.g, c.b);
	page->content += format("     %.2f %.2f %.2f rg\n", c.r, c.g, c.b);
	//page->content += format("     %.2f CA\n", c.a);
	//page->content += format("     %.2f ca\n", c.a);
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
		auto ff = find_ttf(fd.name);
		if (!ff.is_empty()) {
			fd.true_type = true;
			fd.file_contents = os::fs::read_binary(ff);
			//load_ttf(name);
			fd.ttf = new TTF;
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
			auto ff = find_ttf(fd.name);
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
			write_obj(f, fd.id_widths, ia2s(fd.widths).replace(",", ""));

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


void add_font_directory(const Path &dir) {
	font_paths.add(dir);
}

}
