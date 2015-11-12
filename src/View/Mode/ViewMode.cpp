/*
 * ViewMode.cpp
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#include "ViewMode.h"
#include "../AudioView.h"

ViewMode::ViewMode(AudioView *_view, ViewMode *_parent)
{
	view = _view;
	parent = _parent;
	cam = &view->cam;
	selection = &view->selection;
	hover = &view->hover;
	win = view->win;
	song = view->song;
}

ViewMode::~ViewMode()
{
}

void ViewMode::onLeftButtonDown()
{
	if (parent)
		parent->onLeftButtonDown();
}

void ViewMode::onLeftButtonUp()
{
	if (parent)
		parent->onLeftButtonUp();
}

void ViewMode::onLeftDoubleClick()
{
	if (parent)
		parent->onLeftDoubleClick();
}

void ViewMode::onRightButtonDown()
{
	if (parent)
		parent->onRightButtonDown();
}

void ViewMode::onRightButtonUp()
{
	if (parent)
		parent->onRightButtonUp();
}

void ViewMode::onMouseMove()
{
	if (parent)
		parent->onMouseMove();
}

void ViewMode::onMouseWheel()
{
	if (parent)
		parent->onMouseWheel();
}

void ViewMode::onKeyDown(int k)
{
	if (parent)
		parent->onKeyDown(k);
}

void ViewMode::onKeyUp(int k)
{
	if (parent)
		parent->onKeyUp(k);
}

int ViewMode::getTrackHeightMin(Track* t)
{
	if (parent)
		return parent->getTrackHeightMin(t);
	return 5;
}

int ViewMode::getTrackHeightMax(Track* t)
{
	if (parent)
		return parent->getTrackHeightMax(t);
	return 5;
}

Selection ViewMode::getHover()
{
	Selection s;
	return s;
}
