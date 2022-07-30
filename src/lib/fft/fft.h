/*
 * fft.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SRC_LIB_FFT_FFT_H_
#define SRC_LIB_FFT_FFT_H_

#include "../base/base.h"

class complex;

namespace fft {
	void _cdecl c2c(const Array<complex> &in, Array<complex> &out, bool inverse);
	void _cdecl c2c_michi(const Array<complex> &in, Array<complex> &out, bool inverse);
	void _cdecl r2c(const Array<float> &in, Array<complex> &out);
	void _cdecl c2r_inv(const Array<complex> &in, Array<float> &out);
}

#endif /* SRC_LIB_FFT_FFT_H_ */
