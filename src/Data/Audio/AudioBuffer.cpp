/*
 * AudioBuffer.cpp
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#include "AudioBuffer.h"
#include "../base.h"

#include "../../lib/math/math.h"
//#include <math.h>
#include <assert.h>



// peaks:
// ...


const int AudioBuffer::PEAK_CHUNK_EXP = 15;
const int AudioBuffer::PEAK_CHUNK_SIZE = 1<<PEAK_CHUNK_EXP;
const int AudioBuffer::PEAK_OFFSET_EXP = 3;
const int AudioBuffer::PEAK_FINEST_SIZE = 1<<PEAK_OFFSET_EXP;
const int AudioBuffer::PEAK_MAGIC_LEVEL4 = (PEAK_CHUNK_EXP - PEAK_OFFSET_EXP)*4;


AudioBuffer::AudioBuffer()
{
	offset = 0;
	length = 0;
	channels = 2;
}

AudioBuffer::AudioBuffer(int _length, int _channels)
{
	offset = 0;
	length = 0;
	channels = _channels;
	resize(_length);
}

// copy constructor
AudioBuffer::AudioBuffer(const AudioBuffer &b)
{
	offset = b.offset;
	length = b.length;
	channels = b.channels;
	for (int i=0; i<channels; i++)
		c[i] = b.c[i];
}

// move constructor
AudioBuffer::AudioBuffer(AudioBuffer &&b)
{
	offset = b.offset;
	length = b.length;
	channels = b.channels;
	for (int i=0; i<channels; i++)
		c[i] = std::move(b.c[i]);
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

void AudioBuffer::operator=(AudioBuffer &&b)
{
	offset = b.offset;
	length = b.length;
	channels = b.channels;
	for (int i=0; i<channels; i++)
		c[i] = std::move(b.c[i]);
	peaks = std::move(b.peaks);
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

void AudioBuffer::clear_x(int _channels)
{
	for (int i=0; i<channels; i++)
		c[i].clear();
	length = 0;
	peaks.clear();
	channels = _channels;
}

void AudioBuffer::_truncate_peaks(int _length)
{
	int level4 = 0;
	_length /= PEAK_FINEST_SIZE;
	while(level4 < peaks.num){
		for (int k=0; k<4; k++)
			peaks[level4 + k].resize(_length);
		level4 += 4;
		_length /= 2;
	}

}

void AudioBuffer::resize(int _length)
{
	if (is_ref())
		make_own();

	if (_length < length)
		_truncate_peaks(_length);
	for (int i=0; i<channels; i++)
		c[i].resize(_length);
	length = _length;
}

bool AudioBuffer::is_ref() const
{	return c[0].is_ref();	}

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
	for (int i=0; i<2; i++)
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

	if (volume == 0.0f){
		for (int i=0; i<channels; i++)
			memset(&c[i][0], 0, sizeof(float)*length);
		peaks.clear();
		return;
	}

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

void AudioBuffer::add(const AudioBuffer &source, int _offset, float volume, float panning)
{
	/*if (source.channels > channels)
		printf("AudioBuffer.add: channels\n");*/

	// relative to b
	int i0 = max(0, -_offset);
	int i1 = min(source.length, length - _offset);

	// add buffers
	if ((volume == 1.0f) and (panning == 0.0f)){
		for (int tc=0; tc<channels; tc++){
			int sc = min(tc, source.channels-1);
			for (int i=i0;i<i1;i++)
				c[tc][i + _offset] += source.c[sc][i];
		}
	}else{
		float f[2];
		if (channels == 2){
			// for source.channels = 1,2
			f[0] = volume * sin((panning + 1) / 4 * pi) * sqrt(2.0f);
			f[1] = volume * cos((panning + 1) / 4 * pi) * sqrt(2.0f);
		}else{
			f[0] = volume;
		}
		for (int tc=0; tc<channels; tc++){
			int sc = min(tc, source.channels-1);
			for (int i=i0;i<i1;i++)
				c[tc][i + _offset] += source.c[sc][i] * f[tc];
		}
	}
	invalidate_peaks(Range(i0 + _offset + offset, i1 - i0));
}

