/*
 * PeakMeter.cpp
 *
 *  Created on: 01.04.2018
 *      Author: michi
 */

#include "PeakMeter.h"
#include "AudioBuffer.h"
#include "RingBuffer.h"
#include "../Plugins/FastFourierTransform.h"
#include "../lib/hui/hui.h"


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
	return max(super_peak * (1 - (float)pow(super_peak_t, 3)*0.2f), 0.0001f);
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

PeakMeter::PeakMeter(AudioPort *s)
{
	source = s;
	mode = MODE_PEAKS;
	_sample_rate = DEFAULT_SAMPLE_RATE;
	r.reset();
	l.reset();
}

PeakMeter::~PeakMeter()
{
}

inline float nice_peak(float p)
{
	return min((float)pow(p, 0.8f), 1.0f);
}

void PeakMeter::find_peaks(AudioBuffer &buf)
{
	float dt = (float)buf.length / (float)_sample_rate;
	r.update(buf.c[0], dt);
	l.update(buf.c[1], dt);
}

void PeakMeter::clear_data()
{
	r.reset();
	l.reset();
}

inline float PeakMeter::i_to_freq(int i)
{	return FREQ_MIN * exp( (float)i / (float)SPECTRUM_SIZE * log(FREQ_MAX / FREQ_MIN));	}

void PeakMeter::set_mode(int _mode)
{
	mode = _mode;
}

void PeakMeter::find_spectrum(AudioBuffer &buf)
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
		int n0 = f0 * buf.length / (float)_sample_rate;
		int n1 = max((int)(f1 * buf.length / (float)_sample_rate), n0 + 1);
		float s = 0;
		for (int n=n0;n<n1;n++)
			if (n < cr.num){
				s = max(s, cr[n].abs_sqr());
				s = max(s, cl[n].abs_sqr());
			}
		r.spec[i] = l.spec[i] = sqrt(sqrt(s) / (float)SPECTRUM_SIZE / pi / 2);
	}
}

void PeakMeter::update(AudioBuffer &buf)
{
	_sample_rate = source->sample_rate();
	//clearData();
	if (mode == MODE_PEAKS)
		find_peaks(buf);
	else if (mode == MODE_SPECTRUM)
		find_spectrum(buf);
	notify();
}


void PeakMeter::set_source(AudioPort* s)
{
	source = s;
}

int PeakMeter::read(AudioBuffer& buf)
{
	if (!source)
		return 0;
	int r = source->read(buf);
	update(buf);
	return r;
}

int PeakMeter::get_pos(int delta)
{
	if (!source)
		return 0;
	return source->get_pos(delta);
}

int PeakMeter::sample_rate()
{
	if (!source)
		return DEFAULT_SAMPLE_RATE;
	return source->sample_rate();
}

void PeakMeter::reset()
{
	if (source)
		source->reset();
	clear_data();
	notify();
}
