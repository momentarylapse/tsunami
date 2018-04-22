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

namespace pdf
{
class Parser;


struct Page
{
	int width, height;
	string content;
};


class PagePainter : public ::Painter
{
public:
	PagePainter(Parser *parser, Page *page);
	virtual ~PagePainter();
	virtual void _cdecl setColor(const color &c);
	virtual void _cdecl setFont(const string &font, float size, bool bold, bool italic);
	virtual void _cdecl setFontSize(float size);
	virtual void _cdecl setAntialiasing(bool enabled);
	virtual void _cdecl setLineWidth(float w);
	virtual void _cdecl setRoundness(float radius){}
	virtual void _cdecl setLineDash(const Array<float> &dash, float offset);
	virtual void _cdecl setFill(bool fill);
	virtual void _cdecl clip(const rect &r);
	virtual void _cdecl drawPoint(float x, float y);
	virtual void _cdecl drawLine(float x1, float y1, float x2, float y2);
	virtual void _cdecl drawLines(const Array<complex> &p);
	virtual void _cdecl drawPolygon(const Array<complex> &p);
	virtual void _cdecl drawRect(float x1, float y1, float w, float h);
	virtual void _cdecl drawRect(const rect &r);
	virtual void _cdecl drawCircle(float x, float y, float radius);
	virtual void _cdecl drawStr(float x, float y, const string &str);
	virtual float _cdecl getStrWidth(const string &str);
	virtual void _cdecl drawImage(float x, float y, const Image &image);
	virtual void _cdecl drawMaskImage(float x, float y, const Image &image);
	virtual rect getArea();
	virtual rect getClip();

	Parser *parser;
	Page *page;

	color *col;
	float font_size;
	float line_width;
	bool filling;

	float text_x, text_y;
};

class Parser
{
public:
	Parser();
	~Parser();
	Painter *add_page(float width, float height);
	void end();

	File *f;
	Array<Page*> pages;
};

Parser *save(const string &filename);

}




#endif /* SRC_LIB_XFILE_PDF_H_ */
