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
#include "../../Audio/DeviceManager.h"

MiniBar::MiniBar(BottomBar *_bottom_bar, AudioStream *_stream, DeviceManager *_output) :
	Observer("MiniBar")
{
	stream = _stream;
	output = _output;
	bottom_bar = _bottom_bar;
	setBorderWidth(0);
	addGrid("!expandx", 0, 0, 1, 2, "grid0");
	setTarget("grid0", 0);
	addSeparator("!horizontal", 0, 0, 0, 0, "");
	setBorderWidth(10);
	addGrid("!expandx", 0, 1, 5, 1, "grid");
	setTarget("grid", 0);
	addButton("!flat", 0, 0, 0, 0, "show_bottom_bar");
	setImage("show_bottom_bar", "hui:up");
	addDrawingArea("!width=100,noexpandx,noexpandy", 1, 0, 0, 0, "peaks");
	addSlider("!width=100,noexpandx,noexpandy", 2, 0, 0, 0, "volume");

	setTooltip("show_bottom_bar", _("Leiste aufklappen"));
	setTooltip("volume", _("Ausgabelautst&arke"));
	setTooltip("peaks", _("Ausgabepegel"));

	peak_meter = new PeakMeter(this, "peaks", stream);
	setFloat("volume", output->getOutputVolume());

	event("show_bottom_bar", this, &MiniBar::onShowBottomBar);
	event("volume", this, &MiniBar::onVolume);

	subscribe(bottom_bar);
	subscribe(output);
}

MiniBar::~MiniBar()
{
	unsubscribe(output);
	unsubscribe(bottom_bar);
	delete(peak_meter);
}

void MiniBar::onShowBottomBar()
{
	bottom_bar->_show();
	hide();
}

void MiniBar::onVolume()
{
	output->setOutputVolume(getFloat(""));
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
	if (o == bottom_bar){
		if (bottom_bar->visible)
			hide();
		else
			show();
	}else if (o == output){
		setFloat("volume", output->getOutputVolume());
	}
}

