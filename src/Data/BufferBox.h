/*
 * BufferBox.h
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#ifndef BUFFERBOX_H_
#define BUFFERBOX_H_

#include "../lib/file/file.h"
#include "Range.h"



enum SampleFormat
{
	SAMPLE_FORMAT_UNKNOWN,
	SAMPLE_FORMAT_8,
	SAMPLE_FORMAT_16,
	SAMPLE_FORMAT_16_BIGENDIAN,
	SAMPLE_FORMAT_24,
	SAMPLE_FORMAT_24_BIGENDIAN,
	SAMPLE_FORMAT_32,
	SAMPLE_FORMAT_32_BIGENDIAN,
	SAMPLE_FORMAT_32_FLOAT,
	NUM_SAMPLE_FORMATS
};

class BufferBox
{
public:
	BufferBox();
	BufferBox(const BufferBox &b);
	~BufferBox();
	void operator=(const BufferBox &b);
	void __assign__(const BufferBox &other){	*this = other;	}

	int offset, num;
	Array<float> r, l;

	Array<string> peaks;

	Range _cdecl range();
	Range _cdecl range0();

	void _cdecl clear();
	void _cdecl resize(int length);
	bool _cdecl is_ref();
	void _cdecl make_own();
	void _cdecl scale(float volume, float panning = 0);
	void _cdecl swap_ref(BufferBox &b);
	void _cdecl swap_value(BufferBox &b);
	void _cdecl append(BufferBox &b);
	void _cdecl set(const BufferBox &b, int offset, float volume);
	void _cdecl add(const BufferBox &b, int offset, float volume, float panning);
	void _cdecl set_16bit(const void *b, int offset, int length);
	void _cdecl set_as_ref(const BufferBox &b, int offset, int length);
	//void import(void *data, int channels, int bits, int samples);
	void _cdecl import(void *data, int channels, SampleFormat format, int samples);

	bool _cdecl get_16bit_buffer(Array<short> &data);
	void _cdecl interleave(float *p, float volume);
	void _cdecl deinterleave(float *p, int num_channels);

	enum PeakMode
	{
		PEAK_MODE_MAXIMUM,
		PEAK_MODE_SQUAREMEAN
	};

	void _cdecl invalidate_peaks(const Range &r);
	void _cdecl update_peaks(int mode);
};

SampleFormat format_for_bits(int bits);
int format_get_bits(SampleFormat);
string format_name(SampleFormat format);

#endif /* BUFFERBOX_H_ */
