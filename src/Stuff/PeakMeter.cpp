/*
 * PeakMeter.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "PeakMeter.h"
#include "../Plugins/FastFourierTransform.h"

const int SPECTRUM_SIZE = 30;
const float FREQ_MIN = 40.0f;
const float FREQ_MAX = 4000.0f;

PeakMeter::PeakMeter(CHuiWindow *_win, const string &_id, PeakMeterSource *_source)
{
	win = _win;
	id = _id;
	source = _source;
	mode = ModePeaks;
	peak_r = 0;
	peak_l = 0;
	spec_r.resize(SPECTRUM_SIZE);
	spec_l.resize(SPECTRUM_SIZE);

	win->EventMX(id, "hui:redraw", this, (void(HuiEventHandler::*)())&PeakMeter::OnDraw);
	win->EventMX(id, "hui:left-button-down", this, (void(HuiEventHandler::*)())&PeakMeter::OnLeftButtonDown);
	Subscribe(source);
}

PeakMeter::~PeakMeter()
{
	Unsubscribe(source);
}

void PeakMeter::OnDraw()
{
	msg_db_r("PeakMeter.OnDraw", 1);
	HuiDrawingContext *c = win->BeginDraw(id);
	int w = c->width;
	int h = c->height;
	if (mode == ModePeaks){
		c->SetColor(White);
		//c->DrawRect(0, 0, w, h);
		c->DrawRect(2, 2, w-4, h/2 - 2);
		c->DrawRect(2, h/2 + 2, w-4, h - 2);
		c->SetColor(SetColorHSB(1, (1 - peak_r) * 0.33f, 1, 1));
		c->DrawRect(2, 2,       (float)(w - 4) * pow(peak_r, 0.5), h/2 - 2);
		c->DrawRect(2, h/2 + 2, (float)(w - 4) * pow(peak_l, 0.5), h - 2);
	}else{
		c->SetColor(White);
		c->DrawRect(2, 2, w - 4, h - 4);
		c->SetColor(Black);
		float dx = 1.0f / (float)SPECTRUM_SIZE * (w - 2);
		for (int i=0;i<100;i++){
			float x0 = 2 + (float)i / (float)SPECTRUM_SIZE * (w - 2);
			float hh = (h - 4) * spec_r[i];
			c->DrawRect(x0, h - 2 - hh, dx, hh);
		}
	}

	c->End();
	msg_db_l(1);
}

void PeakMeter::FindPeaks()
{
	peak_r = peak_l = 0;
	for (int i=0;i<buf.num;i++){
		if (fabs(buf.r[i]) > peak_r)
			peak_r = fabs(buf.r[i]);
		if (fabs(buf.l[i]) > peak_l)
			peak_l = fabs(buf.l[i]);
	}
}

inline float cabs(complex &c)
{	return sqrt(c.x * c.x + c.y * c.y);	}

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
	win->Redraw(id);
}

void PeakMeter::FindSpectrum()
{
	msg_db_r("PeakMeter.FindSp", 1);
	Array<complex> cr, cl;
	cr.resize(buf.num / 2 + 1);
	cl.resize(buf.num / 2 + 1);
	FastFourierTransform::fft_r2c(buf.r, cr);
	FastFourierTransform::fft_r2c(buf.l, cl);
	spec_r.resize(SPECTRUM_SIZE);
	spec_l.resize(SPECTRUM_SIZE);
	for (int i=0;i<SPECTRUM_SIZE;i++){
		float f0 = i_to_freq(i);
		float f1 = i_to_freq(i + 1);
		int n0 = f0 * buf.num / sample_rate;
		int n1 = f1 * buf.num / sample_rate;
		float s = 0;
		for (int n=n0;n<n1;n++)
			if (n < cr.num){
				s = max(s, cabs(cr[n]));
				s = max(s, cabs(cl[n]));
			}
		spec_r[i] = spec_l[i] = sqrt(s / (float)SPECTRUM_SIZE / pi / 2);
	}
	msg_db_l(1);
}

void PeakMeter::OnUpdate(Observable *o)
{
	sample_rate = source->GetSampleRate();
	buf = source->GetSomeSamples();

	if (mode == ModePeaks)
		FindPeaks();
	else if (mode == ModeSpectrum)
		FindSpectrum();
	win->Redraw(id);
}
