/*
 * Spectrogram.cpp
 *
 *  Created on: 13 Aug 2023
 *      Author: michi
 */

#include "Spectrogram.h"
#include "../../data/base.h"
#include "../../data/audio/AudioBuffer.h"
#include "../../lib/math/complex.h"
#include "../../lib/math/math.h"
#include "../../lib/fft/fft.h"

#include "../../lib/os/msg.h"

namespace Spectrogram {


int spectrum_size(int window_size) {
	return window_size / 2 + 1;
}

Array<float> power_spectrum(Array<float> &chunk) {
	Array<complex> z;
	fft::r2c(chunk, z);

	float scale = 1.0f / (z.num * z.num);

	Array<float> s;
	s.resize(z.num);
	for (int i=0; i<z.num; i++)
		s[i] = z[i].abs_sqr() * scale;
	return s;
}

Array<float> spectrogram(AudioBuffer &b, int step_size, int window_size, WindowFunction wf) {
	Array<float> all;

	Array<complex> z;
	Array<float> power;
	for (int i=0; i<b.length/step_size; i++) {
		auto chunk = b.c[0].sub_ref(i * step_size, i * step_size + window_size);
		if (chunk.num < window_size)
			break;

		chunk.make_own();
		apply_window_function(chunk, wf);

		all.append(power_spectrum(chunk));
	}
	return all;
}

inline float exp_interpolate(float x, float x_min, float x_max) {
	return x_min * exp( log(x_max / x_min) *x);
}

Array<float> log_spectrogram(AudioBuffer &b, float sample_rate, int step_size, float f_min, float f_max, int f_count, WindowFunction wf) {
	int window_size = step_size * 8;
	int fft_size = spectrum_size(window_size);

	float ww = (float)window_size / sample_rate;

	auto s = spectrogram(b, step_size, window_size, wf);

	Array<float> rr;
	for (int i=0; i<s.num/fft_size; i++) {
		auto z = s.sub_ref(i*fft_size, (i+1)*fft_size);
		for (int k=0; k<f_count; k++) {
			float bin_f_min = f_min * exp( log(f_max / f_min) / (f_count - 1) * k);
			float bin_f_max = f_min * exp( log(f_max / f_min) / (f_count - 1) * (k + 1));
			int j0 = clamp(int(bin_f_min * ww), 0, z.num);
			int j1 = clamp(int(bin_f_max * ww) + 1, 0, z.num);
			float f = xmax(z.sub_ref(j0, j1));
			rr.add(f);
		}
	}
	return rr;
}

Array<float> to_db(const Array<float> &data, float db_range, float db_boost) {
	const float scale = 1.0f / db_range;

	Array<float> r;
	r.resize(data.num);
	for (int i=0; i<data.num; i++) {
		float db = power2db(data[i]);
		r[i] = clamp((db + db_boost) * scale + 1.0f, 0.0f, 1.0f);
	}
	return r;
}

bytes quantize(const Array<float> &data) {
	bytes r;
	r.resize(data.num);
	for (int i=0; i<data.num; i++) {
		float f = clamp(data[i], 0.0f, 1.0f);
		r[i] = 254 * f;
	}
	return r;
}

float dequantize(unsigned char q) {
	return (float)q / 254.0f;
}


}

