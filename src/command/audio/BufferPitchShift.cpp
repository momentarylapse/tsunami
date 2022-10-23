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


}

