/*
 * ExtendedBufferBox.cpp
 *
 *  Created on: 01.02.2013
 *      Author: michi
 */

#include "ExtendedBufferBox.h"
#include "FastFourierTransform.h"
#include "../lib/types/types.h"
#include <math.h>

ExtendedBufferBox::ExtendedBufferBox()
{
}

ExtendedBufferBox::~ExtendedBufferBox()
{
}

void ExtendedBufferBox::add_click(int pos, float volume, float freq, int sample_rate)
{
	int sm_d = 0.06f * sample_rate;
	float w_f = 1.0 / sample_rate * freq * 2.0 * 3.14159265358979f;

	for (int i=0; i<sm_d; i++){
		float fi = (float)i / (float)sm_d;
		float envelope = exp(-fi*3);//1 - fi;
		float tt = i * w_f;
		int j = i + pos;// - offset;
		if ((j >= 0) && (j < num)){
			float d = sin(tt) * volume * envelope;
			r[j] += d;
			l[j] += d;
		}
	}
}

void ExtendedBufferBox::add_tone(const Range &range, float volume, float freq, int sample_rate)
{
	float f_w = 1.0f / sample_rate * freq * 2.0f * pi;
	int i0 = max(range.offset, 0);
	int i1 = min(range.end(), num);
	for (int i=i0; i<i1; i++){
		float tt = (i - range.offset) * f_w;
		float d = sin(tt) * volume;
		if (i < range.offset + 1000)
			d *= (i - range.offset) * 0.001;
		if (i > range.end() - 1000)
			d *= (range.end() - i) * 0.001;
		r[i] += d;
		l[i] += d;
	}
}

void ExtendedBufferBox::get_spectrum(Array<complex> &spec_r, Array<complex> &spec_l, int samples)
{
	FastFourierTransform::fft_r2c(r, spec_r);
	FastFourierTransform::fft_r2c(l, spec_l);
}

