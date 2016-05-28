/*
 * FastFourierTransform.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "FastFourierTransform.h"

#ifndef OS_WINDOWS
#include <fftw3.h>
#endif

namespace FastFourierTransform
{

inline void fft_c2c_4(complex *in, complex *out, int stride)
{
	complex e0 = in[0]      + in[stride*2];
	complex e1 = in[0]      - in[stride*2];
	complex o0 = in[stride] + in[stride*3];
	complex o1 = in[stride] - in[stride*3];
	out[0] = e0 + o0;
	out[1] = e1 - o1 * c_i;
	out[2] = e0 - o0;
	out[3] = e1 + o1 * c_i;
}

inline void fft_c2c_inv_4(complex *in, complex *out, int stride)
{
	complex e0 = in[0]      + in[stride*2];
	complex e1 = in[0]      - in[stride*2];
	complex o0 = in[stride] + in[stride*3];
	complex o1 = in[stride] - in[stride*3];
	out[0] = e0 + o0;
	out[1] = e1 + o1 * c_i;
	out[2] = e0 - o0;
	out[3] = e1 - o1 * c_i;
}

static const complex exp_8[4] = {
		complex(1, 0),
		complex(sqrt(0.5f), -sqrt(0.5f)),
		complex(0, -1),
		complex(-sqrt(0.5f), -sqrt(0.5f)),
};
static const complex exp_8_inv[4] = {
		complex(1, 0),
		complex(sqrt(0.5f), sqrt(0.5f)),
		complex(0, 1),
		complex(-sqrt(0.5f), sqrt(0.5f)),
};
static Array<complex*> exp_2n, exp_2n_inv;
static Array<complex*> fft_temp_out;

inline void fft_c2c_8(complex *in, complex *out, int stride)
{
	complex e[4], o[4];
	fft_c2c_4(in, e, stride * 2);
	fft_c2c_4(&in[stride], o, stride * 2);
	out[0] = e[0] + o[0];
	out[1] = e[1] + o[1] * exp_8[1];
	out[2] = e[2] + o[2] * exp_8[2];
	out[3] = e[3] + o[3] * exp_8[3];
	out[4] = e[0] - o[0];
	out[5] = e[1] - o[1] * exp_8[1];
	out[6] = e[2] - o[2] * exp_8[2];
	out[7] = e[3] - o[3] * exp_8[3];
}

inline void fft_c2c_inv_8(complex *in, complex *out, int stride)
{
	complex e[4], o[4];
	fft_c2c_inv_4(in, e, stride * 2);
	fft_c2c_inv_4(&in[stride], o, stride * 2);
	out[0] = e[0] + o[0];
	out[1] = e[1] + o[1] * exp_8_inv[1];
	out[2] = e[2] + o[2] * exp_8_inv[2];
	out[3] = e[3] + o[3] * exp_8_inv[3];
	out[4] = e[0] - o[0];
	out[5] = e[1] - o[1] * exp_8_inv[1];
	out[6] = e[2] - o[2] * exp_8_inv[2];
	out[7] = e[3] - o[3] * exp_8_inv[3];
}

void fft_c2c_2n(complex *in, complex *out, int n, int n2, int stride, bool inv)
{
	complex *o = fft_temp_out[n];
	complex *exp_const = inv ? exp_2n_inv[n] : exp_2n[n];
	if (n2 == 16){
		if (inv){
			fft_c2c_inv_8(in, out, stride * 2);
			fft_c2c_inv_8(&in[stride], o, stride * 2);
		}else{
			fft_c2c_8(in, out, stride * 2);
			fft_c2c_8(&in[stride], o, stride * 2);
		}
	}else{
		fft_c2c_2n(in, out, n - 1, n2 >> 1, stride * 2, inv);
		fft_c2c_2n(&in[stride], o, n - 1, n2 >> 1, stride * 2, inv);
	}
	int n22 = n2/2;
	out[n22] = out[0] - o[0];
	for (int i=1;i<n22;i++)
		out[n22 + i] = out[i] - o[i] * exp_const[i];
	out[0] += o[0];
	for (int i=1;i<n22;i++)
		out[i] += o[i] * exp_const[i];
}

void _init_fft_(int n)
{
	if ((exp_2n.num <= n) or (fft_temp_out.num <= n)){
		fft_temp_out.resize(n+1);
		exp_2n.resize(n+1);
		exp_2n_inv.resize(n+1);
		int n2 = 1;
		for (int i=0;i<=n;i++){
			if (!exp_2n[i]){
				exp_2n[i]     = new complex[n2/2];
				exp_2n_inv[i] = new complex[n2/2];
				for (int k=0;k<n2/2;k++){
					exp_2n[i][k]     = complex(cos(2*pi / (float)n2 * (float)k), -sin(2*pi / (float)n2 * (float)k));
					exp_2n_inv[i][k] = complex(cos(2*pi / (float)n2 * (float)k),  sin(2*pi / (float)n2 * (float)k));
				}
			}
			if (!fft_temp_out[i])
				fft_temp_out[i] = new complex[n2/2];
			n2 *= 2;
		}
	}
}

void fft_c2c_michi(Array<complex> &in, Array<complex> &out, bool inverse)
{
	msg_db_f("fft_c2c_michi", 1);
	out.resize(in.num);

	int n2 = in.num;
	int n = 0;
	while (n2 != (1 << n))
		n ++;

	if (n == 2){
		if (inverse)
			fft_c2c_inv_4(&in[0], &out[0], 1);
		else
			fft_c2c_4(&in[0], &out[0], 1);
	}else if (n == 3){
		if (inverse)
			fft_c2c_inv_8(&in[0], &out[0], 1);
		else
			fft_c2c_8(&in[0], &out[0], 1);
	}else if (n > 3){
		_init_fft_(n);
		fft_c2c_2n(&in[0], &out[0], n, n2, 1, inverse);
	}else
		msg_error("fft_c2c_michi: no power of two... in.num = " + i2s(in.num));
}

#if defined(__x86_64__)

#define align_stack \
	void *s; \
	asm volatile( \
		"movq %%rsp, %%rax\n\t" \
		"movq %%rax, %0\n\t" \
		: "=r" (s) \
		: : "%rax", "%rsp"); \
	string ttt = p2s(s); \
	long ds = ((long)s & 15); \
	if (ds != 0){ \
		asm volatile( \
			"subq %0, %%rsp\n\t" \
			: : "r" (ds) \
			: "%rsp"); \
	}
#elif defined(__i386__)

#define align_stack \
	void *s; \
	asm volatile( \
		"mov %%esp, %%eax\n\t" \
		"mov %%eax, %0\n\t" \
		: "=r" (s) \
		: : "%eax", "%esp"); \
	string ttt = p2s(s); \
	int ds = ((int)(long)s & 15); \
	if (ds != 0){ \
		asm volatile( \
			"sub %0, %%esp\n\t" \
			: : "r" (ds) \
			: "%esp"); \
	}

#else


#define align_stack

#endif

void fft_c2c(Array<complex> &in, Array<complex> &out, bool inverse)
{
	msg_db_f("fft_c2c", 1);
	out.resize(in.num);
	align_stack
#ifndef OS_WINDOWS
	fftwf_plan plan = fftwf_plan_dft_1d(in.num, (float(*)[2])in.data, (float(*)[2])out.data, inverse ? FFTW_BACKWARD : FFTW_FORWARD, FFTW_ESTIMATE);
	fftwf_execute(plan);
	fftwf_destroy_plan(plan);
#endif
}

void fft_r2c(Array<float> &in, Array<complex> &out)
{
	if (in.num == 0)
		return;
	msg_db_f("fft_r2c", 1);
	out.resize(in.num / 2 + 1);
	align_stack
#ifndef OS_WINDOWS
	fftwf_plan plan = fftwf_plan_dft_r2c_1d(in.num, (float*)in.data, (float(*)[2])out.data, FFTW_ESTIMATE | FFTW_PRESERVE_INPUT);
	fftwf_execute(plan);
	fftwf_destroy_plan(plan);
#endif
}

void fft_c2r_inv(Array<complex> &in, Array<float> &out)
{
	if (out.num == 0)
		return;
	msg_db_f("fft_c2r_inv", 1);
	align_stack
	out.resize(in.num * 2 - 2);
#ifndef OS_WINDOWS
	fftwf_plan plan = fftwf_plan_dft_c2r_1d(out.num, (float(*)[2])in.data, (float*)out.data, FFTW_ESTIMATE | FFTW_PRESERVE_INPUT);
	fftwf_execute(plan);
	fftwf_destroy_plan(plan);
#endif
}


}
