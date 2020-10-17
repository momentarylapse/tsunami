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


PeakMeterDisplay::PeakMeterDisplay(PeakMeter *_source, Mode constraint) {
	align.w = 120;
	align.h = 25;
	align.dx = 90;
	align.dy = -20;
	align.horizontal = align.Mode::LEFT;
	align.vertical = align.Mode::BOTTOM;
	align.dz = 100;
	panel = nullptr;
	source = nullptr;
	enabled = false;
	handler_id_draw = -1;
	handler_id_lbut = -1;
	r = new PeakMeterData;
	l = new PeakMeterData;
	r->reset();
	l->reset();

	mode_constraint = constraint;
	mode = Mode::PEAKS;
	if (mode_constraint == Mode::SPECTRUM)
		mode = Mode::SPECTRUM;

	set_source(_source);
	enable(true);
}

PeakMeterDisplay::PeakMeterDisplay(hui::Panel *_panel, const string &_id, PeakMeter *_source, Mode constraint) : PeakMeterDisplay(_source, constraint) {
	panel = _panel;
	id = _id;

	handler_id_draw = panel->event_xp(id, "hui:draw", [=](Painter *p){ area = p->area(); on_draw(p); });
	handler_id_lbut = panel->event_x(id, "hui:left-button-down", [=]{ on_left_button_down(); });
}

PeakMeterDisplay::~PeakMeterDisplay() {
	if (handler_id_draw >= 0)
		panel->remove_event_handler(handler_id_draw);
	if (handler_id_lbut >= 0)
		panel->remove_event_handler(handler_id_lbut);
	set_source(nullptr);
	delete r;
	delete l;
}

void PeakMeterDisplay::set_source(PeakMeter *_source) {
	if (source and enabled)
		unconnect();

	source = _source;

	if (source and enabled)
		connect();
}

void PeakMeterDisplay::enable(bool _enabled) {
	if (source) {
		if (!enabled and _enabled)
			connect();
		if (enabled and !_enabled)
			unconnect();
	}

	enabled = _enabled;
}

void PeakMeterDisplay::connect() {
	source->subscribe(this, [=]{ on_update(); });
	if (mode == Mode::SPECTRUM)
		source->request_spectrum();
}

void PeakMeterDisplay::unconnect() {
	source->unsubscribe(this);
	if (mode == Mode::SPECTRUM)
		source->unrequest_spectrum();
}

color peak_color(float peak, float a = 1) {
	if (peak <= 1.001f)
		return color::hsb((1 - pow(peak, 3) * 0.7f) * 0.33f, 0.8f, 0.8f, a);
	/*if (peak < 0.5f)
		return color(1, 0, 0.8f, 0);
	if (peak < 0.9f)
		return color(1, 0.2f, 0.7f, 0);
	if (peak < 1)
		return color(1, 0.5f, 0.5f, 0);*/
	return Red;
}

inline float nice_peak(float p) {
	return min((float)pow(p, 0.8f), 1.0f);
}

void draw_peak(Painter *c, const rect &r, PeakMeterData &d, const color &bg) {
	float w = r.width();
	float h = r.height();
	float sp = d.get_sp();

	c->set_color(bg);
	if (sp > 1)
		c->set_color(Red);
	c->draw_rect(r);

	c->set_color(peak_color(sp, 0.4f));
	if (w > h)
		c->draw_rect(rect(r.x1, r.x1 + w * nice_peak(sp), r.y1, r.y2));
	else
		c->draw_rect(rect(r.x1, r.x2, r.y2 - h  * nice_peak(sp), r.y2));

	c->set_color(peak_color(d.peak));
	if (w > h)
		c->draw_rect(rect(r.x1, r.x1 + w * nice_peak(d.peak), r.y1, r.y2));
	else
		c->draw_rect(rect(r.x1, r.x2, r.y2 - h  * nice_peak(d.peak), r.y2));

	c->set_color(AudioView::colors.text);
	if (sp > 0) {
		if (w > h)
			c->draw_rect(r.x1 + w * nice_peak(sp), r.y1, 2, h);
		else
			c->draw_rect(r.x1, r.y2 - h * nice_peak(sp), w, 2);
	}
}

void PeakMeterDisplay::on_draw(Painter *c) {
	if (!source)
		return;
	float w = area.width();
	float h = area.height();
	float boundary = 0;

	color bg = (parent ? AudioView::colors.background_overlay : AudioView::colors.background);

	if (mode == Mode::PEAKS) {

		float bb = 1;
		if (w > h) {
			draw_peak(c, rect(area.x1, area.x2, area.y1, area.my() - bb), *l, bg);
			draw_peak(c, rect(area.x1, area.x2, area.my() + bb, area.y2), *r, bg);
		} else if (h > w) {
			draw_peak(c, rect(area.x1, area.mx() - bb, area.y1, area.y2), *l, bg);
			draw_peak(c, rect(area.mx() + bb, area.x2, area.y1, area.y2), *r, bg);
		}
	} else {
		c->set_color(bg);
		c->draw_rect(area);
		c->set_color(AudioView::colors.text);
		float dx = 1.0f / (float)PeakMeter::SPECTRUM_SIZE * w;
		int n = min(100, l->spec.num);
		for (int i=0;i<n;i++) {
			float x0 = area.x1 + (float)i / (float)PeakMeter::SPECTRUM_SIZE * w;
			float hh = h * l->spec[i];
			c->draw_rect(rect(x0, x0 + dx, area.y2 - hh, area.y2));
		}
	}
}

bool PeakMeterDisplay::on_left_button_down() {
	if (mode_constraint != Mode::BOTH)
		return true;
	if (mode == Mode::SPECTRUM) {
		if (source)
			source->unrequest_spectrum();
		mode = Mode::PEAKS;
	} else {
		mode = Mode::SPECTRUM;
		if (source)
			source->request_spectrum();
	}
	return true;
}

bool PeakMeterDisplay::on_right_button_down() {
	return true;
}

// in PeakMeter/SignalChain's thread
void PeakMeterDisplay::on_update() {
	if (source) {
		*l = source->l;
		*r = source->r;
	}
	//hui::RunLater(0, std::bind(&hui::Panel::redraw, panel, id));
	if (panel)
		panel->redraw(id);
}
