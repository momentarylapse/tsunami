/*
 * pdf.h
 *
 *  Created on: 22.04.2018
 *      Author: michi
 */

#ifndef SRC_LIB_XFILE_PDF_H_
#define SRC_LIB_XFILE_PDF_H_

#include "../base/base.h"
#include "../image/Painter.h"

class Painter;
class Image;
class color;
class rect;
class complex;
class File;

namespace pdf {

class Parser;


struct Page {
	int width, height;
	string content;
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
public:
	Parser();
	~Parser();
	Painter *add_page(float width, float height);
	void end();

	File *f;
	Array<Page*> pages;
	Array<string> font_names;

	int font_id(const string &name);
};

Parser *save(const string &filename);

}




#endif /* SRC_LIB_XFILE_PDF_H_ */
