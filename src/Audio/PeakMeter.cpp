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
#include "../Session.h"


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

PeakMeter::Output::Output(PeakMeter *p)
{
	peak_meter = p;
}

PeakMeter::PeakMeter(Session *s) :
	Module(s, Type::PEAK_METER)
{
	source = NULL;
	mode = MODE_PEAKS;
	out = new Output(this);
	port_out.add(PortDescription(SignalType::AUDIO, out, "out"));
	port_in.add(PortDescription(SignalType::AUDIO, NULL, "in"));
	r.reset();
	l.reset();
	ring_buffer = new RingBuffer(1<<18);
}

PeakMeter::~PeakMeter()
{
	delete ring_buffer;
}

inline float nice_peak(float p)
{
	return min((float)pow(p, 0.8f), 1.0f);
}

void PeakMeter::find_peaks(AudioBuffer &buf)
{
	float dt = (float)buf.length / (float)session->sample_rate();
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
	float sample_rate = (float)session->sample_rate();
	for (int i=0;i<SPECTRUM_SIZE;i++){
		float f0 = i_to_freq(i);
		float f1 = i_to_freq(i + 1);
		int n0 = f0 * buf.length / sample_rate;
		int n1 = max((int)(f1 * buf.length / sample_rate), n0 + 1);
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
	if (mode == MODE_PEAKS)
		find_peaks(buf);
	else if (mode == MODE_SPECTRUM)
		find_spectrum(buf);
	notify();
	clear_data();
}


void PeakMeter::set_source(AudioPort* s)
{
	source = s;
}

int PeakMeter::Output::read(AudioBuffer& buf)
{
	if (!peak_meter->source)
		return 0;
	int r = peak_meter->source->read(buf);
	if (r <= 0)
		return r;

	AudioBuffer b;
	b.set_as_ref(buf, 0, r);
	peak_meter->ring_buffer->write(b);

	if (peak_meter->ring_buffer->available() > NUM_SAMPLES){
		AudioBuffer b2;
		b2.resize(peak_meter->ring_buffer->available());
		peak_meter->ring_buffer->read(b2);
		peak_meter->update(b2);
	}
	return r;
}

int PeakMeter::Output::get_pos(int delta)
{
	if (!peak_meter->source)
		return 0;
	return peak_meter->source->get_pos(delta);
}

void PeakMeter::Output::reset()
{
	if (peak_meter->source)
		peak_meter->source->reset();
	peak_meter->clear_data();
	peak_meter->notify();
}
