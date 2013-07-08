/*
 * BufferBox.cpp
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#include "BufferBox.h"
#include <math.h>
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
	if ((format == SAMPLE_FORMAT_16) || (format == SAMPLE_FORMAT_16_BIGENDIAN))
		return 16;
	if ((format == SAMPLE_FORMAT_24) || (format == SAMPLE_FORMAT_24_BIGENDIAN))
		return 24;
	if ((format == SAMPLE_FORMAT_32) || (format == SAMPLE_FORMAT_32_BIGENDIAN) || (format == SAMPLE_FORMAT_32_FLOAT))
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
	num = 0;
}

BufferBox::BufferBox(const BufferBox &b)
{
	offset = b.offset;
	num = b.num;
	r = b.r;
	l = b.l;
}

void BufferBox::operator=(const BufferBox &b)
{
	offset = b.offset;
	num = b.num;
	r = b.r;
	l = b.l;
	peak.clear();
}

BufferBox::~BufferBox()
{
}

void BufferBox::clear()
{
	r.clear();
	l.clear();
	num = 0;
	peak.clear();
}

void BufferBox::resize(int length)
{
	if (length < num)
		//peak.clear();
		invalidate_peaks(Range(offset + length, num - length));
	r.resize(length);
	l.resize(length);
	num = length;
}

bool BufferBox::is_ref()
{	return ((num > 0) && (r.allocated == 0));	}

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
		fa_make_own(r);
		fa_make_own(l);
	}
}

void BufferBox::swap_ref(BufferBox &b)
{
	// buffer
	r.exchange(b.r);
	l.exchange(b.l);

	// num
	int t = num;
	num = b.num;
	b.num = t;

	// offset
	t = offset;
	offset = b.offset;
	b.offset = t;
}

void BufferBox::append(BufferBox &b)
{
	int num0 = num;
	resize(num + b.num);
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
	assert(num == b.num && "BufferBox.swap_value");
	// buffer
	float_array_swap_values(r, b.r);
	float_array_swap_values(l, b.l);
	peak.clear();
	b.peak.clear();
}

void BufferBox::scale(float volume, float panning)
{
	if ((volume == 1.0f) && (panning == 0))
		return;
	make_own();

	float f_r = volume * sin((panning + 1) / 4 * M_PI) * sqrt(2);
	float f_l = volume * cos((panning + 1) / 4 * M_PI) * sqrt(2);

	// scale
	for (int i=0;i<r.num;i++){
		r[i] *= f_r;
		l[i] *= f_l;
	}
}

void BufferBox::add(const BufferBox &b, int offset, float volume, float panning)
{
	// relative to b
	int i0 = max(0, -offset);
	int i1 = min(b.r.num, r.num - offset);

	// add buffers
	if ((volume == 1.0f) && (panning == 0.0f)){
		for (int i=i0;i<i1;i++){
			r[i + offset] += b.r[i];
			l[i + offset] += b.l[i];
		}
	}else{
		float f_r = volume * sin((panning + 1) / 4 * M_PI) * sqrt(2);
		float f_l = volume * cos((panning + 1) / 4 * M_PI) * sqrt(2);
		for (int i=i0;i<i1;i++){
			r[i + offset] += b.r[i] * f_r;
			l[i + offset] += b.l[i] * f_l;
		}
	}
}

void BufferBox::set(const BufferBox &b, int _offset, float volume)
{
	// relative to b
	int i0 = max(0, -_offset);
	int i1 = min(b.r.num, r.num - _offset);
	if (i1 <= i0)
		return;

	// set buffers
	if (volume == 1.0f){
		memcpy(&r[i0 + _offset], (float*)b.r.data + i0, sizeof(float) * (i1 - i0));
		memcpy(&l[i0 + _offset], (float*)b.l.data + i0, sizeof(float) * (i1 - i0));
	}else{
		for (int i=i0;i<i1;i++){
			r[i + _offset] = b.r[i] * volume;
			l[i + _offset] = b.l[i] * volume;
		}
	}
	invalidate_peaks(Range(i0 + _offset + offset, i1 - i0));
}

void BufferBox::set_as_ref(const BufferBox &b, int _offset, int _length)
{
	clear();
	num = _length;
	offset = _offset + b.offset;
	r.set_ref(b.r.sub(_offset, _length));
	l.set_ref(b.l.sub(_offset, _length));
}

void BufferBox::set_16bit(const void *b, int offset, int length)
{
	// relative to b
	int i0 = max(0, - offset);
	int i1 = min(length, num - offset);
	length = i1 - i0;
	float *pr = &r[i0 + offset];
	float *pl = &l[i0 + offset];
	short *pb = &((short*)b)[i0 * 2];
	for (int i=0;i<length;i++){
		(*pr ++) = (float)(*pb ++) / 32768.0f;
		(*pl ++) = (float)(*pb ++) / 32768.0f;
	}
}

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


void BufferBox::import(void *data, int channels, SampleFormat format, int samples)
{
	char *cb = (char*)data;
	short *sb = (short*)data;

	for (int i=0;i<samples;i++){
		if (channels == 2){
			if (format == SAMPLE_FORMAT_8){
				r[i] = (float)cb[i*2] / 128.0f;
				l[i] = (float)cb[i*2+1] / 128.0f;
			}else if (format == SAMPLE_FORMAT_16){
				r[i] = (float)sb[i*2] / 32768.0f;
				l[i] = (float)sb[i*2+1] / 32768.0f;
			}else if (format == SAMPLE_FORMAT_16_BIGENDIAN){
				r[i] = (float)invert_16(sb[i*2]) / 32768.0f;
				l[i] = (float)invert_16(sb[i*2+1]) / 32768.0f;
			}else if (format == SAMPLE_FORMAT_24){
				r[i] = (float)*(short*)&cb[i*6 + 1] / 32768.0f; // only high 16 bits
				l[i] = (float)*(short*)&cb[i*6 + 4] / 32768.0f;
			}else if (format == SAMPLE_FORMAT_24_BIGENDIAN){
				r[i] = (float)invert_24(*(int*)&cb[i*6 + 0] >> 8) / 32768.0f / 256;
				l[i] = (float)invert_24(*(int*)&cb[i*6 + 3] >> 8) / 32768.0f / 256;
			}else if (format == SAMPLE_FORMAT_32){
				r[i] = (float)sb[i*4+1] / 32768.0f; // only high 16 bits...
				l[i] = (float)sb[i*4+3] / 32768.0f;
			}else if (format == SAMPLE_FORMAT_32_FLOAT){
				r[i] = *(float*)&sb[i*4];
				l[i] = *(float*)&sb[i*4+2];
			}else
				throw string("BufferBox.import: unhandled format");
		}else{
			if (format == SAMPLE_FORMAT_8){
				r[i] = (float)cb[i] / 128.0f;
			}else if (format == SAMPLE_FORMAT_16){
				r[i] = (float)sb[i] / 32768.0f;
			}else if (format == SAMPLE_FORMAT_16_BIGENDIAN){
				r[i] = (float)invert_16(sb[i]) / 32768.0f;
			}else if (format == SAMPLE_FORMAT_24){
				r[i] = (float)*(short*)&cb[i*3 + 1] / 32768.0f;
			}else if (format == SAMPLE_FORMAT_24_BIGENDIAN){
				r[i] = (float)invert_24(*(int*)&cb[i*3] >> 8) / 32768.0f / 256;
			}else if (format == SAMPLE_FORMAT_32){
				r[i] = (float)sb[i*2+1] / 32768.0f;
			}else if (format == SAMPLE_FORMAT_32_FLOAT){
				r[i] = *(float*)&sb[i*2];
			}else
				throw string("BufferBox.import: unhandled format");
			l[i] = r[i];
		}
	}
}



#define val_max		32766
#define val_alert	32770

static bool wtb_overflow;

inline void set_data(short *data, float value)
{
	int value_int = (int)(value * 32768.0f);
	if (value_int > val_max){
		if (value_int > val_alert)
			wtb_overflow = true;
		value_int = val_max;
	}else if (value_int < - val_max){
		if (value_int < -val_alert)
			wtb_overflow = true;
		value_int = -val_max;
	}
	*data = value_int;
}

bool BufferBox::get_16bit_buffer(Array<short> &data)
{
	wtb_overflow = false;

	data.resize(num * 2);
	short *b = &data[0];
	for (int i=0;i<num;i++){
		set_data(b ++, r[i]);
		set_data(b ++, l[i]);
	}

	return !wtb_overflow;
}

Range BufferBox::range()
{
	return Range(offset, num);
}

#define shrink_max(a, b)	max((a), (b))
#define shrink_mean(a, b)	(unsigned char)(sqrt(((float)(a) * (float)(a) + (float)(b) * (float)(b)) / 2))

void BufferBox::invalidate_peaks(const Range &_range)
{
	assert(range().covers(_range));
	int i0 = _range.start() - range().start();
	int i1 = _range.end() - range().start();
	int n = r.num;

	if (peak.num < 2)
		peak.resize(2);

	n /= 4;
	i0 /= 4;
	i1 = min(i1 / 4 + 1, n);
	//msg_write(format("inval %d  %d-%d", n, i0, i1));

	for (int k=0;k<2;k++)
		if (peak[k].num < n){
			int n0 = peak[k].num;
			peak[k].resize(n);
			peak[k][n0] = -1;
		}
	for (int i=i0;i<i1;i++){
		peak[0][i] = -1;
		peak[1][i] = -1;
	}
}

inline void find_update_peak_range(string &p0, string &p1, int &i0, int &i1, int n)
{
	i0 = 0;
	i1 = n;
	if ((p0.num < n) || (p1.num < n))
		return;
	//msg_write("t");
	bool found = false;
	for (int i=0;i<n;i++)
		if (p0[i] == -1){
			//msg_write(format("i0: %d (%d)", i, p0[i]));
			i0 = i;
			found = true;
			break;
		}
	if (!found)
		i0 = n;
	for (int i=n-1;i>i0;i--)
		if (p0[i] == -1){
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

void BufferBox::update_peaks(int mode)
{
	// first level
	if (peak.num < 2)
		peak.resize(2);
	int n = r.num / 4;
	int i0 = 0;
	int i1 = n;
	if ((peak[0].num >= n) && (peak[1].num >= n))
		find_update_peak_range(peak[0], peak[1], i0, i1, n);
	peak[0].resize(n);
	peak[1].resize(n);
	//msg_write(format("  %d %d", i0, i1));
	for (int i=i0;i<i1;i++){
		peak[0][i] = fabsmax(r[i * 4], r[i * 4 + 1], r[i * 4 + 2], r[i * 4 + 3]) * 254;
		peak[1][i] = fabsmax(l[i * 4], l[i * 4 + 1], l[i * 4 + 2], l[i * 4 + 3]) * 254;
	}

	// higher levels
	int level = 2;
	while (n > 4){
		n /= 2;
		i0 /= 2;
		i1 = min((i1 + 1) / 2, n);
		if (peak.num < level + 2)
			peak.resize(level + 2);
		peak[level    ].resize(n);
		peak[level + 1].resize(n);
		if (mode == PEAK_MODE_MAXIMUM){
			for (int i=i0;i<i1;i++){
				peak[level    ][i] = shrink_max((unsigned char)peak[level - 2][i * 2], (unsigned char)peak[level - 2][i * 2 + 1]);
				peak[level + 1][i] = shrink_max((unsigned char)peak[level - 1][i * 2], (unsigned char)peak[level - 1][i * 2 + 1]);
			}
		}else if (mode == PEAK_MODE_SQUAREMEAN){
			for (int i=i0;i<i1;i++){
				peak[level    ][i] = shrink_mean((unsigned char)peak[level - 2][i * 2], (unsigned char)peak[level - 2][i * 2 + 1]);
				peak[level + 1][i] = shrink_mean((unsigned char)peak[level - 1][i * 2], (unsigned char)peak[level - 1][i * 2 + 1]);
			}
		}

		level += 2;
	}
}
