/*
 * PeakMeter.cpp
 *
 *  Created on: 01.04.2018
 *      Author: michi
 */

#include "PeakMeter.h"

#include "../../Data/Audio/AudioBuffer.h"
#include "../../Data/Audio/RingBuffer.h"
#include "../../Plugins/FastFourierTransform.h"
#include "../../Session.h"
#include "../../lib/hui/hui.h"


const int PeakMeter::SPECTRUM_SIZE = 30;
const float PeakMeter::FREQ_MIN = 40.0f;
const float PeakMeter::FREQ_MAX = 4000.0f;

PeakMeterData::PeakMeterData() {
	reset();
}

void PeakMeterData::reset() {
	peak = 0;
	super_peak = super_peak_t = 0;
	spec.clear();
	spec.resize(PeakMeter::SPECTRUM_SIZE);
}

float PeakMeterData::get_sp() {
	return max(super_peak * (1 - (float)pow(super_peak_t, 3)*0.2f), 0.0001f);
}

void PeakMeterData::update(Array<float> &buf, float dt) {
	peak = 0;
	for (int i=0; i<buf.num; i++) {
		if (fabs(buf[i]) > peak)
			peak = fabs(buf[i]);
	}
	if (peak > get_sp()) {
		super_peak = peak;
		super_peak_t = 0;
	} else {
		super_peak_t += dt;
	}
}

PeakMeter::PeakMeter() {
	module_class = "PeakMeter";
	spectrum_requests = 0;
	_set_channels(2);
}

PeakMeter::~PeakMeter() {
}

void PeakMeter::_set_channels(int num_channels) {
	channels.resize(num_channels);
}

inline float nice_peak(float p) {
	return min((float)pow(p, 0.8f), 1.0f);
}

void PeakMeter::find_peaks(AudioBuffer &buf) {
	float dt = (float)buf.length / (float)session->sample_rate();
	for (int i=0; i<buf.channels; i++)
		channels[i].update(buf.c[i], dt);
}

void PeakMeter::clear_data() {
	for (auto &c: channels)
		c.reset();
}

inline float PeakMeter::i_to_freq(int i) {
	return FREQ_MIN * exp( (float)i / (float)SPECTRUM_SIZE * log(FREQ_MAX / FREQ_MIN));
}

void PeakMeter::request_spectrum() {
	spectrum_requests ++;
}

void PeakMeter::unrequest_spectrum() {
	spectrum_requests --;
}

void PeakMeter::find_spectrum(AudioBuffer &buf) {
	for (int i=0; i<buf.channels; i++) {
		auto &c = channels[i];

		Array<complex> zz;
		FastFourierTransform::fft_r2c(buf.c[i], zz);
		c.spec.resize(SPECTRUM_SIZE);
		float sample_rate = (float)session->sample_rate();
		for (int i=0;i<SPECTRUM_SIZE;i++) {
			float f0 = i_to_freq(i);
			float f1 = i_to_freq(i + 1);
			int n0 = f0 * buf.length / sample_rate;
			int n1 = max((int)(f1 * buf.length / sample_rate), n0 + 1);
			float s = 0;
			for (int n=n0; n<n1; n++)
				if (n < zz.num)
					s = max(s, zz[n].abs_sqr());
			c.spec[i] = sqrt(sqrt(s) / (float)SPECTRUM_SIZE / pi / 2);
		}
	}
}

void PeakMeter::process(AudioBuffer& buf) {
	clear_data();
	_set_channels(buf.channels);

	find_peaks(buf);
	if (spectrum_requests > 0)
		find_spectrum(buf);
	notify();
}

void PeakMeter::reset_state() {
	clear_data();
	notify();
}
