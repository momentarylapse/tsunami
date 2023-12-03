#include "BufferPitchShift.h"
#include "BufferInterpolator.h"
#include "../../data/audio/AudioBuffer.h"
#include "../../lib/fft/fft.h"
#include "../../lib/math/complex.h"

namespace BufferPitchShift {

// FIXME: this sucks!



void ca_split(Array<complex> &z, Array<float> &x, Array<float> &y) {
	x.resize(z.num);
	y.resize(z.num);
	for (int i=0; i<z.num; i++) {
		x[i] = z[i].x;
		y[i] = z[i].y;
	}
}

void ca_join(Array<complex> &z, Array<float> &x, Array<float> &y) {
	z.resize(x.num);
	for (int i=0; i<z.num; i++)
		z[i] = complex(x[i], y[i]);
}

void pitch_shift_channel(Array<float> &buf, float factor) {
	Array<complex> z, zz;
	fft::r2c(buf, z);

	Array<float> x, y;
	ca_split(z, x, y);

	Array<float> xx, yy;
	xx.resize(x.num * factor);
	yy.resize(y.num * factor);
	BufferInterpolator::interpolate_channel_cubic(x, xx);
	BufferInterpolator::interpolate_channel_cubic(y, yy);
	xx.resize(x.num);
	yy.resize(y.num);
	ca_join(zz, xx, yy);
	for (auto &e: zz)
		e /= buf.num * factor;

	/*zz.resize(z.num);

	for (int i=0; i<z.num; i++)
		zz[i] = complex(0,0);

	for (int i=0; i<z.num; i++) {
		int j = (float)i * factor;
		if (j >= z.num)
			break;
		zz[j] += z[i] / z.num / 2;
	}*/

	fft::c2r_inv(zz, buf);
}

void pitch_shift(AudioBuffer &buf, float factor) {
	for (int i=0; i<buf.channels; i++)
		pitch_shift_channel(buf.c[i], factor);
}

void apply_fade_in(AudioBuffer &buf) {
	if (buf.length == 0)
		return;
	for (auto &c: buf.c)
		for (int i=0; i<buf.length; i++)
			c[i] *= (float)i / (float)buf.length;
}

void apply_fade_out(AudioBuffer &buf) {
	if (buf.length == 0)
		return;
	for (auto &c: buf.c)
		for (int i=0; i<buf.length; i++)
			c[i] *= (float)(buf.length - i) / (float)buf.length;
}

AudioBuffer dummy_stretch_equal_pitch(const AudioBuffer &buf, int new_size) {
	int CHUNK_SIZE = 1024;
	AudioBuffer buf_out(new_size, buf.channels);
	for (int i0=0; i0<new_size; i0+=CHUNK_SIZE) {
		int j0 = (int)((float)i0 * (float)buf.length / (float)new_size);
		int j1 = (int)((float)(i0 + CHUNK_SIZE) * (float)buf.length / (float)new_size) - CHUNK_SIZE;
		j0 = clamp(j0, 0, buf.length - CHUNK_SIZE);
		j1 = clamp(j1, 0, buf.length - CHUNK_SIZE);
		auto t0 = buf.cref(j0, j0 + CHUNK_SIZE);
		auto t1 = buf.cref(j1, j1 + CHUNK_SIZE);
		t0.make_own();
		t1.make_own();
		apply_fade_out(t0);
		buf_out.add(t0, i0, 1.0f);
		apply_fade_in(t1);
		buf_out.add(t1, i0, 1.0f);
	}
	return buf_out;
}



AudioBuffer scale_and_pitch_shift(const AudioBuffer &buf, int new_size, BufferInterpolator::Method scaling_method, float pitch_factor) {
	const float natural_pitch_factor = (float)new_size / (float)buf.length;
	const float EPSILON = 0.0001f;

	AudioBuffer buf_out(new_size, buf.channels);
	if (std::abs(pitch_factor - natural_pitch_factor) > EPSILON) {
		auto buf_pitched = dummy_stretch_equal_pitch(buf, buf.length * pitch_factor);
		BufferInterpolator::interpolate(buf_pitched, buf_out, scaling_method);
	} else {
		BufferInterpolator::interpolate(buf, buf_out, scaling_method);
	}
	return buf_out;
}

}

