/*
 * BufferSpectrogram.cpp
 *
 *  Created on: 13 Aug 2023
 *      Author: michi
 */

#include "BufferSpectrogram.h"
#include "../../data/audio/AudioBuffer.h"
#include "../../lib/math/complex.h"
#include "../../lib/math/math.h"
#include "../../lib/fft/fft.h"

#include "../../lib/os/msg.h"

namespace BufferSpectrogram {


static float sum_abs(const Array<complex> &z) {
	float r = 0;
	for (auto &x: z)
		r += x.abs();
	return r;
}

/*static float max_abs(const Array<complex> &z) {
	float r = 0;
	for (auto &x: z)
		r = max(r, x.abs());
	return r;
}*/

Array<complex> spectrogram(AudioBuffer &b, int step_size, int window_size, WindowFunction wf) {
	Array<complex> r;

	Array<complex> z;
	for (int i=0; i<b.length/step_size; i++) {
		auto chunk = b.c[0].sub_ref(i * step_size, i * step_size + window_size);
		if (chunk.num < window_size)
			break;

		// TODO: window
		fft::r2c(chunk, z);

		r.append(z);
	}
	return r;
}

bytes quantized_spectrogram(AudioBuffer &b, float sample_rate, int step_size, float f_min, float f_max, int f_count, WindowFunction wf) {
	int window_size = step_size * 8;
	int fft_size = window_size / 2 + 1;

	float ww = (float)window_size / sample_rate;

	auto s = spectrogram(b, step_size, window_size, WindowFunction::RECTANGLE);

	bytes spectrum;
	for (int i=0; i<s.num/fft_size; i++) {
		auto z = s.sub_ref(i*fft_size, (i+1)*fft_size);
		for (int k=0; k<f_count; k++) {
			float bin_f_min = f_min * exp( log(f_max / f_min) / (f_count - 1) * k);
			float bin_f_max = f_min * exp( log(f_max / f_min) / (f_count - 1) * (k + 1));
			int j0 = bin_f_min * ww;
			int j1 = bin_f_max * ww + 1;
			j0 = clamp(j0, 0, z.num);
			j1 = clamp(j1, 0, z.num);
			float f = sum_abs(z.sub_ref(j0, j1)) / (window_size / 2) * pi * 3; // arbitrary... just "louder"
			//float f = max_abs(z.sub_ref(j0, j1)) / (SPECTRUM_FFT_INPUT / 2) * pi * 3; // arbitrary... just "louder"
			// / (SPECTRUM_FFT_INPUT / 2 / SPECTRUM_N);
			f = clamp(f, 0.0f, 1.0f);
			//f = 1-exp(-f);
			spectrum.add(254 * f);
		}
	}
	return spectrum;
}


}

