/*
 * PeakMeter.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "PeakMeter.h"
#include "../../Plugins/FastFourierTransform.h"

const int NUM_SAMPLES = 2048;
const int SPECTRUM_SIZE = 30;
const float FREQ_MIN = 40.0f;
const float FREQ_MAX = 4000.0f;

void PeakMeter::Data::reset()
{
	peak = 0;
	super_peak = super_peak_t = 0;
	spec.clear();
	spec.resize(SPECTRUM_SIZE);
}

float PeakMeter::Data::get_sp()
{
	return super_peak * (1 - pow(super_peak_t, 3)*0.2f);
}

void PeakMeter::Data::update(Array<float> &buf, float sample_rate)
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
		super_peak_t += (float)buf.num / sample_rate;
	}
}

PeakMeter::PeakMeter(HuiPanel *_panel, const string &_id, PeakMeterSource *_source)
{
	panel = _panel;
	id = _id;
	source = _source;
	mode = ModePeaks;
	sample_rate = 44100;
	enabled = false;
	r.reset();
	l.reset();

	panel->EventMX(id, "hui:draw", this, &PeakMeter::OnDraw);
	panel->EventMX(id, "hui:left-button-down", this, &PeakMeter::OnLeftButtonDown);

	Enable(true);
}

PeakMeter::~PeakMeter()
{
	if (enabled)
		Unsubscribe(source);
}

void PeakMeter::Enable(bool _enabled)
{
	if ((!enabled) && (_enabled))
		Subscribe(source);
	if ((enabled) && (!_enabled))
		Unsubscribe(source);

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

void PeakMeter::DrawPeak(HuiPainter *c, const rect &r, Data &d)
{
	msg_db_f("PeakMeter.DrawPeak", 1);
	int w = r.width();
	int h = r.height();
	float sp = d.get_sp();

	c->setColor(White);
	if ((sp > 1) || (sp > 1))
		c->setColor(Red);
	c->drawRect(r);

	c->setColor(peak_color(sp, 0.4f));
	c->drawRect(r.x1, r.y1,       (float)w * nice_peak(sp), h);

	c->setColor(peak_color(d.peak));
	c->drawRect(r.x1, r.y1,       (float)w * nice_peak(d.peak), h);

	c->setColor(Black);
	if (sp > 0)
		c->drawRect(w * nice_peak(sp), r.y1, 2, h);
}

void PeakMeter::OnDraw()
{
	msg_db_f("PeakMeter.OnDraw", 1);
	HuiPainter *c = panel->BeginDraw(id);
	int w = c->width;
	int h = c->height;
	if (mode == ModePeaks){

		DrawPeak(c, rect(2, w-2, 2, h/2-1), r);
		DrawPeak(c, rect(2, w-2, h/2 + 1, h-2), l);
	}else{
		c->setColor(White);
		c->drawRect(2, 2, w - 4, h - 4);
		c->setColor(Black);
		float dx = 1.0f / (float)SPECTRUM_SIZE * (w - 2);
		for (int i=0;i<100;i++){
			float x0 = 2 + (float)i / (float)SPECTRUM_SIZE * (w - 2);
			float hh = (h - 4) * r.spec[i];
			c->drawRect(x0, h - 2 - hh, dx, hh);
		}
	}

	c->end();
}

void PeakMeter::FindPeaks()
{
	r.update(buf.r, sample_rate);
	l.update(buf.l, sample_rate);
}

void PeakMeter::ClearData()
{
	r.reset();
	l.reset();
}

inline float i_to_freq(int i)
{	return FREQ_MIN * exp( (float)i / (float)SPECTRUM_SIZE * log(FREQ_MAX / FREQ_MIN));	}

void PeakMeter::OnLeftButtonDown()
{
	SetMode((mode == ModePeaks) ? ModeSpectrum : ModePeaks);
}

void PeakMeter::OnRightButtonDown()
{
}

void PeakMeter::SetMode(int _mode)
{
	mode = _mode;
	panel->Redraw(id);
}

void PeakMeter::FindSpectrum()
{
	msg_db_f("PeakMeter.FindSp", 1);
	Array<complex> cr, cl;
	cr.resize(buf.num / 2 + 1);
	cl.resize(buf.num / 2 + 1);
	FastFourierTransform::fft_r2c(buf.r, cr);
	FastFourierTransform::fft_r2c(buf.l, cl);
	r.spec.resize(SPECTRUM_SIZE);
	l.spec.resize(SPECTRUM_SIZE);
	for (int i=0;i<SPECTRUM_SIZE;i++){
		float f0 = i_to_freq(i);
		float f1 = i_to_freq(i + 1);
		int n0 = f0 * buf.num / sample_rate;
		int n1 = max(f1 * buf.num / sample_rate, n0 + 1);
		float s = 0;
		for (int n=n0;n<n1;n++)
			if (n < cr.num){
				s = max(s, cr[n].abs_sqr());
				s = max(s, cl[n].abs_sqr());
			}
		r.spec[i] = l.spec[i] = sqrt(sqrt(s) / (float)SPECTRUM_SIZE / pi / 2);
	}
}

void PeakMeter::OnUpdate(Observable *o, const string &message)
{
	int state = source->GetState();

	if (state == source->STATE_PLAYING){
		sample_rate = source->GetSampleRate();
		source->GetSomeSamples(buf, NUM_SAMPLES);
		if (mode == ModePeaks)
			FindPeaks();
		else if (mode == ModeSpectrum)
			FindSpectrum();
	}else if (state == source->STATE_STOPPED){
		ClearData();
	}
	panel->Redraw(id);
}
