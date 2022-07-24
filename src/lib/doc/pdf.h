/*
 * pdf.h
 *
 *  Created on: 22.04.2018
 *      Author: michi
 */

#ifndef SRC_LIB_DOC_PDF_H_
#define SRC_LIB_DOC_PDF_H_

#include "../base/base.h"
#include "../base/pointer.h"
#include "../image/Painter.h"

class Path;
class Painter;
class Image;
class color;
class rect;
class vec2;

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
	void set_color(const color &c) override;
	void set_font(const string &font, float size, bool bold, bool italic) override;
	void set_font_size(float size) override;
	void set_antialiasing(bool enabled) override;
	void set_line_width(float w) override;
	void set_roundness(float radius) override {}
	void set_line_dash(const Array<float> &dash, float offset) override;
	void set_fill(bool fill) override;
	void set_clip(const rect &r) override;
	void draw_point(const vec2 &p) override;
	void draw_line(const vec2 &a, const vec2 &b) override;
	void draw_lines(const Array<vec2> &p) override;
	void draw_polygon(const Array<vec2> &p) override;
	void draw_rect(const rect &r) override;
	void draw_circle(const vec2 &c, float radius) override;
	void draw_str(const vec2 &p, const string &str) override;
	float get_str_width(const string &str) override;
	void draw_image(const vec2 &d, const Image *image) override;
	void draw_mask_image(const vec2 &d, const Image *image) override;
	bool allow_images() const override { return false; }
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




#endif /* SRC_LIB_DOC_PDF_H_ */
