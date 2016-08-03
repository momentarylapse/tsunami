/*
 * BufferBox.cpp
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#include "BufferBox.h"
#include "../lib/math/math.h"
//#include <math.h>
#include <assert.h>

SampleFormat format_for_bits(int bits)
{
	if (bits == 8)
		return SAMPLE_FORMAT_8;
	if (bits == 16)
		return SAMPLE_FORMAT_16;
	if (bits == 24)
		return SAMPLE_FORMAT_24;
	if (bits == 32)
		return SAMPLE_FORMAT_32;
	return SAMPLE_FORMAT_UNKNOWN;
}

int format_get_bits(SampleFormat format)
{
	if (format == SAMPLE_FORMAT_8)
		return 8;
	if ((format == SAMPLE_FORMAT_16) or (format == SAMPLE_FORMAT_16_BIGENDIAN))
		return 16;
	if ((format == SAMPLE_FORMAT_24) or (format == SAMPLE_FORMAT_24_BIGENDIAN))
		return 24;
	if ((format == SAMPLE_FORMAT_32) or (format == SAMPLE_FORMAT_32_BIGENDIAN) or (format == SAMPLE_FORMAT_32_FLOAT))
		return 32;
	return 0;
}

string format_name(SampleFormat format)
{
	if (format == SAMPLE_FORMAT_8)
		return "8 bit";
	if (format == SAMPLE_FORMAT_16)
		return "16 bit";
	if (format == SAMPLE_FORMAT_16_BIGENDIAN)
		return "16 bit BigEndian";
	if (format == SAMPLE_FORMAT_24)
		return "24 bit";
	if (format == SAMPLE_FORMAT_24_BIGENDIAN)
		return "24 bit BigEndian";
	if (format == SAMPLE_FORMAT_32)
		return "32 bit";
	if (format == SAMPLE_FORMAT_32_BIGENDIAN)
		return "32 bit BigEndian";
	if (format == SAMPLE_FORMAT_32_FLOAT)
		return "32 bit float";
	return "???";
}


BufferBox::BufferBox()
{
	offset = 0;
	length = 0;
	channels = 2;
	peaks_dirty = false;
}

BufferBox::BufferBox(const BufferBox &b)
{
	offset = b.offset;
	length = b.length;
	channels = b.channels;
	for (int i=0; i<channels; i++)
		c[i] = b.c[i];
	peaks_dirty = true;
}

void BufferBox::__init__()
{
	new(this) BufferBox;
}

void BufferBox::__delete__()
{
	this->BufferBox::~BufferBox();
}

void BufferBox::operator=(const BufferBox &b)
{
	offset = b.offset;
	length = b.length;
	channels = b.channels;
	for (int i=0; i<channels; i++)
		c[i] = b.c[i];
	peaks = b.peaks;
	peaks_dirty = b.peaks_dirty;
}

BufferBox::~BufferBox()
{
}

void BufferBox::clear()
{
	for (int i=0; i<channels; i++)
		c[i].clear();
	length = 0;
	peaks.clear();
	peaks_dirty = false;
}

void BufferBox::resize(int _length)
{
	if (_length < length)
		//peak.clear();
		invalidate_peaks(Range(offset + _length, length - _length));
	for (int i=0; i<channels; i++)
		c[i].resize(_length);
	length = _length;
}

bool BufferBox::is_ref() const
{	return ((length > 0) and (c[0].allocated == 0));	}

void fa_make_own(Array<float> &a)
{
	void *data = a.data;
	int num = a.num;
	a.clear();
	a.resize(num);
	memcpy(a.data, data, a.element_size * num);
}

void BufferBox::make_own()
{
	if (is_ref()){
		//msg_write("bb::make_own!");
		for (int i=0; i<channels; i++)
			fa_make_own(c[i]);
	}
}

void BufferBox::swap_ref(BufferBox &b)
{
	// buffer
	for (int i=0; i<channels; i++)
		c[i].exchange(b.c[i]);

	// num
	int t = length;
	length = b.length;
	b.length = t;

	// offset
	t = offset;
	offset = b.offset;
	b.offset = t;

	// channels
	t = channels;
	channels = b.channels;
	b.channels = t;
}

void BufferBox::append(BufferBox &b)
{
	int num0 = length;
	resize(length + b.length);
	set(b, num0, 1.0f);
}

void float_array_swap_values(Array<float> &a, Array<float> &b)
{
	for (int i=0;i<a.num;i++){
		float t = a[i];
		a[i] = b[i];
		b[i] = t;
	}
}

void BufferBox::swap_value(BufferBox &b)
{
	assert(length == b.length and "BufferBox.swap_value");
	// buffer
	for (int i=0; i<channels; i++)
		float_array_swap_values(c[i], b.c[i]);
	peaks.clear();
	b.peaks.clear();
}

void BufferBox::scale(float volume, float panning)
{
	if ((volume == 1.0f) and (panning == 0))
		return;

	float f[2];
	if (channels == 2){
		f[0] = volume * sin((panning + 1) / 4 * pi) * sqrt(2.0f);
		f[1] = volume * cos((panning + 1) / 4 * pi) * sqrt(2.0f);
	}else{
		f[0] = volume;
	}

	// scale
	for (int j=0; j<channels; j++)
		for (int i=0;i<length;i++)
			c[j][i] *= f[j];
}

void BufferBox::add(const BufferBox &b, int _offset, float volume, float panning)
{
	// relative to b
	int i0 = max(0, -_offset);
	int i1 = min(b.length, length - _offset);

	// add buffers
	if ((volume == 1.0f) and (panning == 0.0f)){
		for (int j=0; j<channels; j++)
			for (int i=i0;i<i1;i++)
				c[j][i + _offset] += b.c[j][i];
	}else{
		float f[2];
		if (channels == 2){
			f[0] = volume * sin((panning + 1) / 4 * pi) * sqrt(2.0f);
			f[1] = volume * cos((panning + 1) / 4 * pi) * sqrt(2.0f);
		}else{
			f[0] = volume;
		}
		for (int j=0; j<channels; j++)
			for (int i=i0;i<i1;i++)
				c[j][i + _offset] += b.c[j][i] * f[j];
	}
}

void BufferBox::set(const BufferBox &b, int _offset, float volume)
{
	// relative to b
	int i0 = max(0, -_offset);
	int i1 = min(b.length, length - _offset);
	if (i1 <= i0)
		return;

	// set buffers
	if (volume == 1.0f){
		for (int j=0; j<channels; j++)
			memcpy(&c[j][i0 + _offset], (float*)b.c[j].data + i0, sizeof(float) * (i1 - i0));
	}else{
		for (int j=0; j<channels; j++)
			for (int i=i0;i<i1;i++)
				c[j][i + _offset] = b.c[j][i] * volume;
	}
	invalidate_peaks(Range(i0 + _offset + offset, i1 - i0));
}

void BufferBox::set_as_ref(const BufferBox &b, int _offset, int _length)
{
	clear();
	length = _length;
	offset = _offset + b.offset;
	channels = b.channels;
	for (int i=0; i<channels; i++)
		c[i].set_ref(b.c[i].sub(_offset, _length));
}

#if 0
void BufferBox::set_16bit(const void *b, int offset, int length)
{
	// relative to b
	int i0 = max(0, - offset);
	int i1 = min(length, length - offset);
	length = i1 - i0;
	float *pr = &r[i0 + offset];
	float *pl = &l[i0 + offset];
	short *pb = &((short*)b)[i0 * 2];
	for (int i=0;i<length;i++){
		(*pr ++) = (float)(*pb ++) / 32768.0f;
		(*pl ++) = (float)(*pb ++) / 32768.0f;
	}
}
#endif

inline int invert_16(int i)
{
	return ((i & 0xff) << 8) + ((i & 0xff00) >> 8);
}

inline int invert_24(int i)
{
	unsigned int ui = i;
	unsigned int ext = (i < 0) ? 0xff000000 : 0;
	ui = ((ui & 0xff) << 16) | (ui & 0xff00) | ((ui & 0xff0000) >> 16) | ext;
	return (signed)ui;
}

inline int invert_32(int i)
{
	return ((i & 0xff) << 24) + ((i & 0xff00) << 8) + ((i & 0xff0000) >> 8) + ((i & 0xff000000) >> 24);
}

inline float import_24(int i)
{
	if ((i & 0x00800000) != 0)
		return (float)((i & 0x00ffffff) - 0x01000000) / 8388608.0f;
	return (float)(i & 0x00ffffff) / 8388608.0f;
}

void BufferBox::import(void *data, int _channels, SampleFormat format, int samples)
{
	char *cb = (char*)data;
	short *sb = (short*)data;
	int *ib = (int*)data;
	float *fb = (float*)data;

	for (int i=0;i<samples;i++){
		if (_channels == 2){
			if (format == SAMPLE_FORMAT_8){
				c[0][i] = (float)cb[i*2    ] / 128.0f;
				c[1][i] = (float)cb[i*2 + 1] / 128.0f;
			}else if (format == SAMPLE_FORMAT_16){
				c[0][i] = (float)sb[i*2    ] / 32768.0f;
				c[1][i] = (float)sb[i*2 + 1] / 32768.0f;
			}else if (format == SAMPLE_FORMAT_16_BIGENDIAN){
				c[0][i] = (float)invert_16(sb[i*2    ]) / 32768.0f;
				c[1][i] = (float)invert_16(sb[i*2 + 1]) / 32768.0f;
			}else if (format == SAMPLE_FORMAT_24){
				c[0][i] = import_24(*(int*)&cb[i*6    ]);
				c[1][i] = import_24(*(int*)&cb[i*6 + 3]);
			}else if (format == SAMPLE_FORMAT_24_BIGENDIAN){
				c[0][i] = (float)invert_24(*(int*)&cb[i*6    ] >> 8) / 8388608.0f;
				c[1][i] = (float)invert_24(*(int*)&cb[i*6 + 3] >> 8) / 8388608.0f;
			}else if (format == SAMPLE_FORMAT_32){
				c[0][i] = (float)ib[i*2  ] / 2147483648.0f;
				c[1][i] = (float)ib[i*2+1] / 2147483648.0f;
			}else if (format == SAMPLE_FORMAT_32_FLOAT){
				c[0][i] = fb[i*2];
				c[1][i] = fb[i*2+1];
			}else
				throw string("BufferBox.import: unhandled format");
		}else{
			if (format == SAMPLE_FORMAT_8){
				c[0][i] = (float)cb[i] / 128.0f;
			}else if (format == SAMPLE_FORMAT_16){
				c[0][i] = (float)sb[i] / 32768.0f;
			}else if (format == SAMPLE_FORMAT_16_BIGENDIAN){
				c[0][i] = (float)invert_16(sb[i]) / 32768.0f;
			}else if (format == SAMPLE_FORMAT_24){
				c[0][i] = import_24(*(int*)&cb[i*3]);
			}else if (format == SAMPLE_FORMAT_24_BIGENDIAN){
				c[0][i] = (float)invert_24(*(int*)&cb[i*3] >> 8) / 8388608.0f;
			}else if (format == SAMPLE_FORMAT_32){
				c[0][i] = (float)ib[i] / 2147483648.0f;
			}else if (format == SAMPLE_FORMAT_32_FLOAT){
				c[0][i] = fb[i];
			}else
				throw string("BufferBox.import: unhandled format");
			c[1][i] = c[0][i];
		}
	}
}



#define VAL_MAX_16		32766
#define VAL_ALERT_16	32770
#define VAL_MAX_24		8388606
#define VAL_ALERT_24	8388610

static bool wtb_overflow;

inline int set_data(float value, float scale, int clamp, int warn_thresh)
{
	int value_int = (int)(value * scale);
	if (value_int > clamp){
		if (value_int > warn_thresh)
			wtb_overflow = true;
		value_int = clamp;
	}else if (value_int < - clamp){
		if (value_int < -warn_thresh)
			wtb_overflow = true;
		return -clamp;
	}
	return value_int;
}

inline void set_data_16(short *data, float value)
{
	*data = set_data(value, 32768.0f, VAL_MAX_16, VAL_ALERT_16);
}

inline void set_data_24(int *data, float value)
{
	*data = set_data(value, 8388608.0f, VAL_MAX_24, VAL_ALERT_24) & 0x00ffffff;
}

bool BufferBox::_export(void *data, int _channels, SampleFormat format, bool align32) const
{
	wtb_overflow = false;

	if (format == SAMPLE_FORMAT_16){
		short *sb = (short*)data;
		int d = align32 ? 2 : 1;
		for (int i=0;i<length;i++){
			set_data_16(sb, c[0][i]);
			sb += d;
			set_data_16(sb, c[1][i]);
			sb += d;
		}
	}else if (format == SAMPLE_FORMAT_24){
		char *sc = (char*)data;
		int d = align32 ? 4 : 3;
		for (int i=0;i<length;i++){
			set_data_24((int*)sc, c[0][i]);
			sc += d;
			set_data_24((int*)sc, c[1][i]);
			sc += d;
		}
	}else if (format == SAMPLE_FORMAT_32_FLOAT){
		float *fc = (float*)data;
		for (int i=0;i<length;i++){
			*(fc ++) = c[0][i];
			*(fc ++) = c[1][i];
		}
	}else{
		//tsunami->log->error("invalid export format");
		msg_error("invalid export format");
	}

	return !wtb_overflow;
}

bool BufferBox::exports(string &data, int _channels, SampleFormat format) const
{
	data.resize(length * _channels * (format_get_bits(format) / 8));
	return _export(data.data, _channels, format, false);
}

inline float _clamp_(float f)
{
	if (f < -0.999f)
		return -0.999f;
	if (f > 0.999f)
		return 0.999f;
	return f;
}

void BufferBox::interleave(float *p, float volume) const
{
	float *pr = &c[0][0];
	float *pl = &c[1][0];
	if (volume == 1.0f){
		for (int i=0; i<length; i++){
			*p ++ = _clamp_(*pr ++);
			*p ++ = _clamp_(*pl ++);
		}
	}else{
		for (int i=0; i<length; i++){
			*p ++ = _clamp_((*pr ++) * volume);
			*p ++ = _clamp_((*pl ++) * volume);
		}
	}
}

void BufferBox::deinterleave(float *p, int num_channels)
{
	float *pr = &c[0][0];
	float *pl = &c[1][0];
	if (num_channels == 1){
		for (int i=0; i<length; i++){
			*pr ++ = *p;
			*pl ++ = *p ++;
		}
	}else if (num_channels == 2){
		for (int i=0; i<length; i++){
			*pr ++ = *p ++;
			*pl ++ = *p ++;
		}
	}
}

Range BufferBox::range() const
{
	return Range(offset, length);
}

Range BufferBox::range0() const
{
	return Range(0, length);
}

#define shrink_max(a, b)	max((a), (b))
#define shrink_mean(a, b)	(unsigned char)(sqrt(((float)(a) * (float)(a) + (float)(b) * (float)(b)) / 2))

void BufferBox::invalidate_peaks(const Range &_range)
{
	assert(range().covers(_range));
	int i0 = _range.start() - offset;
	int i1 = _range.end() - offset;
	int n = length;

	if (peaks.num < 4)
		peaks.resize(4);

	n /= 4;
	i0 /= 4;
	i1 = min(i1 / 4 + 1, n);
	//msg_write(format("inval %d  %d-%d", n, i0, i1));

	for (int k=0;k<2;k++)
		if (peaks[k].num < i1){
			int n0 = peaks[k].num;
			peaks[k].resize(i1);
			for (int i=n0; i<i0; i++)
				peaks[k][i] = 255;
		}
	for (int i=i0; i<i1; i++){
		peaks[0][i] = 255;
		peaks[1][i] = 255;
	}
	peaks_dirty = true;
}

inline void find_update_peak_range(string &p0, string &p1, int &i0, int &i1, int n)
{
	i0 = p0.num;
	i1 = n;
	//msg_write("t");
	bool found = false;
	for (int i=0;i<p0.num;i++)
		if (p0[i] == 255){
			//msg_write(format("i0: %d (%d)", i, p0[i]));
			i0 = i;
			found = true;
			break;
		}

	if (!found or (p0.num < n))
		return;

	for (int i=n-1;i>i0;i--)
		if (p0[i] == 255){
			//msg_write(format("i1: %d (%d)", i, p0[i]));
			i1 = i + 1;
			break;
		}
}

inline float fabsmax(float a, float b, float c, float d)
{
	a = fabs(a);
	b = fabs(b);
	c = fabs(c);
	d = fabs(d);
	return max(max(a, b), max(c, d));
}

void BufferBox::update_peaks()
{
	// first level
	if (peaks.num < 4)
		peaks.resize(4);
	int n = length / 4;
	int i0 = peaks[0].num;
	int i1 = n;
	if (peaks_dirty){
		find_update_peak_range(peaks[0], peaks[1], i0, i1, n);
	}
	peaks[0].resize(n);
	peaks[1].resize(n);
	peaks[2].resize(n);
	peaks[3].resize(n);
	//msg_write(format("  %d %d   %d  %d", i0, i1, channels, peaks.num));
	for (int i=i0;i<i1;i++){
		for (int j=0; j<channels; j++)
			peaks[j][i] = fabsmax(c[j][i * 4], c[j][i * 4 + 1], c[j][i * 4 + 2], c[j][i * 4 + 3]) * 254;
		peaks[2][i] = peaks[0][i];
		peaks[3][i] = peaks[1][i];
	}

	// higher levels
	int level = 4;
	while (n > 4){
		n = (n - 1) / 2;
		i0 = max((i0 - 1) / 2, 0);
		i1 = min((i1 + 1) / 2, n);
		if (peaks.num < level + 4)
			peaks.resize(level + 4);
		peaks[level    ].resize(n);
		peaks[level + 1].resize(n);
		peaks[level + 2].resize(n);
		peaks[level + 3].resize(n);
		for (int i=i0;i<i1;i++){
			peaks[level    ][i] = shrink_max(peaks[level - 4][i * 2], peaks[level - 4][i * 2 + 1]);
			peaks[level + 1][i] = shrink_max(peaks[level - 3][i * 2], peaks[level - 3][i * 2 + 1]);
			peaks[level + 2][i] = shrink_mean(peaks[level - 2][i * 2], peaks[level - 2][i * 2 + 1]);
			peaks[level + 3][i] = shrink_mean(peaks[level - 1][i * 2], peaks[level - 1][i * 2 + 1]);
		}

		level += 4;
	}
	peaks_dirty = false;
}
