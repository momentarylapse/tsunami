/*
 * SymbolRenderer.cpp
 *
 *  Created on: 26.07.2018
 *      Author: michi
 */

#include "SymbolRenderer.h"
#include "../../lib/image/image.h"
#include "../../lib/image/Painter.h"
#include "../../lib/hui/hui.h"
#include <math.h>

#define ENABLED 1

namespace SymbolRenderer
{

struct Symbol
{
	float size;
	string text;
	Image im;

	Symbol(float _size, const string &_text)
	{
		text = _text;
		size = _size;
		im.create(size * text.num, size * 1.5f, color(0,0,0,0));
	}

	void render()
	{
		// render symbol into image
		Painter *p = hui::start_image_paint(im);
		p->set_color(White);
		p->set_font_size(size);
		p->draw_str(0, 0, text);
		hui::end_image_paint(im, p);
	}

	void compress()
	{
		// used area?
		int xmax = 0, ymax = 0;
		for (int x=0; x<im.width; x++)
			for (int y=0; y<im.height; y++)
				if (im.getPixel(x, y).r > 0){
					if (x > xmax)
						xmax = x;
					if (y > ymax)
						ymax = y;
				}

		// how much unused area?
		if (im.width * im.height - xmax * ymax < 15)
			return;

		// copy used part into smaller image
		//printf("compress %d %d -> %d %d\n", im.width, im.height, xmax, ymax);
		Image _im = im;
		im.create(xmax, ymax, Black);
		for (int x=0; x<xmax; x++)
			for (int y=0; y<ymax; y++)
				im.setPixel(x, y, _im.getPixel(x,y));
	}
};

static Array<Symbol*> symbols;



Symbol *make_symbol(float size, const string &s)
{
	//msg_write("new symbol " + f2s(size, 1) + " " + s);
	Symbol *sym = new Symbol(size, s);
	sym->render();
	sym->compress();

	symbols.add(sym);
	return sym;
}

Symbol *get_symbol(float size, const string &s)
{
	// already exists?
	for (Symbol *sym: symbols)
		if (sym->text == s and fabs(sym->size - size) <= 0.5f)
			return sym;

	// no -> make new
	return make_symbol(size, s);
}

void draw(Painter *p, float x, float y, float size, const string &s, int align)
{
#if ENABLED
	Symbol *sym = get_symbol(size, s);
	float dx = 0;
	if (align == 0)
		dx = - sym->size / 2;
	if (align == -1)
		dx = - sym->size;
	//p->drawImage(x + dx, y, sym->im);
	p->draw_mask_image(x + dx, y, sym->im);
#else
	p->setFontSize(size);
	float dx = 0;
	if (align == 0)
		dx = - p->getStrWidth(s) / 2;
	if (align == -1)
		dx = - p->getStrWidth(s);
	p->drawStr(x + dx, y, s);
#endif
}

};
