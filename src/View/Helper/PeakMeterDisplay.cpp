/*
 * PeakMeterDisplay.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "PeakMeterDisplay.h"

#include "../../Tsunami.h"
#include "../AudioView.h"
#include "../../lib/hui/hui.h"
#include <math.h>
#include "../../Module/Audio/PeakMeter.h"


PeakMeterDisplay::PeakMeterDisplay(hui::Panel *_panel, const string &_id, PeakMeter *_source)
{
	panel = _panel;
	id = _id;
	source = nullptr;
	enabled = false;
	r = new PeakMeterData;
	l = new PeakMeterData;
	r->reset();
	l->reset();

	handler_id_draw = panel->eventXP(id, "hui:draw", std::bind(&PeakMeterDisplay::on_draw, this, std::placeholders::_1));
	handler_id_lbut = panel->eventX(id, "hui:left-button-down", std::bind(&PeakMeterDisplay::on_left_button_down, this));

	set_source(_source);
	enable(true);
}

PeakMeterDisplay::~PeakMeterDisplay()
{
	panel->removeEventHandler(handler_id_draw);
	panel->removeEventHandler(handler_id_lbut);
	set_source(nullptr);
	delete r;
	delete l;
}

void PeakMeterDisplay::set_source(PeakMeter *_source)
{
	if (source and enabled)
		source->unsubscribe(this);

	source = _source;

	if (source and enabled)
		source->subscribe(this, std::bind(&PeakMeterDisplay::on_update, this));
}

void PeakMeterDisplay::enable(bool _enabled)
{
	if (source){
		if (!enabled and _enabled)
			source->subscribe(this, std::bind(&PeakMeterDisplay::on_update, this));
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

void drawPeak(Painter *c, const rect &r, PeakMeterData &d)
{
	int w = r.width();
	int h = r.height();
	float sp = d.get_sp();

	c->set_color(AudioView::colors.background);
	if (sp > 1)
		c->set_color(Red);
	c->draw_rect(r);

	c->set_color(peak_color(sp, 0.4f));
	c->draw_rect(r.x1, r.y1,       (float)w * nice_peak(sp), h);

	c->set_color(peak_color(d.peak));
	c->draw_rect(r.x1, r.y1,       (float)w * nice_peak(d.peak), h);

	c->set_color(AudioView::colors.text);
	if (sp > 0)
		c->draw_rect(w * nice_peak(sp), r.y1, 2, h);
}

void PeakMeterDisplay::on_draw(Painter *c)
{
	if (!source)
		return;
	int w = c->width;
	int h = c->height;
	if (source->mode == PeakMeter::Mode::PEAKS){

		drawPeak(c, rect(2, w-2, 2, h/2-1), *r);
		drawPeak(c, rect(2, w-2, h/2 + 1, h-2), *l);
	}else{
		c->set_color(AudioView::colors.background);
		c->draw_rect(2, 2, w - 4, h - 4);
		c->set_color(AudioView::colors.text);
		float dx = 1.0f / (float)PeakMeter::SPECTRUM_SIZE * (w - 2);
		for (int i=0;i<100;i++){
			float x0 = 2 + (float)i / (float)PeakMeter::SPECTRUM_SIZE * (w - 2);
			float hh = (h - 4) * r->spec[i];
			c->draw_rect(x0, h - 2 - hh, dx, hh);
		}
	}
}

void PeakMeterDisplay::on_left_button_down()
{
	if (source)
		source->set_mode((source->mode == PeakMeter::Mode::PEAKS) ? PeakMeter::Mode::SPECTRUM : PeakMeter::Mode::PEAKS);
}

void PeakMeterDisplay::on_right_button_down()
{
}

// in PeakMeter/SignalChain's thread
void PeakMeterDisplay::on_update()
{
	if (source){
		*r = source->r;
		*l = source->l;
	}
	//hui::RunLater(0, std::bind(&hui::Panel::redraw, panel, id));
	panel->redraw(id);
}
