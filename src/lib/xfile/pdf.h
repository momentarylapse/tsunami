/*
 * pdf.h
 *
 *  Created on: 22.04.2018
 *      Author: michi
 */

#ifndef SRC_LIB_XFILE_PDF_H_
#define SRC_LIB_XFILE_PDF_H_

#include "../base/base.h"
#include "../base/pointer.h"
#include "../image/Painter.h"

class Path;
class Painter;
class Image;
class color;
class rect;
class complex;
class File;

namespace pdf {

class Parser;


struct Page {
	float width, height;
	string content;
};


class TTF;

struct FontData {
	bool true_type;
	string name, internal_name;
	int id, id_widths, id_descr, id_file;
	Array<int> widths;
	string file_contents;
	shared<TTF> ttf;
};

class PagePainter : public ::Painter {
public:
	PagePainter(Parser *parser, Page *page);
	~PagePainter() override;
	void _cdecl set_color(const color &c) override;
	void _cdecl set_font(const string &font, float size, bool bold, bool italic) override;
	void _cdecl set_font_size(float size) override;
	void _cdecl set_antialiasing(bool enabled) override;
	void _cdecl set_line_width(float w) override;
	void _cdecl set_roundness(float radius) override {}
	void _cdecl set_line_dash(const Array<float> &dash, float offset) override;
	void _cdecl set_fill(bool fill) override;
	void _cdecl set_clip(const rect &r) override;
	void _cdecl draw_point(float x, float y) override;
	void _cdecl draw_line(float x1, float y1, float x2, float y2) override;
	void _cdecl draw_lines(const Array<complex> &p) override;
	void _cdecl draw_polygon(const Array<complex> &p) override;
	void _cdecl draw_rect(float x1, float y1, float w, float h) override;
	void _cdecl draw_rect(const rect &r) override;
	void _cdecl draw_circle(float x, float y, float radius) override;
	void _cdecl draw_str(float x, float y, const string &str) override;
	float _cdecl get_str_width(const string &str) override;
	void _cdecl draw_image(float x, float y, const Image *image) override;
	void _cdecl draw_mask_image(float x, float y, const Image *image) override;
	rect area() const override;
	rect clip() const override;

	Parser *parser;
	Page *page;

	color *col;
	float font_size;
	string font_name;
	float line_width;
	bool filling;

	float text_x, text_y;
};

class Parser {
	friend class PagePainter;
public:
	Parser();
	~Parser();

	void __init__();
	void __delete__();

	void set_page_size(float width, float height);

	Painter *add_page();
	void save(const Path &filename);

private:
	Array<Page*> pages;
	//Array<string> font_names;
	float page_width, page_height;

	Painter *current_painter;

	Array<FontData> font_data;
	//int font_id(const string &name);
	FontData *font_get(const string &name);
};

void add_font_directory(const Path &dir);

}




#endif /* SRC_LIB_XFILE_PDF_H_ */