inline void _buf_copy_samples_(AudioBuffer &target, int target_offset, const AudioBuffer &source, int source_offset, int length)
{
	for (int tc=0; tc<target.channels; tc++){
		int sc = min(tc, source.channels-1);
		memcpy(&target.c[tc][target_offset], &source.c[sc][source_offset], sizeof(float) * length);
	}
}

inline void _buf_copy_samples_scale_(AudioBuffer &target, int target_offset, const AudioBuffer &source, int source_offset, int length, float volume)
{
	for (int tc=0; tc<target.channels; tc++){
		int sc = min(tc, source.channels-1);
		float *pt = &target.c[tc][target_offset];
		float *ps = &source.c[sc][source_offset];
		float *ps_end = ps + length;
		while(ps < ps_end){
			*pt = *ps * volume;
			ps ++;
			pt ++;
		}
	}
}

// this[offset:] = source[:length]
void AudioBuffer::set_x(const AudioBuffer &source, int _offset, int _length, float volume)
{
	/*if (source.channels > channels)
		printf("AudioBuffer.set_x: channels >\n");*/

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

void AudioBuffer::set_as_ref(const AudioBuffer &source, int _offset, int _length)
{
	clear();
	length = _length;
	offset = _offset + source.offset;
	channels = source.channels;
	for (int i=0; i<channels; i++)
		c[i].set_ref(source.c[i].sub(_offset, _length));
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
			if (format == SampleFormat::SAMPLE_FORMAT_8){
				c[0][i] = (float)cb[i*2    ] / 128.0f;
				if (channels > 1)
					c[1][i] = (float)cb[i*2 + 1] / 128.0f;
			}else if (format == SampleFormat::SAMPLE_FORMAT_16){
				c[0][i] = (float)sb[i*2    ] / 32768.0f;
				if (channels > 1)
					c[1][i] = (float)sb[i*2 + 1] / 32768.0f;
			}else if (format == SampleFormat::SAMPLE_FORMAT_16_BIGENDIAN){
				c[0][i] = (float)invert_16(sb[i*2    ]) / 32768.0f;
				if (channels > 1)
					c[1][i] = (float)invert_16(sb[i*2 + 1]) / 32768.0f;
			}else if (format == SampleFormat::SAMPLE_FORMAT_24){
				c[0][i] = import_24(*(int*)&cb[i*6    ]);
				if (channels > 1)
					c[1][i] = import_24(*(int*)&cb[i*6 + 3]);
			}else if (format == SampleFormat::SAMPLE_FORMAT_24_BIGENDIAN){
				c[0][i] = (float)invert_24(*(int*)&cb[i*6    ] >> 8) / 8388608.0f;
				if (channels > 1)
					c[1][i] = (float)invert_24(*(int*)&cb[i*6 + 3] >> 8) / 8388608.0f;
			}else if (format == SampleFormat::SAMPLE_FORMAT_32){
				c[0][i] = (float)ib[i*2  ] / 2147483648.0f;
				if (channels > 1)
					c[1][i] = (float)ib[i*2+1] / 2147483648.0f;
			}else if (format == SampleFormat::SAMPLE_FORMAT_32_FLOAT){
				c[0][i] = fb[i*2];
				if (channels > 1)
					c[1][i] = fb[i*2+1];
			}else
				throw string("BufferBox.import: unhandled format");
		}else{
			if (format == SampleFormat::SAMPLE_FORMAT_8){
				c[0][i] = (float)cb[i] / 128.0f;
			}else if (format == SampleFormat::SAMPLE_FORMAT_16){
				c[0][i] = (float)sb[i] / 32768.0f;
			}else if (format == SampleFormat::SAMPLE_FORMAT_16_BIGENDIAN){
				c[0][i] = (float)invert_16(sb[i]) / 32768.0f;
			}else if (format == SampleFormat::SAMPLE_FORMAT_24){
				c[0][i] = import_24(*(int*)&cb[i*3]);
			}else if (format == SampleFormat::SAMPLE_FORMAT_24_BIGENDIAN){
				c[0][i] = (float)invert_24(*(int*)&cb[i*3] >> 8) / 8388608.0f;
			}else if (format == SampleFormat::SAMPLE_FORMAT_32){
				c[0][i] = (float)ib[i] / 2147483648.0f;
			}else if (format == SampleFormat::SAMPLE_FORMAT_32_FLOAT){
				c[0][i] = fb[i];
			}else
				throw string("BufferBox.import: unhandled format");
			if (channels > 1)
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
	float*source[2];
	for (int ci=0; ci<_channels; ci++)
		source[ci] = &c[min(ci, channels-1)][0];

	if (format == SampleFormat::SAMPLE_FORMAT_16){
		short *sb = (short*)data;
		int d = align32 ? 2 : 1;
		for (int i=0;i<length;i++){
			for (int ci=0; ci<_channels; ci++){
				set_data_16(sb, *(source[ci]++));
				sb += d;
			}
		}
	}else if (format == SampleFormat::SAMPLE_FORMAT_24){
		char *sc = (char*)data;
		int d = align32 ? 4 : 3;
		for (int i=0;i<length;i++){
			for (int ci=0; ci<_channels; ci++){
				set_data_24((int*)sc, *(source[ci]++));
				sc += d;
			}
		}
	}else if (format == SampleFormat::SAMPLE_FORMAT_32_FLOAT){
		float *fc = (float*)data;
		for (int i=0;i<length;i++){
			for (int ci=0; ci<_channels; ci++){
				*(fc ++) = *(source[ci]++);
			}
		}
	}else{
		throw Exception("invalid export format");
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

// always outputting stereo... (for OutputStreams)
void AudioBuffer::interleave(float *p, float volume) const
{
	float *pr = &c[0][0];
	float *pl = &c[1][0];
	if (volume == 1.0f){
		if (channels == 2){
			for (int i=0; i<length; i++){
				*p ++ = _clamp_(*pr ++);
				*p ++ = _clamp_(*pl ++);
			}
		}else{
			for (int i=0; i<length; i++){
				float ff = _clamp_(*pr ++);
				*p ++ = ff;
				*p ++ = ff;
			}
		}
	}else{
		if (channels == 2){
			for (int i=0; i<length; i++){
				*p ++ = _clamp_((*pr ++) * volume);
				*p ++ = _clamp_((*pl ++) * volume);
			}
		}else{
			for (int i=0; i<length; i++){
				float ff = _clamp_((*pr ++) * volume);
				*p ++ = ff;
				*p ++ = ff;
			}
		}
	}
}

void AudioBuffer::deinterleave(float *p, int num_channels)
{
	float *pr = &c[0][0];
	float *pl = &c[1][0];
	if (num_channels == 1){
		if (channels == 2){
			for (int i=0; i<length; i++){
				*pr ++ = *p;
				*pl ++ = *p ++;
			}
		}else{
			memcpy(pr, p, length * sizeof(float));
		}
	}else if (num_channels == 2){
		if (channels == 2){
			for (int i=0; i<length; i++){
				*pr ++ = *p ++;
				*pl ++ = *p ++;
			}
		}else{
			for (int i=0; i<length; i++){
				*pr ++ = *p ++;
				p ++;
			}
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
	Range r = range() and _range;

	if (peaks.num < PEAK_MAGIC_LEVEL4)
		return;

	int i0 = (r.start() - offset) / PEAK_CHUNK_SIZE;
	int i1 = min((r.end() - offset) / PEAK_CHUNK_SIZE + 1, peaks[PEAK_MAGIC_LEVEL4].num);

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

void AudioBuffer::_ensure_peak_size(int level4, int n, bool set_invalid)
{
	if (peaks.num < level4 + 4)
		peaks.resize(level4 + 4);
	if (peaks[level4].num < n){
		int n0 = peaks[level4].num;
		for (int k=0; k<4; k++){
			peaks[level4 + k].resize(n);
			if (set_invalid)
				memset(&peaks[level4 + k][n0], 255, (n - n0));
		}
	}/*else if (peaks[level4].num < n){
		for (int k=0; k<4; k++)
			peaks[level4 + k].resize(n);
	}*/
}

bool AudioBuffer::_peaks_chunk_needs_update(int index)
{
	if (peaks.num <= PEAK_MAGIC_LEVEL4)
		return true;
	if (index == peaks[PEAK_MAGIC_LEVEL4].num)
		return true;
	return (peaks[PEAK_MAGIC_LEVEL4][index] == 255);
}

void AudioBuffer::_update_peaks_chunk(int index)
{
	// first level
	int i0 = index * (PEAK_CHUNK_SIZE / PEAK_FINEST_SIZE);
	int i1 = min(i0 + PEAK_CHUNK_SIZE / PEAK_FINEST_SIZE, length / PEAK_FINEST_SIZE);
	int n = i1 - i0;

	_ensure_peak_size(0, i1, true);

	//msg_write(format("lvl0:  %d  %d     %d  %d", i0, n, peaks[0].num, index));

	for (int j=0; j<channels; j++){
		for (int i=i0; i<i1; i++)
			peaks[j][i] = fabsmax(&c[j][i * PEAK_FINEST_SIZE]) * 254;
	}
	memcpy(&peaks[2][i0], &peaks[0][i0], n);
	memcpy(&peaks[3][i0], &peaks[1][i0], n);

	// medium levels
	int level4 = 0;
	while (n >= 2){
		level4 += 4;
		n = n / 2;
		i0 = i0 / 2;
		i1 = i0 + n;
		_ensure_peak_size(level4, i1);

		for (int i=i0; i<i1; i++){
			peaks[level4    ][i] = shrink_max(peaks[level4 - 4][i * 2], peaks[level4 - 4][i * 2 + 1]);
			peaks[level4 + 1][i] = shrink_max(peaks[level4 - 3][i * 2], peaks[level4 - 3][i * 2 + 1]);
			peaks[level4 + 2][i] = shrink_mean(peaks[level4 - 2][i * 2], peaks[level4 - 2][i * 2 + 1]);
			peaks[level4 + 3][i] = shrink_mean(peaks[level4 - 1][i * 2], peaks[level4 - 1][i * 2 + 1]);
		}
	}

	//	msg_write(format("%d  %d  %d", level4 / 4, peaks.num / 4 - 1, n));
	if (n == 0)
		return;

	// high levels
	for (int k=0; k<32; k++){
		if ((index & (1<<k)) == 0)
			break;

		if (peaks[level4].num <= (i0 | 1))
			break;

		level4 += 4;
		i0 = i0 / 2;

		_ensure_peak_size(level4, i0 + 1);

		peaks[level4    ][i0] = shrink_max(peaks[level4 - 4][i0 * 2], peaks[level4 - 4][i0 * 2 + 1]);
		peaks[level4 + 1][i0] = shrink_max(peaks[level4 - 3][i0 * 2], peaks[level4 - 3][i0 * 2 + 1]);
		peaks[level4 + 2][i0] = shrink_mean(peaks[level4 - 2][i0 * 2], peaks[level4 - 2][i0 * 2 + 1]);
		peaks[level4 + 3][i0] = shrink_mean(peaks[level4 - 1][i0 * 2], peaks[level4 - 1][i0 * 2 + 1]);
	}
}

int AudioBuffer::_update_peaks_prepare()
{
	if (!_shrink_table_created)
		update_shrink_table();

	int n = (int)ceil((float)length / (float)PEAK_CHUNK_SIZE);

	for (int i=PEAK_OFFSET_EXP; i<=PEAK_CHUNK_EXP; i++)
		_ensure_peak_size((i - PEAK_OFFSET_EXP) * 4, length >> i, true);
	//_ensure_peak_size(PEAK_MAGIC_LEVEL4, n, true);

	return n;
}
