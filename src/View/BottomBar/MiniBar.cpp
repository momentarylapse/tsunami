/*
 * MiniBar.cpp
 *
 *  Created on: 23.03.2014
 *      Author: michi
 */

#include "BottomBar.h"
#include "../Helper/PeakMeter.h"
#include "../../Audio/AudioStream.h"
#include "MiniBar.h"

MiniBar::MiniBar(BottomBar *_bottom_bar, AudioStream *_stream) :
	Observer("MiniBar")
{
	stream = _stream;
	bottom_bar = _bottom_bar;
	addControlTable("!noexpandx", 0, 0, 5, 1, "grid");
	setTarget("grid", 0);
	addButton("", 0, 0, 0, 0, "show_bottom_bar");
	setImage("show_bottom_bar", "hui:up");
	addDrawingArea("!width=100,noexpandx,noexpandy", 1, 0, 0, 0, "peaks");

	peak_meter = new PeakMeter(this, "peaks", stream);

	event("show_bottom_bar", this, &MiniBar::onShowBottomBar);

	subscribe(bottom_bar);
}

MiniBar::~MiniBar()
{
	unsubscribe(bottom_bar);
	delete(peak_meter);
}

void MiniBar::onShowBottomBar()
{
	bottom_bar->show();
	hide();
}

void MiniBar::onShow()
{
	peak_meter->enable(true);
}

void MiniBar::onHide()
{
	peak_meter->enable(false);
}

void MiniBar::onUpdate(Observable *o, const string &message)
{
	if (!bottom_bar->visible)
		show();
}

