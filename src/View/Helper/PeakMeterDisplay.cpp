/*
 * PeakMeterDisplay.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "PeakMeterDisplay.h"

#include "../../Audio/PeakMeter.h"
#include "../../Tsunami.h"
#include "../AudioView.h"
#include "../../lib/hui/hui.h"
#include <math.h>


PeakMeterDisplay::PeakMeterDisplay(hui::Panel *_panel, const string &_id, PeakMeter *_source, AudioView *_view)
{
	panel = _panel;
	id = _id;
	source = NULL;
	view = _view;
	enabled = false;

	panel->eventXP(id, "hui:draw", std::bind(&PeakMeterDisplay::onDraw, this, std::placeholders::_1));
	panel->eventX(id, "hui:left-button-down", std::bind(&PeakMeterDisplay::onLeftButtonDown, this));

	setSource(_source);
	enable(true);
}

PeakMeterDisplay::~PeakMeterDisplay()
{
	setSource(NULL);
}

void PeakMeterDisplay::setSource(PeakMeter *_source)
{
	if (source and enabled)
		source->unsubscribe(this);

	source = _source;

	if (source and enabled)
		source->subscribe(this, std::bind(&PeakMeterDisplay::onUpdate, this));
}

void PeakMeterDisplay::enable(bool _enabled)
{
	if (source){
		if (!enabled and _enabled)
			source->subscribe(this, std::bind(&PeakMeterDisplay::onUpdate, this));
		if (enabled and !_enabled)
			source->unsubscribe(this);
	}

	enabled = _enabled;
}

color peak_color(float peak, float a = 1)
{
	if (peak <= 1.001f)
		return SetColorHSB(a, (1 - pow(peak, 3) * 0.7f) * 0.33f, 0.8f, 0.8f);
	/*if (peak < 0.5f)
		return color(1, 0, 0.8f, 0);
	if (peak < 0.9f)
		return color(1, 0.2f, 0.7f, 0);
	if (peak < 1)
		return color(1, 0.5f, 0.5f, 0);*/
	return Red;
}

inline float nice_peak(float p)
{
	return min((float)pow(p, 0.8f), 1.0f);
}

void drawPeak(Painter *c, const rect &r, PeakMeter::Data &d, AudioView *view)
{
	int w = r.width();
	int h = r.height();
	float sp = d.get_sp();

	c->setColor(view->colors.background);
	if (sp > 1)
		c->setColor(Red);
	c->drawRect(r);

	c->setColor(peak_color(sp, 0.4f));
	c->drawRect(r.x1, r.y1,       (float)w * nice_peak(sp), h);

	c->setColor(peak_color(d.peak));
	c->drawRect(r.x1, r.y1,       (float)w * nice_peak(d.peak), h);

	c->setColor(view->colors.text);
	if (sp > 0)
		c->drawRect(w * nice_peak(sp), r.y1, 2, h);
}

void PeakMeterDisplay::onDraw(Painter *c)
{
	if (!source)
		return;
	int w = c->width;
	int h = c->height;
	if (source->mode == PeakMeter::MODE_PEAKS){

		drawPeak(c, rect(2, w-2, 2, h/2-1), source->r, view);
		drawPeak(c, rect(2, w-2, h/2 + 1, h-2), source->l, view);
	}else{
		c->setColor(view->colors.background);
		c->drawRect(2, 2, w - 4, h - 4);
		c->setColor(view->colors.text);
		float dx = 1.0f / (float)PeakMeter::SPECTRUM_SIZE * (w - 2);
		for (int i=0;i<100;i++){
			float x0 = 2 + (float)i / (float)PeakMeter::SPECTRUM_SIZE * (w - 2);
			float hh = (h - 4) * source->r.spec[i];
			c->drawRect(x0, h - 2 - hh, dx, hh);
		}
	}
}

void PeakMeterDisplay::onLeftButtonDown()
{
	if (source)
		source->set_mode((source->mode == PeakMeter::MODE_PEAKS) ? PeakMeter::MODE_SPECTRUM : PeakMeter::MODE_PEAKS);
}

void PeakMeterDisplay::onRightButtonDown()
{
}

void PeakMeterDisplay::onUpdate()
{
	panel->redraw(id);
}
