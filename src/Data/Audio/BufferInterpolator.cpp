/*
 * BufferInterpolator.cpp
 *
 *  Created on: 21.04.2017
 *      Author: michi
 */

#include "BufferInterpolator.h"
#include "../../Plugins/FastFourierTransform.h"
#include "AudioBuffer.h"


void BufferInterpolator::interpolate_channel_linear(Array<float> &in, Array<float> &out) {
	out[0] = in[0];
	out.back() = in.back();

	for (int i=0; i<out.num-1; i++) {
		double x = (double)i * (double)(in.num - 1) / (double)(out.num - 1);
		int j = (int)x;
		double t = x - j;
		if (j < in.num - 1)
			out[i] = (1-t) * in[j] + t * in[j+1];
		else
			out[i] = in[j];
	}
}

double BufferInterpolator::cubic_inter(double A, double B, double C, double D, float t) {
	double a = D/2 - C*3.0/2 + B*3.0/2 - A/2;
	double b = A - B*5.0/2 + C*2 - D/2;
	double c = (C-A)/2;
	return a*t*t*t + b*t*t + c*t + B;
}

void BufferInterpolator::interpolate_channel_cubic(Array<float> &in, Array<float> &out) {
	if (in.num < 3) {
		interpolate_channel_linear(in, out);
		return;
	}

	out[0] = in[0];
	out.back() = in.back();
	for (int i=1; i<out.num-1; i++) {
		double x = (double)i * (double)(in.num - 1) / (double)(out.num - 1);
		int j = (int)x;
		double t = x - j;
		if (j >= 1 and j < in.num - 2) {
			out[i] = cubic_inter(in[j-1], in[j], in[j+1], in[j+2], t);
		} else if (j == 0) {
			out[i] = cubic_inter(in[1], in[0], in[1], in[2], t);
		} else if (j == in.num - 2) {
			out[i] = cubic_inter(in[j-1], in[j], in[j+1], in[j], t);
		}
	}
}


void BufferInterpolator::interpolate_channel_fourier(Array<float> &in, Array<float> &out) {
	Array<complex> z;

	FastFourierTransform::fft_r2c(in, z);
	z.resize(out.num / 2 + 1);

	int size = out.num;
	FastFourierTransform::fft_c2r_inv(z, out);
	out.resize(size);
}

void BufferInterpolator::interpolate_channel(Array<float> &in, Array<float> &out, Method method) {
	if (method == Method::LINEAR) {
		interpolate_channel_linear(in, out);
	} else if (method == Method::CUBIC) {
		interpolate_channel_cubic(in, out);
	} else if (method == Method::SINC) {
		// TODO...
	} else if (method == Method::FOURIER) {
		interpolate_channel_fourier(in, out);
	}
}

void BufferInterpolator::interpolate(AudioBuffer &in, AudioBuffer &out, Method method) {
	out.set_channels(in.channels);
	out.offset = in.offset;
	for (int i=0; i<in.channels; i++)
		interpolate_channel(in.c[i], out.c[i], method);
}
