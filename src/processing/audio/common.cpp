/*
 * common.cpp
 *
 *  Created on: 13 Aug 2023
 *      Author: michi
 */

#include "common.h"
#include "../../lib/math/complex.h"
#include "../../lib/math/math.h"

float sum(const Array<float> &a) {
	float r = 0;
	for (auto &x: a)
		r += x;
	return r;
}

float xmax(const Array<float> &a) {
	float r = 0;
	for (auto &x: a)
		if (x > r)
			r = x;
	return r;
}

float sum_abs(const Array<complex> &z) {
	float r = 0;
	for (auto &x: z)
		r += x.abs();
	return r;
}

float max_abs(const Array<complex> &z) {
	float r = 0;
	for (auto &x: z)
		r = max(r, x.abs());
	return r;
}

namespace tsunami {

void apply_window_function(Array<float> &data, WindowFunction wf) {
	if (wf == WindowFunction::Hann) {
		for (int k=0; k<data.num; k++) {
			float s = sin((float)k * pi / (data.num - 1));
			data[k] *= s*s * 2;
		}
	}
}

}
