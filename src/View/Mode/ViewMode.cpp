/*
 * ViewMode.cpp
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#include "ViewMode.h"
#include "../AudioView.h"

ViewMode::ViewMode(AudioView *_view)
{
	view = _view;
	session = view->session;
	cam = &view->cam;
	hover = &view->hover;
	win = view->win;
	song = view->song;
	side_bar_console = -1;
}

ViewMode::~ViewMode()
{
}

Selection ViewMode::get_hover()
{
	return Selection();
}

SongSelection ViewMode::get_selection(const Range &r)
{
	if (view->selection_mode == view->SelectionMode::TIME)
		return get_selection_for_range(r);
	if (view->selection_mode == view->SelectionMode::RECT)
		return get_selection_for_rect(r, hover->y0, hover->y1);
	if (view->selection_mode == view->SelectionMode::TRACK_RECT)
		return get_selection_for_track_rect(r, hover->y0, hover->y1);
	return SongSelection();
}

SongSelection ViewMode::get_selection_for_range(const Range &r)
{
	return SongSelection();
}

SongSelection ViewMode::get_selection_for_rect(const Range &r, int y0, int y1)
{
	return SongSelection();
}

SongSelection ViewMode::get_selection_for_track_rect(const Range &r, int y0, int y1)
{
	return SongSelection();
}
