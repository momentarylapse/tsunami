/*
 * AudioBuffer.h
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#ifndef SRC_DATA_AUDIOBUFFER_H_
#define SRC_DATA_AUDIOBUFFER_H_

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

class AudioBuffer
{
public:
	AudioBuffer();
	AudioBuffer(const AudioBuffer &b);
	~AudioBuffer();

	void _cdecl __init__();
	void _cdecl __delete__();

	void operator=(const AudioBuffer &b);
	void __assign__(const AudioBuffer &other){ *this = other; }

	int offset, length;
	int channels;
	Array<float> c[2];

	Array<string> peaks;

	Range _cdecl range() const;
	Range _cdecl range0() const;

	void _cdecl clear();
	void _cdecl resize(int length);
	bool _cdecl is_ref() const;
	void _cdecl make_own();
	void _cdecl scale(float volume, float panning = 0);
	void _cdecl swap_ref(AudioBuffer &b);
	void _cdecl swap_value(AudioBuffer &b);
	void _cdecl append(AudioBuffer &b);
	void _cdecl set(const AudioBuffer &b, int offset, float volume);
	void _cdecl add(const AudioBuffer &b, int offset, float volume, float panning);
	void _cdecl set_as_ref(const AudioBuffer &b, int offset, int length);
	void _cdecl import(void *data, int channels, SampleFormat format, int samples);

	bool _cdecl _export(void *data, int channels, SampleFormat format, bool align32) const;
	bool _cdecl exports(string &data, int channels, SampleFormat format) const;
	void _cdecl interleave(float *p, float volume) const;
	void _cdecl deinterleave(float *p, int num_channels);


	static const int PEAK_CHUNK_EXP;
	static const int PEAK_CHUNK_SIZE;
	static const int PEAK_OFFSET_EXP;
	static const int PEAK_FINEST_SIZE;
	static const int PEAK_MAGIC_LEVEL4;

	void _cdecl invalidate_peaks(const Range &r);
	void _cdecl update_peaks();
};

SampleFormat format_for_bits(int bits);
int format_get_bits(SampleFormat);
string format_name(SampleFormat format);

#endif /* SRC_DATA_AUDIOBUFFER_H_ */
