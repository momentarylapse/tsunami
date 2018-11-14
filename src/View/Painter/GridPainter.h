/*
 * GridPainter.h
 *
 *  Created on: 12.11.2018
 *      Author: michi
 */

#ifndef SRC_VIEW_PAINTER_GRIDPAINTER_H_
#define SRC_VIEW_PAINTER_GRIDPAINTER_H_

#include "../../lib/base/base.h"
#include "../../lib/math/rect.h"
#include "../../lib/image/color.h"

class Painter;
class rect;
class color;
class AudioView;
class ViewPort;
class Song;

class GridColors
{
public:
	color bg, bg_sel;
	color fg, fg_sel;
};

class GridPainter
{
public:
	GridPainter(AudioView *view);
	void draw_time(Painter *c);
	void draw_time_numbers(Painter *c);
	void draw_bars(Painter *c, int beat_partition = 0);
	void draw_bar_numbers(Painter *c);
	void draw_whatever(Painter *c, int beat_partition = 0);

	void set_context(const rect &area, const GridColors &c);

	AudioView *view;
	ViewPort *cam;
	Song *song;
	rect area;
	GridColors colors;
};

#endif /* SRC_VIEW_PAINTER_GRIDPAINTER_H_ */
