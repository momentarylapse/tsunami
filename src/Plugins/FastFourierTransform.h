/*
 * FastFourierTransform.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef FASTFOURIERTRANSFORM_H_
#define FASTFOURIERTRANSFORM_H_

#include "../lib/file/file.h"
#include "../lib/types/types.h"

namespace FastFourierTransform
{
	void fft_c2c(Array<complex> &in, Array<complex> &out, bool inverse);
	void fft_r2c(Array<float> &in, Array<complex> &out);
	void fft_c2r_inv(Array<complex> &in, Array<float> &out);
	void fft_i2c(Array<int> &in, Array<complex> &out);
	void fft_c2i_inv(Array<complex> &in, Array<int> &out);
}

#endif /* FASTFOURIERTRANSFORM_H_ */
