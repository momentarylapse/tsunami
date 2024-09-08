/*
 * GridPainter.h
 *
 *  Created on: 12.11.2018
 *      Author: michi
 */

#ifndef SRC_VIEW_PAINTER_GRIDPAINTER_H_
#define SRC_VIEW_PAINTER_GRIDPAINTER_H_

#include <functional>
#include <base/optional.h>

#include "BasicGridPainter.h"

namespace tsunami {

class ViewPort;
class Song;
class ColorScheme;
class SongSelection;
class HoverData;
class Bar;

class GridPainter : public BasicGridPainter {
public:
	GridPainter(Song *song, ViewPort *cam, SongSelection *sel, ColorScheme &colors);
	void __init__(Song *song, ViewPort *cam, SongSelection *sel, ColorScheme &colors);

	void draw_empty_background(Painter *c);
	void draw_time(Painter *c);
	void draw_time_numbers(Painter *c);
	void draw_bars(Painter *c, base::optional<int> beat_partition = base::None);
	void draw_bar_numbers(Painter *c);
	void draw_whatever(Painter *c, base::optional<int> beat_partition = base::None);

	std::function<Bar*()> get_hover_bar;
	ViewPort *cam;
	Song *song;
	SongSelection *sel;
	ColorScheme &local_theme;
};

}

#endif /* SRC_VIEW_PAINTER_GRIDPAINTER_H_ */
