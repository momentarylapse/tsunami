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
#include "../../lib/math/vector.h"
#include <math.h>

#define ENABLED 1

namespace SymbolRenderer
{

static bool enabled = ENABLED;

struct Symbol {
	float size;
	bool bold;
	string text;
	Image im;

	Symbol(float _size, bool _bold, const string &_text) {
		text = _text;
		size = _size;
		bold = _bold;
		im.create(size * text.num, size * 1.5f, color(0,0,0,0));
	}

	void render() {
		// render symbol into image
		Painter *p = hui::start_image_paint(&im);
		p->set_color(White);
		p->set_font_size(size);
		p->set_font("Sans", size, bold, false);
		p->draw_str({0, 0}, text);
		hui::end_image_paint(&im, p);
	}

	void compress() {
		// used area?
		int xmax = 0, ymax = 0;
		for (int x=0; x<im.width; x++)
			for (int y=0; y<im.height; y++)
				if (im.get_pixel(x, y).r > 0) {
					xmax = max(x+1, xmax);
					ymax = max(y+1, ymax);
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
				im.set_pixel(x, y, _im.get_pixel(x,y));
	}
};

static Array<Symbol*> symbols;



Symbol *make_symbol(float size, bool bold, const string &s) {
	//msg_write("new symbol " + f2s(size, 1) + " " + s);
	Symbol *sym = new Symbol(size, bold, s);
	sym->render();
	sym->compress();

	symbols.add(sym);
	return sym;
}

Symbol *get_symbol(float size, bool bold, const string &s) {
	// already exists?
	for (Symbol *sym: symbols)
		if ((sym->text == s) and (sym->bold == bold) and fabs(sym->size - size) <= 0.5f)
			return sym;

	// no -> make new
	return make_symbol(size, bold, s);
}

void draw(Painter *p, const vec2 &pos, float size, const string &s, bool bold, int align) {
	vec2 d = {0,0};
	if (enabled and p->allow_images()) {
		auto sym = get_symbol(size, bold, s);
		if (align == 0)
			d.x = - sym->im.width / 2;
		if (align == -1)
			d.x = - sym->im.width;
		//p->drawImage(x + dx, y, sym->im);
		p->draw_mask_image(pos + d, &sym->im);
	} else {
		p->set_font_size(size);
		if (align == 0)
			d.x = - p->get_str_width(s) / 2;
		if (align == -1)
			d.x = - p->get_str_width(s);
		p->draw_str(pos + d, s);
	}
}

float width(Painter *p, float size, const string &s, bool bold) {
	if (enabled) {
		auto sym = get_symbol(size, bold, s);
		return sym->im.width;
	} else {
		p->set_font_size(size);
		return p->get_str_width(s);
	}
}

void enable(bool _enabled) {
	enabled = _enabled;
}

};
