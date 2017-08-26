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
	cam = &view->cam;
	hover = &view->hover;
	win = view->win;
	song = view->song;
}

ViewMode::~ViewMode()
{
}

Selection ViewMode::getHover()
{
	return Selection();
}

SongSelection ViewMode::getSelection()
{
	if (view->selection_mode == view->SELECTION_MODE_TIME)
		return getSelectionForRange(hover->range);
	if (view->selection_mode == view->SELECTION_MODE_RECT)
		return getSelectionForRect(hover->range, hover->y0, hover->y1);
	if (view->selection_mode == view->SELECTION_MODE_TRACK_RECT)
		return getSelectionForTrackRect(hover->range, hover->y0, hover->y1);
	return SongSelection();
}

SongSelection ViewMode::getSelectionForRange(const Range &r)
{
	return SongSelection();
}

SongSelection ViewMode::getSelectionForRect(const Range &r, int y0, int y1)
{
	return SongSelection();
}

SongSelection ViewMode::getSelectionForTrackRect(const Range &r, int y0, int y1)
{
	return SongSelection();
}
