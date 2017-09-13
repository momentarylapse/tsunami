/*
 * BufferInterpolator.cpp
 *
 *  Created on: 21.04.2017
 *      Author: michi
 */

#include "BufferInterpolator.h"
#include "../Plugins/FastFourierTransform.h"
#include "AudioBuffer.h"

BufferInterpolator::BufferInterpolator()
{
}

BufferInterpolator::~BufferInterpolator()
{
}

static void interpolate_channel(Array<float> &in, Array<float> &out, BufferInterpolator::Method method)
{
	if (method == BufferInterpolator::METHOD_LINEAR) {

		for (int i=0; i<out.num; i++) {
			double x = (double)i * (double)in.num / (double)out.num;
			int j = (int)x;
			double t = x - j;
			if (j < in.num - 1)
				out[i] = (1-t) * in[j] + t * in[j+1];
			else
				out[i] = in[j];
		}

	} else if (method == BufferInterpolator::METHOD_CUBIC) {

	} else if (method == BufferInterpolator::METHOD_SINC) {

	} else if (method == BufferInterpolator::METHOD_FOURIER) {

		Array<complex> z;

		FastFourierTransform::fft_r2c(in, z);
		z.resize(out.num / 2 + 1);

		int size = out.num;
		FastFourierTransform::fft_c2r_inv(z, out);
		out.resize(size);
	}
}

void BufferInterpolator::interpolate(AudioBuffer &in, AudioBuffer &out, int new_size, Method method)
{
	out.resize(new_size);
	for (int i=0; i<in.channels; i++)
		interpolate_channel(in.c[i], out.c[i], method);
	out.offset = in.offset;
}
