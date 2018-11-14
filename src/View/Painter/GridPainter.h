/*
 * GridPainter.h
 *
 *  Created on: 12.11.2018
 *      Author: michi
 */

#ifndef SRC_VIEW_PAINTER_GRIDPAINTER_H_
#define SRC_VIEW_PAINTER_GRIDPAINTER_H_

class Painter;
class rect;
class color;
class AudioView;
class ViewPort;
class Song;

class GridPainter
{
public:
	GridPainter(AudioView *view);
	void draw_grid_time(Painter *c, const rect &r, const color &fg, const color &fg_sel, const color &bg, const color &bg_sel, bool show_time);
	void draw_bar_numbers(Painter *c, const rect &area, const color &fg, const color &fg_sel, const color &bg, const color &bg_sel);
	void draw_grid_bars(Painter *c, const rect &area, const color &fg, const color &fg_sel, const color &bg, const color &bg_sel, int beat_partition);

	AudioView *view;
	ViewPort *cam;
	Song *song;
};

#endif /* SRC_VIEW_PAINTER_GRIDPAINTER_H_ */
