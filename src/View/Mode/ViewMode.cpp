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
	Selection s;
	return s;
}
