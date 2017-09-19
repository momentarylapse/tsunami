/*
 * BufferBox.cpp
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#include "AudioBuffer.h"

#include "../lib/math/math.h"
//#include <math.h>
#include <assert.h>



#include "../Tsunami.h"
#include "Song.h"
#include "../lib/threads/Thread.h"

// peaks:
// ...


const int AudioBuffer::PEAK_CHUNK_EXP = 15;
const int AudioBuffer::PEAK_CHUNK_SIZE = 1<<PEAK_CHUNK_EXP;
const int AudioBuffer::PEAK_OFFSET_EXP = 3;
const int AudioBuffer::PEAK_FINEST_SIZE = 1<<PEAK_OFFSET_EXP;
const int AudioBuffer::PEAK_MAGIC_LEVEL4 = (PEAK_CHUNK_EXP - PEAK_OFFSET_EXP)*4;

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


AudioBuffer::AudioBuffer()
{
	offset = 0;
	length = 0;
	channels = 2;
}

AudioBuffer::AudioBuffer(const AudioBuffer &b)
{
	offset = b.offset;
	length = b.length;
	channels = b.channels;
	for (int i=0; i<channels; i++)
		c[i] = b.c[i];
}

void AudioBuffer::__init__()
{
	new(this) AudioBuffer;
}

void AudioBuffer::__delete__()
{
	this->AudioBuffer::~AudioBuffer();
}

void AudioBuffer::operator=(const AudioBuffer &b)
{
	offset = b.offset;
	length = b.length;
	channels = b.channels;
	for (int i=0; i<channels; i++)
		c[i] = b.c[i];
	peaks = b.peaks;
}

AudioBuffer::~AudioBuffer()
{
}

void AudioBuffer::clear()
{
	for (int i=0; i<channels; i++)
		c[i].clear();
	length = 0;
	peaks.clear();
}

void truncate_peaks(AudioBuffer &buf, int length)
{
	int level4 = 0;
	length /= buf.PEAK_FINEST_SIZE;
	while(level4 < buf.peaks.num){
		for (int k=0; k<4; k++)
			buf.peaks[level4 + k].resize(length);
		level4 += 4;
		length /= 2;
	}

}

void AudioBuffer::resize(int _length)
{
	if (_length < length)
		truncate_peaks(*this, _length);
	for (int i=0; i<channels; i++)
		c[i].resize(_length);
	length = _length;
}

bool AudioBuffer::is_ref() const
{	return ((length > 0) and (c[0].allocated == 0));	}

void fa_make_own(Array<float> &a)
{
	void *data = a.data;
	int num = a.num;
	a.clear();
	a.resize(num);
	memcpy(a.data, data, a.element_size * num);
}

void AudioBuffer::make_own()
{
	if (is_ref()){
		//msg_write("bb::make_own!");
		for (int i=0; i<channels; i++)
			fa_make_own(c[i]);
	}
}

void AudioBuffer::swap_ref(AudioBuffer &b)
{
	// buffer
	for (int i=0; i<channels; i++)
		c[i].exchange(b.c[i]);

	// peaks
	peaks.exchange(b.peaks);

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

void AudioBuffer::append(AudioBuffer &b)
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

void AudioBuffer::swap_value(AudioBuffer &b)
{
	assert(length == b.length and "BufferBox.swap_value");
	// buffer
	for (int i=0; i<channels; i++)
		float_array_swap_values(c[i], b.c[i]);
	peaks.clear();
	b.peaks.clear();
}

void AudioBuffer::scale(float volume, float panning)
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

	peaks.clear();
}

void AudioBuffer::add(const AudioBuffer &b, int _offset, float volume, float panning)
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
	invalidate_peaks(Range(i0 + _offset + offset, i1 - i0));
}

inline void _buf_copy_samples_(AudioBuffer &target, int target_offset, const AudioBuffer &source, int source_offset, int length)
{
	for (int j=0; j<source.channels; j++)
		memcpy(&target.c[j][target_offset], &source.c[j][source_offset], sizeof(float) * length);
}

inline void _buf_copy_samples_scale_(AudioBuffer &target, int target_offset, const AudioBuffer &source, int source_offset, int length, float volume)
{
	for (int j=0; j<source.channels; j++){
		float *pt = &target.c[j][target_offset];
		float *ps = &source.c[j][source_offset];
		float *ps_end = ps + length;
		while(ps < ps_end){
			*pt = *ps;
			ps ++;
			pt ++;
		}
	}
}

// this[offset:] = source[:length]
void AudioBuffer::set_x(const AudioBuffer &source, int _offset, int _length, float volume)
{
	_length = min(_length, source.length);

	// relative to self
	int i0 = max(0, _offset);
	int i1 = min(_length + _offset, length);
	if (i1 <= i0)
		return;

	// set buffers
	if (volume == 1.0f){
		_buf_copy_samples_(*this, i0, source, i0 - _offset, i1 - i0);
	}else{
		_buf_copy_samples_scale_(*this, i0, source, i0 - _offset, i1 - i0, volume);
	}
	invalidate_peaks(Range(i0 + offset, i1 - i0));
}

// this[offset:] = source[:]
void AudioBuffer::set(const AudioBuffer &source, int _offset, float volume)
{
	set_x(source, _offset, source.length, volume);
}

void AudioBuffer::set_as_ref(const AudioBuffer &target, int _offset, int _length)
{
	clear();
	length = _length;
	offset = _offset + target.offset;
	channels = target.channels;
	for (int i=0; i<channels; i++)
		c[i].set_ref(target.c[i].sub(_offset, _length));
}

#if 0
void AudioBuffer::set_16bit(const void *b, int offset, int length)
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

void AudioBuffer::import(void *data, int _channels, SampleFormat format, int samples)
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

bool AudioBuffer::_export(void *data, int _channels, SampleFormat format, bool align32) const
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

bool AudioBuffer::exports(string &data, int _channels, SampleFormat format) const
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

void AudioBuffer::interleave(float *p, float volume) const
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

void AudioBuffer::deinterleave(float *p, int num_channels)
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

Range AudioBuffer::range() const
{
	return Range(offset, length);
}

Range AudioBuffer::range0() const
{
	return Range(0, length);
}

unsigned char inline _shrink_mean(unsigned char a, unsigned char b)
{
	return (unsigned char)(sqrt(((float)a * (float)a + (float)b * (float)b) / 2));
}

static bool _shrink_table_created = false;
static unsigned char _shrink_mean_table[256][256];

static void update_shrink_table()
{
	for (int a=0; a<256; a++)
		for (int b=0; b<256; b++)
			_shrink_mean_table[a][b] = _shrink_mean(a, b);
	_shrink_table_created = true;
}

#define shrink_max(a, b)	max((a), (b))
unsigned char inline shrink_mean(unsigned char a, unsigned char b)
{	return _shrink_mean_table[a][b];	}

void AudioBuffer::invalidate_peaks(const Range &_range)
{
	assert(range().covers(_range));

	if (peaks.num < PEAK_MAGIC_LEVEL4)
		return;

	int i0 = (_range.start() - offset) / PEAK_CHUNK_SIZE;
	int i1 = min((_range.end() - offset) / PEAK_CHUNK_SIZE + 1, peaks[PEAK_MAGIC_LEVEL4].num);

	for (int i=i0; i<i1; i++)
		peaks[PEAK_MAGIC_LEVEL4][i] = 255;
}

inline float fabsmax(float *p)
{
	float a = fabs(*p ++);
	float b = fabs(*p ++);
	float c = fabs(*p ++);
	float d = fabs(*p ++);
	float e = fabs(*p ++);
	float f = fabs(*p ++);
	float g = fabs(*p ++);
	float h = fabs(*p ++);
	return max(max(max(a, b), max(c, d)), max(max(e, f), max(g, h)));
}

void ensure_peak_size(AudioBuffer &buf, int level4, int n, bool set_invalid = false)
{
	if (buf.peaks.num < level4 + 4)
		buf.peaks.resize(level4 + 4);
	if (buf.peaks[level4].num < n){
		int n0 = buf.peaks[level4].num;
		buf.peaks[level4    ].resize(n);
		buf.peaks[level4 + 1].resize(n);
		buf.peaks[level4 + 2].resize(n);
		buf.peaks[level4 + 3].resize(n);
		if (set_invalid)
			for (int i=n0; i<n; i++)
				buf.peaks[level4][i] = buf.peaks[level4 + 1][i] = 255;
	}
}

void update_peaks_chunk(AudioBuffer &buf, int index)
{
	// first level
	int i0 = index * buf.PEAK_CHUNK_SIZE / buf.PEAK_FINEST_SIZE;
	int i1 = min(i0 + buf.PEAK_CHUNK_SIZE / buf.PEAK_FINEST_SIZE, buf.length / buf.PEAK_FINEST_SIZE);
	int n = i1 - i0;

	ensure_peak_size(buf, 0, i1);

	//msg_write(format("lvl0:  %d  %d     %d  %d", i0, n, buf.peaks[0].num, index));

	for (int j=0; j<buf.channels; j++){
		for (int i=i0; i<i1; i++)
			buf.peaks[j][i] = fabsmax(&buf.c[j][i * buf.PEAK_FINEST_SIZE]) * 254;
	}
	memcpy(&buf.peaks[2][i0], &buf.peaks[0][i0], n);
	memcpy(&buf.peaks[3][i0], &buf.peaks[1][i0], n);

	// medium levels
	int level4 = 0;
	while (n >= 2){
		level4 += 4;
		n = n / 2;
		i0 = i0 / 2;
		i1 = i0 + n;
		ensure_peak_size(buf, level4, i1);

		for (int i=i0; i<i1; i++)
			buf.peaks[level4    ][i] = shrink_max(buf.peaks[level4 - 4][i * 2], buf.peaks[level4 - 4][i * 2 + 1]);
		for (int i=i0; i<i1; i++)
			buf.peaks[level4 + 1][i] = shrink_max(buf.peaks[level4 - 3][i * 2], buf.peaks[level4 - 3][i * 2 + 1]);
		for (int i=i0; i<i1; i++)
			buf.peaks[level4 + 2][i] = shrink_mean(buf.peaks[level4 - 2][i * 2], buf.peaks[level4 - 2][i * 2 + 1]);
		for (int i=i0; i<i1; i++)
			buf.peaks[level4 + 3][i] = shrink_mean(buf.peaks[level4 - 1][i * 2], buf.peaks[level4 - 1][i * 2 + 1]);
	}

	//	msg_write(format("%d  %d  %d", level4 / 4, buf.peaks.num / 4 - 1, n));
	if (n == 0)
		return;

	// high levels
	for (int k=0; k<32; k++){
		if ((index & (1<<k)) == 0)
			break;

		level4 += 4;
		i0 = i0 / 2;
		ensure_peak_size(buf, level4, i0 + 1);

		buf.peaks[level4    ][i0] = shrink_max(buf.peaks[level4 - 4][i0 * 2], buf.peaks[level4 - 4][i0 * 2 + 1]);
		buf.peaks[level4 + 1][i0] = shrink_max(buf.peaks[level4 - 3][i0 * 2], buf.peaks[level4 - 3][i0 * 2 + 1]);
		buf.peaks[level4 + 2][i0] = shrink_mean(buf.peaks[level4 - 2][i0 * 2], buf.peaks[level4 - 2][i0 * 2 + 1]);
		buf.peaks[level4 + 3][i0] = shrink_mean(buf.peaks[level4 - 1][i0 * 2], buf.peaks[level4 - 1][i0 * 2 + 1]);
	}
}

void AudioBuffer::update_peaks()
{
	if (!_shrink_table_created)
		update_shrink_table();

	int n = length / PEAK_CHUNK_SIZE;

	tsunami->song->lock();
	for (int i=PEAK_OFFSET_EXP; i<PEAK_CHUNK_EXP; i++)
		ensure_peak_size(*this, (i - PEAK_OFFSET_EXP) * 4, length >> i, false);
	ensure_peak_size(*this, PEAK_MAGIC_LEVEL4, n, true);
	tsunami->song->unlock();

	Thread::cancelationPoint();

	for (int i=0; i<n; i++)
		if (peaks[PEAK_MAGIC_LEVEL4][i] == 255){
			while (!tsunami->song->try_lock()){
				Thread::cancelationPoint();
				hui::Sleep(0.01f);
			}
			update_peaks_chunk(*this, i);
			tsunami->song->unlock();

			Thread::cancelationPoint();
		}
}
