/*
 * fft.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SRC_LIB_FFT_FFT_H_
#define SRC_LIB_FFT_FFT_H_

#include "../base/base.h"

struct complex;

namespace fft {
	void c2c(const Array<complex> &in, Array<complex> &out, bool inverse);
	void c2c_michi(const Array<complex> &in, Array<complex> &out, bool inverse);
	void r2c(const Array<float> &in, Array<complex> &out);
	void c2r_inv(const Array<complex> &in, Array<float> &out);

	void c2c_2d(const Array<complex> &in, Array<complex> &out, int n, bool inverse);
}

#endif /* SRC_LIB_FFT_FFT_H_ */
