/*
 * ExtendedAudioBuffer.cpp
 *
 *  Created on: 01.02.2013
 *      Author: michi
 */

#include "FastFourierTransform.h"
#include "../lib/math/math.h"
#include <math.h>
#include "ExtendedAudioBuffer.h"


void ExtendedAudioBuffer::get_spectrum(Array<complex> &spec_r, Array<complex> &spec_l, int samples)
{
	FastFourierTransform::fft_r2c(c[0], spec_r);
	FastFourierTransform::fft_r2c(c[1], spec_l);
}

