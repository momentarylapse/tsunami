/*
 * PeakMeter.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "PeakMeter.h"
#include "../../Plugins/FastFourierTransform.h"
#include "../../Tsunami.h"
#include "../AudioView.h"

const int PeakMeter::NUM_SAMPLES = 1024;
const int PeakMeter::SPECTRUM_SIZE = 30;
const float PeakMeter::FREQ_MIN = 40.0f;
const float PeakMeter::FREQ_MAX = 4000.0f;
const float PeakMeter::UPDATE_DT = 0.05f;

void PeakMeter::Data::reset()
{
	peak = 0;
	super_peak = super_peak_t = 0;
	spec.clear();
	spec.resize(SPECTRUM_SIZE);
}

float PeakMeter::Data::get_sp()
{
	return max(super_peak * (1 - pow(super_peak_t, 3)*0.2f), 0.0001f);
}

void PeakMeter::Data::update(Array<float> &buf, float dt)
{
	peak = 0;
	for (int i=0; i<buf.num; i++){
		if (fabs(buf[i]) > peak)
			peak = fabs(buf[i]);
	}
	if (peak > get_sp()){
		super_peak = peak;
		super_peak_t = 0;
	}else{
		super_peak_t += dt;
	}
}

PeakMeter::PeakMeter(HuiPanel *_panel, const string &_id, PeakMeterSource *_source, AudioView *_view) :
	Observer("PeakMeter")
{
	panel = _panel;
	id = _id;
	source = NULL;
	view = _view;
	mode = ModePeaks;
	sample_rate = DEFAULT_SAMPLE_RATE;
	enabled = false;
	r.reset();
	l.reset();

	panel->eventX(id, "hui:draw", this, &PeakMeter::onDraw);
	panel->eventX(id, "hui:left-button-down", this, &PeakMeter::onLeftButtonDown);

	setSource(_source);
	enable(true);
}

PeakMeter::~PeakMeter()
{
	setSource(NULL);
}

void PeakMeter::setSource(PeakMeterSource *_source)
{
	if (source and enabled)
		unsubscribe(source);

	source = _source;

	if (source and enabled)
		subscribe(source);
}

void PeakMeter::enable(bool _enabled)
{
	if (source){
		if (!enabled and _enabled)
			subscribe(source);
		if (enabled and !_enabled)
			unsubscribe(source);
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
	return min(pow(p, 0.8f), 1);
}

void PeakMeter::drawPeak(Painter *c, const rect &r, Data &d)
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

void PeakMeter::onDraw(Painter *c)
{
	int w = c->width;
	int h = c->height;
	if (mode == ModePeaks){

		drawPeak(c, rect(2, w-2, 2, h/2-1), r);
		drawPeak(c, rect(2, w-2, h/2 + 1, h-2), l);
	}else{
		c->setColor(view->colors.background);
		c->drawRect(2, 2, w - 4, h - 4);
		c->setColor(view->colors.text);
		float dx = 1.0f / (float)SPECTRUM_SIZE * (w - 2);
		for (int i=0;i<100;i++){
			float x0 = 2 + (float)i / (float)SPECTRUM_SIZE * (w - 2);
			float hh = (h - 4) * r.spec[i];
			c->drawRect(x0, h - 2 - hh, dx, hh);
		}
	}
}

void PeakMeter::findPeaks()
{
	float dt = timer.peek();
	r.update(buf.c[0], dt);
	l.update(buf.c[1], dt);
}

void PeakMeter::clearData()
{
	r.reset();
	l.reset();
}

inline float PeakMeter::i_to_freq(int i)
{	return FREQ_MIN * exp( (float)i / (float)SPECTRUM_SIZE * log(FREQ_MAX / FREQ_MIN));	}

void PeakMeter::onLeftButtonDown()
{
	setMode((mode == ModePeaks) ? ModeSpectrum : ModePeaks);
}

void PeakMeter::onRightButtonDown()
{
}

void PeakMeter::setMode(int _mode)
{
	mode = _mode;
	panel->redraw(id);
}

void PeakMeter::findSpectrum()
{
	Array<complex> cr, cl;
	cr.resize(buf.length / 2 + 1);
	cl.resize(buf.length / 2 + 1);
	FastFourierTransform::fft_r2c(buf.c[0], cr);
	FastFourierTransform::fft_r2c(buf.c[1], cl);
	r.spec.resize(SPECTRUM_SIZE);
	l.spec.resize(SPECTRUM_SIZE);
	for (int i=0;i<SPECTRUM_SIZE;i++){
		float f0 = i_to_freq(i);
		float f1 = i_to_freq(i + 1);
		int n0 = f0 * buf.length / sample_rate;
		int n1 = max(f1 * buf.length / sample_rate, n0 + 1);
		float s = 0;
		for (int n=n0;n<n1;n++)
			if (n < cr.num){
				s = max(s, cr[n].abs_sqr());
				s = max(s, cl[n].abs_sqr());
			}
		r.spec[i] = l.spec[i] = sqrt(sqrt(s) / (float)SPECTRUM_SIZE / pi / 2);
	}
}

void PeakMeter::onUpdate(Observable *o, const string &message)
{
	int state = source->getState();

	if (state == source->STATE_PLAYING){
		if (timer.peek() < UPDATE_DT)
			return;
		sample_rate = source->getSampleRate();
		source->getSomeSamples(buf, NUM_SAMPLES);
		if (mode == ModePeaks)
			findPeaks();
		else if (mode == ModeSpectrum)
			findSpectrum();
		timer.get();
	}else if (state == source->STATE_STOPPED){
		clearData();
	}
	panel->redraw(id);
}
