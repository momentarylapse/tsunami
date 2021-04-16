/*
 * GridPainter.h
 *
 *  Created on: 12.11.2018
 *      Author: michi
 */

#ifndef SRC_VIEW_PAINTER_GRIDPAINTER_H_
#define SRC_VIEW_PAINTER_GRIDPAINTER_H_

#include "BasicGridPainter.h"

class ViewPort;
class Song;
class ColorScheme;
class SongSelection;
class HoverData;

class GridPainter : public BasicGridPainter {
public:
	GridPainter(Song *song, ViewPort *cam, SongSelection *sel, HoverData *hover, ColorScheme &colors);
	void __init__(Song *song, ViewPort *cam, SongSelection *sel, HoverData *hover, ColorScheme &colors);

	void draw_empty_background(Painter *c);
	void draw_time(Painter *c);
	void draw_time_numbers(Painter *c);
	void draw_bars(Painter *c, int beat_partition = 0);
	void draw_bar_numbers(Painter *c);
	void draw_whatever(Painter *c, int beat_partition = 0);


	ViewPort *cam;
	Song *song;
	SongSelection *sel;
	HoverData *hover;
	ColorScheme &color_scheme;
};

#endif /* SRC_VIEW_PAINTER_GRIDPAINTER_H_ */
