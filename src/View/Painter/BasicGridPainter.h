/*
 * BasicGridPainter.h
 *
 *  Created on: Apr 16, 2021
 *      Author: michi
 */

#pragma once


#include "../../lib/base/base.h"
#include "../../lib/base/pointer.h"
#include "../../lib/math/rect.h"
#include "../../lib/image/color.h"


class Painter;
class rect;
class color;

class GridColors {
public:
	color bg, bg_sel;
	color fg, fg_sel;
};


class BasicGridPainter {
public:
	BasicGridPainter();


	void set_context(const rect &area, const GridColors &c);

	struct Tick {
		float x, v;
		int weight;
	};
	struct TickLabel : Tick {
		string label;
	};
	float weights[8];

	Array<Tick> ticks;
	Array<TickLabel> labels;

	void plan_linear(float v0, float v1, float min_dist);

	void draw(Painter *p);
	void draw_empty_background(Painter *p);

	bool horizontal;
	rect area;
	double min_grid_dist;
	GridColors colors;
};
