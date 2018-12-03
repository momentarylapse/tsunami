/*
 * SymbolRenderer.h
 *
 *  Created on: 26.07.2018
 *      Author: michi
 */

#ifndef SRC_VIEW_HELPER_SYMBOLRENDERER_H_
#define SRC_VIEW_HELPER_SYMBOLRENDERER_H_

#include "../../lib/base/base.h"

class Painter;


namespace SymbolRenderer
{
	struct Symbol;

	Symbol *make_symbol(float size, const string &s);
	Symbol *get_symbol(float size, const string &s);
	void draw(Painter *p, float x, float y, float size, const string &s, int align = 1);

	void enable(bool enabled);
};

#endif /* SRC_VIEW_HELPER_SYMBOLRENDERER_H_ */
