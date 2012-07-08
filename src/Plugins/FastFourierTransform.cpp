/*
 * FastFourierTransform.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "FastFourierTransform.h"

#include <fftw3.h>

namespace FastFourierTransform
{

void fft_c2c(Array<complex> &in, Array<complex> &out, bool inverse)
{
	msg_db_r("fft_c2c", 1);
	if (out.num < in.num)
		out.resize(in.num);
	fftwf_plan plan = fftwf_plan_dft_1d(in.num, (float(*)[2])in.data, (float(*)[2])out.data, inverse ? FFTW_BACKWARD : FFTW_FORWARD, FFTW_ESTIMATE);
	fftwf_execute(plan);
	fftwf_destroy_plan(plan);
	msg_db_l(1);
}

void fft_r2c(Array<float> &in, Array<complex> &out)
{
	if (out.num < in.num / 2 + 1){
		msg_error("fft_r2c: out.num < in.num / 2 + 1");
		return;
	}
	msg_db_r("fft_r2c", 1);
	/*if (out.num < in.num / 2 + 1)
		out.resize(in.num / 2 + 1);*/
	fftwf_plan plan = fftwf_plan_dft_r2c_1d(in.num, (float*)in.data, (float(*)[2])out.data, FFTW_ESTIMATE | FFTW_PRESERVE_INPUT);
	fftwf_execute(plan);
	fftwf_destroy_plan(plan);
	msg_db_l(1);
}

void fft_c2r_inv(Array<complex> &in, Array<float> &out)
{
	if (in.num < out.num / 2 - 1){
		msg_error("fft_c2r_inv: in.num < out.num / 2 - 1");
		return;
	}
	msg_db_r("fft_c2r_inv", 1);
	fftwf_plan plan = fftwf_plan_dft_c2r_1d(out.num, (float(*)[2])in.data, (float*)out.data, FFTW_ESTIMATE | FFTW_PRESERVE_INPUT);
	fftwf_execute(plan);
	fftwf_destroy_plan(plan);
	msg_db_l(1);
}

void fft_i2c(Array<int> &in, Array<complex> &out)
{
	msg_db_r("fft_i2c", 1);
	Array<float> inf;
	for (int i=0;i<in.num;i++)
		inf.add(in[i]);
	fft_r2c(inf, out);
	msg_db_l(1);
}

void fft_c2i_inv(Array<complex> &in, Array<int> &out)
{
	msg_db_r("fft_c2i_inv", 1);
	Array<float> outf;
	fft_c2r_inv(in, outf);
	if (out.num < outf.num)
		out.resize(outf.num);
	for (int i=0;i<outf.num;i++)
		out[i] = outf[i];
	msg_db_l(1);
}

}
