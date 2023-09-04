/*
 * fft.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "fft.h"
#include "../math/complex.h"
#include "../os/msg.h"
#include <mutex>


#if HAS_LIB_FFTW3
#include <fftw3.h>
#endif

namespace fft
{


static std::mutex planer_mtx;

inline void c2c_4(const complex *in, complex *out, int stride) {
	complex e0 = in[0]      + in[stride*2];
	complex e1 = in[0]      - in[stride*2];
	complex o0 = in[stride] + in[stride*3];
	complex o1 = in[stride] - in[stride*3];
	out[0] = e0 + o0;
	out[1] = e1 - o1 * complex::I;
	out[2] = e0 - o0;
	out[3] = e1 + o1 * complex::I;
}

inline void c2c_inv_4(const complex *in, complex *out, int stride) {
	complex e0 = in[0]      + in[stride*2];
	complex e1 = in[0]      - in[stride*2];
	complex o0 = in[stride] + in[stride*3];
	complex o1 = in[stride] - in[stride*3];
	out[0] = e0 + o0;
	out[1] = e1 + o1 * complex::I;
	out[2] = e0 - o0;
	out[3] = e1 - o1 * complex::I;
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

inline void c2c_8(const complex *in, complex *out, int stride) {
	complex e[4], o[4];
	c2c_4(in, e, stride * 2);
	c2c_4(&in[stride], o, stride * 2);
	out[0] = e[0] + o[0];
	out[1] = e[1] + o[1] * exp_8[1];
	out[2] = e[2] + o[2] * exp_8[2];
	out[3] = e[3] + o[3] * exp_8[3];
	out[4] = e[0] - o[0];
	out[5] = e[1] - o[1] * exp_8[1];
	out[6] = e[2] - o[2] * exp_8[2];
	out[7] = e[3] - o[3] * exp_8[3];
}

inline void c2c_inv_8(const complex *in, complex *out, int stride) {
	complex e[4], o[4];
	c2c_inv_4(in, e, stride * 2);
	c2c_inv_4(&in[stride], o, stride * 2);
	out[0] = e[0] + o[0];
	out[1] = e[1] + o[1] * exp_8_inv[1];
	out[2] = e[2] + o[2] * exp_8_inv[2];
	out[3] = e[3] + o[3] * exp_8_inv[3];
	out[4] = e[0] - o[0];
	out[5] = e[1] - o[1] * exp_8_inv[1];
	out[6] = e[2] - o[2] * exp_8_inv[2];
	out[7] = e[3] - o[3] * exp_8_inv[3];
}

void c2c_2n(const complex *in, complex *out, int n, int n2, int stride, bool inv) {
	complex *o = fft_temp_out[n];
	complex *exp_const = inv ? exp_2n_inv[n] : exp_2n[n];
	if (n2 == 16) {
		if (inv) {
			c2c_inv_8(in, out, stride * 2);
			c2c_inv_8(&in[stride], o, stride * 2);
		} else {
			c2c_8(in, out, stride * 2);
			c2c_8(&in[stride], o, stride * 2);
		}
	} else {
		c2c_2n(in, out, n - 1, n2 >> 1, stride * 2, inv);
		c2c_2n(&in[stride], o, n - 1, n2 >> 1, stride * 2, inv);
	}
	int n22 = n2/2;
	out[n22] = out[0] - o[0];
	for (int i=1;i<n22;i++)
		out[n22 + i] = out[i] - o[i] * exp_const[i];
	out[0] += o[0];
	for (int i=1;i<n22;i++)
		out[i] += o[i] * exp_const[i];
}

// some pre-calculations
void _init_fft_(int n) {
	if ((exp_2n.num <= n) or (fft_temp_out.num <= n)) {
		fft_temp_out.resize(n+1);
		exp_2n.resize(n+1);
		exp_2n_inv.resize(n+1);
		int n2 = 1;
		for (int i=0;i<=n;i++) {
			if (!exp_2n[i]) {
				exp_2n[i]     = new complex[n2/2];
				exp_2n_inv[i] = new complex[n2/2];
				for (int k=0;k<n2/2;k++) {
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

void c2c_michi(const Array<complex> &in, Array<complex> &out, bool inverse) {
	out.resize(in.num);

	int n2 = in.num;
	int n = 0;
	while (n2 != (1 << n)) {
		n ++;
		if (n > 30) {
			msg_error("fft.c2c_michi: no power of two... in.num = " + i2s(in.num));
			return;
		}
	}

	_init_fft_(n);

	if (n == 2) {
		if (inverse)
			c2c_inv_4(&in[0], &out[0], 1);
		else
			c2c_4(&in[0], &out[0], 1);
	} else if (n == 3) {
		if (inverse)
			c2c_inv_8(&in[0], &out[0], 1);
		else
			c2c_8(&in[0], &out[0], 1);
	} else if (n > 3) {
		c2c_2n(&in[0], &out[0], n, n2, 1, inverse);
	}
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
	int_p ds = ((int_p)s & 15); \
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
	int ds = ((int)(int_p)s & 15); \
	if (ds != 0){ \
		asm volatile( \
			"sub %0, %%esp\n\t" \
			: : "r" (ds) \
			: "%esp"); \
	}

#else


#define align_stack

#endif

bool is_power_of_two(int i) {
	if ((i & 0x0000ffff) == 0)
		i >>= 16;
	if ((i & 0x000000ff) == 0)
		i >>= 8;
	if ((i & 0x0000000f) == 0)
		i >>= 4;
	if ((i & 0x00000007) == 0)
		i >>= 2;
	if ((i & 0x00000003) == 0)
		i >>= 1;
	return i == 1;
}

void c2c(const Array<complex> &in, Array<complex> &out, bool inverse) {
	out.resize(in.num);
	//align_stack

#if HAS_LIB_FFTW3
	planer_mtx.lock();
	fftwf_plan plan = fftwf_plan_dft_1d(in.num, (float(*)[2])in.data, (float(*)[2])out.data, inverse ? FFTW_BACKWARD : FFTW_FORWARD, FFTW_ESTIMATE);
	planer_mtx.unlock();
	fftwf_execute(plan);
	planer_mtx.lock();
	fftwf_destroy_plan(plan);
	planer_mtx.unlock();
#else
	if (is_power_of_two(in.num))
		c2c_michi(in, out, inverse);
	else
		msg_error("only 2^n ffts allowed! (program compiled without fftw3 support)");
#endif
}

void r2c(const Array<float> &in, Array<complex> &out) {
	if (in.num == 0)
		return;

	out.resize(in.num / 2 + 1);
	//align_stack

#if HAS_LIB_FFTW3
	planer_mtx.lock();
	fftwf_plan plan = fftwf_plan_dft_r2c_1d(in.num, (float*)in.data, (float(*)[2])out.data, FFTW_ESTIMATE | FFTW_PRESERVE_INPUT);
	planer_mtx.unlock();
	fftwf_execute(plan);
	planer_mtx.lock();
	fftwf_destroy_plan(plan);
	planer_mtx.unlock();
#else
	msg_error("no real->complex fft allowed! (program compiled without fftw3 support)");
#endif
}

void c2r_inv(const Array<complex> &in, Array<float> &out) {
	if (out.num == 0)
		return;

	//align_stack
	out.resize(in.num * 2 - 2);

#if HAS_LIB_FFTW3
	planer_mtx.lock();
	fftwf_plan plan = fftwf_plan_dft_c2r_1d(out.num, (float(*)[2])in.data, (float*)out.data, FFTW_ESTIMATE | FFTW_PRESERVE_INPUT);
	planer_mtx.unlock();
	fftwf_execute(plan);
	planer_mtx.lock();
	fftwf_destroy_plan(plan);
	planer_mtx.unlock();
#else
	msg_error("no complex->real fft allowed! (program compiled without fftw3 support)");
#endif
}


void c2c_2d(const Array<complex> &in, Array<complex> &out, int n, bool inverse) {
	out.resize(in.num);
	//align_stack

#if HAS_LIB_FFTW3
	planer_mtx.lock();
	fftwf_plan plan = fftwf_plan_dft_2d(n, in.num / n, (float(*)[2])in.data, (float(*)[2])out.data, inverse ? FFTW_BACKWARD : FFTW_FORWARD, FFTW_ESTIMATE);
	planer_mtx.unlock();
	fftwf_execute(plan);
	planer_mtx.lock();
	fftwf_destroy_plan(plan);
	planer_mtx.unlock();
#else
	msg_error("can not perform 2d fft (program compiled without fftw3 support)");
#endif
}

}
