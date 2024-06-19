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
class vec2;

namespace tsunami {

namespace SymbolRenderer {
	struct Symbol;

	Symbol *make_symbol(float size, bool bold, const string &s, bool);
	Symbol *get_symbol(float size, bool bold, const string &s);
	void draw(Painter *p, const vec2 &pos, float size, const string &s, bool bold = false, int align = 1);
	float width(Painter *p, float size, const string &s, bool bold = false);

	void enable(bool enabled);
}

}

#endif /* SRC_VIEW_HELPER_SYMBOLRENDERER_H_ */
