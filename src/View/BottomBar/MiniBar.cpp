/*
 * MiniBar.cpp
 *
 *  Created on: 23.03.2014
 *      Author: michi
 */

#include "BottomBar.h"
#include "../Helper/PeakMeter.h"
#include "../../Device/OutputStream.h"
#include "MiniBar.h"
#include "../../Device/DeviceManager.h"

MiniBar::MiniBar(BottomBar *_bottom_bar, OutputStream *_stream, DeviceManager *_output, AudioView *view)
{
	stream = _stream;
	output = _output;
	bottom_bar = _bottom_bar;

	fromResource("mini_bar");

	peak_meter = new PeakMeter(this, "peaks", stream, view);
	setFloat("volume", output->getOutputVolume());

	event("show_bottom_bar", std::bind(&MiniBar::onShowBottomBar, this));
	event("volume", std::bind(&MiniBar::onVolume, this));

	bottom_bar->subscribe(this, std::bind(&MiniBar::onBottomBarUpdate, this));
	output->subscribe(this, std::bind(&MiniBar::onVolumeChange, this));
}

MiniBar::~MiniBar()
{
	output->unsubscribe(this);
	bottom_bar->unsubscribe(this);
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

void MiniBar::onBottomBarUpdate()
{
	if (bottom_bar->visible)
		hide();
	else
		show();
}

void MiniBar::onVolumeChange()
{
	setFloat("volume", output->getOutputVolume());
}

