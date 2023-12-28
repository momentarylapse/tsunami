/*
 * AudioBuffer.cpp
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#include "AudioBuffer.h"
#include "../base.h"
#include "../../lib/math/math.h"
#include "../../lib/os/msg.h"
//#include <math.h>
#include <assert.h>


// mono/stereo:
//   always have 2 channels existing and correctly sized
//   but ignore data in mono


// peaks:
//   ALWAYS correct channels


const int MIN_CHANNELS = 2;
const int MAX_CHANNELS = 1024;

#define MEM_CHANNELS  max(channels, MIN_CHANNELS)

static int next_buffer_uid() {
	static int state = 0;
	return state ++;
	//rand();
}

static int next_buffer_version(int v) {
	return v + 0x10000;
}

AudioBuffer::AudioBuffer(int _length, int _channels) {
	uid = next_buffer_uid();
	version = 0;
	offset = 0;
	length = 0;
	channels = 0;
	set_channels(_channels);
	resize(_length);
}

AudioBuffer::AudioBuffer() : AudioBuffer(0, 2) {
}

// copy constructor
AudioBuffer::AudioBuffer(const AudioBuffer &b) {
	uid = next_buffer_uid();
	offset = b.offset;
	length = b.length;
	channels = b.channels;
	c = b.c;
	compressed = b.compressed;
	version = 0;
}

// move constructor
AudioBuffer::AudioBuffer(AudioBuffer &&b)  noexcept {
	offset = b.offset;
	length = b.length;
	channels = b.channels;
	c = std::move(b.c);
	b.length = 0;
	b.channels = 0;
	compressed = b.compressed;
#if 1
	uid = b.uid;
	version = b.version;
#else
	uid = next_buffer_uid();
	version = 0;
#endif
	b.uid = -1;
	b.version = -1;
}

void AudioBuffer::__init__() {
	new(this) AudioBuffer;
}

void AudioBuffer::__delete__() {
	this->AudioBuffer::~AudioBuffer();
}

void AudioBuffer::operator=(const AudioBuffer &b) {
	clear();
	if (uid < 0)
		uid = next_buffer_uid();
	offset = b.offset;
	length = b.length;
	channels = b.channels;
	c = b.c;
	_data_was_changed();
	compressed = b.compressed;
}

void AudioBuffer::operator=(AudioBuffer &&b) noexcept {
	clear();
	offset = b.offset;
	length = b.length;
	channels = b.channels;
	c = std::move(b.c);
	b.length = 0;
	b.channels = 0;
	compressed = b.compressed;
#if 1
	uid = b.uid;
	version = b.version;
#else
	uid = next_buffer_uid();
	version = next_buffer_version();
#endif
	b.uid = -1;
	b.version = -1;
}

void AudioBuffer::clear() {
	for (auto &cc: c)
		cc.clear();
	length = 0;
	_data_was_changed();
}

void AudioBuffer::set_zero() {
	for (auto &cc: c)
		memset(&cc[0], 0, sizeof(float) * length);
	_data_was_changed();
}

void AudioBuffer::scale(float scale) {
	for (auto &cc: c)
		for (float &f: cc)
			f *= scale;
	_data_was_changed();
}

void AudioBuffer::set_channels(int new_channels) {
	if (new_channels < 1 or new_channels >= MAX_CHANNELS)
		return;
	if (channels == new_channels)
		return;

	int c_num_before = c.num;
	channels = new_channels;
	c.resize(MEM_CHANNELS);

	// add new channels
	for (int i=c_num_before; i<c.num; i++)
		c[i].resize(length);

	_data_was_changed();
}

void AudioBuffer::resize(int _length) {
	if (_length < 0)
		_length = 0;

	if (is_ref())
		make_own();

	for (auto &cc: c)
		cc.resize(_length);
	length = _length;
	_data_was_changed();
}

bool AudioBuffer::is_ref() const
{	return c[0].is_ref();	}

void fa_make_own(Array<float> &a) {
	void *data = a.data;
	int num = a.num;
	a.forget();
	a.resize(num);
	memcpy(a.data, data, a.element_size * num);
}

void AudioBuffer::make_own() {
	if (is_ref()) {
		for (auto &cc: c)
			fa_make_own(cc);
	}
}

void AudioBuffer::swap_ref(AudioBuffer &b) {
	// buffer
	c.exchange(b.c);

	std::swap(length, b.length);
	std::swap(offset, b.offset);
	std::swap(channels, b.channels);

	// TODO
	std::swap(compressed, b.compressed);
	std::swap(version, b.version);
}

void AudioBuffer::append(const AudioBuffer &b) {
	int num0 = length;
	int v0 = version;
	resize(length + b.length);
	set(b, num0, 1.0f);

	// incremental change!
	version = v0 + 1;
}

void float_array_swap_values(Array<float> &a, Array<float> &b) {
	for (int i=0;i<a.num;i++) {
		std::swap(a[i], b[i]);
	}
}

// USAGE???
void AudioBuffer::swap_value(AudioBuffer &b) {
	assert(length == b.length and "BufferBox.swap_value");
	// buffer
	for (int i=0; i<min(channels, b.channels); i++)
		float_array_swap_values(c[i], b.c[i]);
	_data_was_changed();
	b._data_was_changed();
}

// mixing a mono track will scale by (1,1) in the center
// and by (0,sqrt(2)) on left/right (OVERDRIVE)!
void AudioBuffer::mix_stereo(float volume, float panning) {
	if ((volume == 1.0f) and (panning == 0) and (channels == 2))
		return;
	if (channels > 2)
		return;


	float fl, fr;
	if (channels == 2) {
		fl = volume;
		fr = volume;
		if (panning > 0)
			fl *= (1 - panning);
		else
			fr *= (1 + panning);
	} else if (channels == 1) {
		fl = volume * cos((panning + 1) / 4 * pi) * sqrt(2.0f);
		fr = volume * sin((panning + 1) / 4 * pi) * sqrt(2.0f);
	}

	// scale
	if (channels == 2) {
		// stereo -> stereo
		for (int i=0;i<length;i++) {
			c[0][i] *= fl;
			c[1][i] *= fr;
		}
	} else if (channels == 1) {
		set_channels(2);
		// mono -> stereo
		for (int i=0;i<length;i++) {
			float v = c[0][i];
			c[0][i] *= fl;
			c[1][i] = v * fr;
		}

	}

	_data_was_changed();
}


void AudioBuffer::add(const AudioBuffer &source, int _offset, float volume) {

	// relative to b
	int i0 = max(0, -_offset);
	int i1 = min(source.length, length - _offset);

	// add buffers
	if (volume == 1.0f) {
		for (int tc=0; tc<channels; tc++) {
			int sc = min(tc, source.channels-1);
			for (int i=i0;i<i1;i++)
				c[tc][i + _offset] += source.c[sc][i];
		}
	} else {
		for (int tc=0; tc<channels; tc++) {
			int sc = min(tc, source.channels-1);
			for (int i=i0;i<i1;i++)
				c[tc][i + _offset] += source.c[sc][i] * volume;
		}
	}
	//invalidate_peaks(Range(i0 + _offset + offset, i1 - i0));
	_data_was_changed();
}

// offsets must be valid!
inline void _buf_copy_samples_(AudioBuffer &target, int target_offset, const AudioBuffer &source, int source_offset, int length) {
	for (int tc=0; tc<target.channels; tc++) {
		int sc = min(tc, source.channels-1);
		memcpy(&target.c[tc][target_offset], &source.c[sc][source_offset], sizeof(float) * length);
	}
}

// @TESTME skipping bug...?
inline void _buf_copy_samples_scale_(AudioBuffer &target, int target_offset, const AudioBuffer &source, int source_offset, int length, float volume) {
	for (int tc=0; tc<target.channels; tc++) {
		int sc = min(tc, source.channels-1);
		float *pt = &target.c[tc][target_offset];
		float *ps = &source.c[sc][source_offset];
		float *ps_end = ps + length;
		while (ps < ps_end) {
			*pt = *ps * volume;
			ps ++;
			pt ++;
		}
	}
}

// this[target_offset:] = source[source_offset:length]
void AudioBuffer::set_x(const AudioBuffer &source, int source_offset, int _length, int target_offset, float volume) {
	/*if (source.channels > channels)
		printf("AudioBuffer.set_x: channels >\n");*/

	if (target_offset < 0) {
		source_offset += -target_offset;
		_length -= -target_offset;
		target_offset = 0;
	}
	if (source_offset < 0) {
		target_offset += -source_offset;
		_length -= -source_offset;
		source_offset = 0;
	}
	_length = min(min(_length, source.length - source_offset), length - target_offset);

	if (_length <= 0)
		return;

	// set buffers
	if (volume == 1.0f) {
		_buf_copy_samples_(*this, target_offset, source, source_offset, _length);
	} else {
		_buf_copy_samples_scale_(*this, target_offset, source, source_offset, _length, volume);
	}
	//invalidate_peaks(Range(target_offset, _length));
	_data_was_changed();
}

// this[offset:] = source[:]
void AudioBuffer::set(const AudioBuffer &source, int target_offset, float volume) {
	set_x(source, 0, source.length, target_offset, volume);
}

// this[:] = source[offset:offset+length] ???
void AudioBuffer::set_as_ref(const AudioBuffer &source, int _offset, int _length) {
	clear();
	offset = _offset + source.offset;
	channels = source.channels;
	c.resize(MEM_CHANNELS);
	for (int i=0; i<MEM_CHANNELS; i++)
		c[i].set_ref(source.c[i].sub_ref(_offset, _offset+_length));
	length = c[0].num; // might be smaller than _length
}

AudioBuffer AudioBuffer::ref(int start, int end) {
	if (end == DynamicArray::MAGIC_END_INDEX)
		end = length;
	AudioBuffer r;
	if (start < 0)
		start += length;
	if (start < 0)
		start = 0;
	if (end < 0)
		end = length;
	r.set_as_ref(*this, start, end - start);
	return r;
}

AudioBuffer AudioBuffer::cref(int start, int end) const {
	return const_cast<AudioBuffer*>(this)->ref(start, end);
}

inline int invert_16(int i) {
	return ((i & 0xff) << 8) + ((i & 0xff00) >> 8);
}

inline int invert_24(int i) {
	unsigned int ui = i;
	unsigned int ext = (i < 0) ? 0xff000000 : 0;
	ui = ((ui & 0xff) << 16) | (ui & 0xff00) | ((ui & 0xff0000) >> 16) | ext;
	return (signed)ui;
}

inline int invert_32(int i) {
	return ((i & 0xff) << 24) + ((i & 0xff00) << 8) + ((i & 0xff0000) >> 8) + ((i & 0xff000000) >> 24);
}

inline float import_24(int i) {
	if ((i & 0x00800000) != 0)
		return (float)((i & 0x00ffffff) - 0x01000000) / 8388608.0f;
	return (float)(i & 0x00ffffff) / 8388608.0f;
}

void AudioBuffer::import(void *data, int _channels, SampleFormat format, int samples) {
	char *cb = (char*)data;
	short *sb = (short*)data;
	int *ib = (int*)data;
	float *fb = (float*)data;

	for (int i=0;i<samples;i++) {
		if (_channels == 2) {
			if (format == SampleFormat::INT_8) {
				c[0][i] = (float)cb[i*2    ] / 128.0f;
				if (channels > 1)
					c[1][i] = (float)cb[i*2 + 1] / 128.0f;
			} else if (format == SampleFormat::INT_16) {
				c[0][i] = (float)sb[i*2    ] / 32768.0f;
				if (channels > 1)
					c[1][i] = (float)sb[i*2 + 1] / 32768.0f;
			} else if (format == SampleFormat::INT_16_BIGENDIAN) {
				c[0][i] = (float)invert_16(sb[i*2    ]) / 32768.0f;
				if (channels > 1)
					c[1][i] = (float)invert_16(sb[i*2 + 1]) / 32768.0f;
			} else if (format == SampleFormat::INT_24) {
				c[0][i] = import_24(*(int*)&cb[i*6    ]);
				if (channels > 1)
					c[1][i] = import_24(*(int*)&cb[i*6 + 3]);
			} else if (format == SampleFormat::INT_24_BIGENDIAN) {
				c[0][i] = (float)invert_24(*(int*)&cb[i*6    ] >> 8) / 8388608.0f;
				if (channels > 1)
					c[1][i] = (float)invert_24(*(int*)&cb[i*6 + 3] >> 8) / 8388608.0f;
			} else if (format == SampleFormat::INT_32) {
				c[0][i] = (float)ib[i*2  ] / 2147483648.0f;
				if (channels > 1)
					c[1][i] = (float)ib[i*2+1] / 2147483648.0f;
			} else if (format == SampleFormat::FLOAT_32) {
				c[0][i] = fb[i*2];
				if (channels > 1)
					c[1][i] = fb[i*2+1];
			} else
				throw string("BufferBox.import: unhandled format");
		} else {
			if (format == SampleFormat::INT_8) {
				c[0][i] = (float)cb[i] / 128.0f;
			} else if (format == SampleFormat::INT_16) {
				c[0][i] = (float)sb[i] / 32768.0f;
			} else if (format == SampleFormat::INT_16_BIGENDIAN) {
				c[0][i] = (float)invert_16(sb[i]) / 32768.0f;
			} else if (format == SampleFormat::INT_24) {
				c[0][i] = import_24(*(int*)&cb[i*3]);
			} else if (format == SampleFormat::INT_24_BIGENDIAN) {
				c[0][i] = (float)invert_24(*(int*)&cb[i*3] >> 8) / 8388608.0f;
			} else if (format == SampleFormat::INT_32) {
				c[0][i] = (float)ib[i] / 2147483648.0f;
			} else if (format == SampleFormat::FLOAT_32) {
				c[0][i] = fb[i];
			} else
				throw string("BufferBox.import: unhandled format");
			if (channels > 1)
				c[1][i] = c[0][i];
		}
	}
	_data_was_changed();
}



#define VAL_MAX_16		32766
#define VAL_ALERT_16	32770
#define VAL_MAX_24		8388606
#define VAL_ALERT_24	8388610

static bool wtb_overflow;

inline int set_data(float value, float scale, int clamp, int warn_thresh) {
	int value_int = (int)(value * scale);
	if (value_int > clamp) {
		if (value_int > warn_thresh)
			wtb_overflow = true;
		value_int = clamp;
	} else if (value_int < - clamp) {
		if (value_int < -warn_thresh)
			wtb_overflow = true;
		return -clamp;
	}
	return value_int;
}

inline void set_data_16(short *data, float value) {
	*data = set_data(value, 32768.0f, VAL_MAX_16, VAL_ALERT_16);
}

inline void set_data_24(int *data, float value) {
	*data = set_data(value, 8388608.0f, VAL_MAX_24, VAL_ALERT_24) & 0x00ffffff;
}

inline void set_data_32(int *data, float value) {
	*data = set_data(value, 2147483648.0f, 0x7fffffff, 0x80000000);
}

bool AudioBuffer::_export(void *data, int _channels, SampleFormat format, bool align32) const {
	wtb_overflow = false;
	float*source[2];
	for (int ci=0; ci<_channels; ci++)
		source[ci] = &c[min(ci, channels-1)][0];

	if (format == SampleFormat::INT_16) {
		short *sb = (short*)data;
		int d = align32 ? 2 : 1;
		for (int i=0;i<length;i++) {
			for (int ci=0; ci<_channels; ci++) {
				set_data_16(sb, *(source[ci]++));
				sb += d;
			}
		}
	} else if (format == SampleFormat::INT_24) {
		char *sc = (char*)data;
		int d = align32 ? 4 : 3;
		for (int i=0;i<length;i++) {
			for (int ci=0; ci<_channels; ci++) {
				set_data_24((int*)sc, *(source[ci]++));
				sc += d;
			}
		}
	} else if (format == SampleFormat::INT_32) {
		int *sc = (int*)data;
		for (int i=0;i<length;i++) {
			for (int ci=0; ci<_channels; ci++) {
				set_data_32(sc, *(source[ci]++));
				sc ++;
			}
		}
	} else if (format == SampleFormat::FLOAT_32) {
		float *fc = (float*)data;
		for (int i=0;i<length;i++) {
			for (int ci=0; ci<_channels; ci++) {
				*(fc ++) = *(source[ci]++);
			}
		}
	} else {
		throw Exception("invalid export format");
	}

	return !wtb_overflow;
}

bool AudioBuffer::exports(bytes &data, int _channels, SampleFormat format) const {
	data.resize(length * _channels * (format_get_bits(format) / 8));
	return _export(data.data, _channels, format, false);
}

inline float _clamp_(float f) {
	if (f < -0.999f)
		return -0.999f;
	if (f > 0.999f)
		return 0.999f;
	return f;
}

// always outputting stereo... (for OutputStreams)
// TODO add channels parameter!
void AudioBuffer::interleave(float *p, float volume) const {
	float *pl = &c[0][0];
	float *pr = &c[1][0];
	if (volume == 1.0f) {
		if (channels == 2) {
			for (int i=0; i<length; i++) {
				*p ++ = _clamp_(*pl ++);
				*p ++ = _clamp_(*pr ++);
			}
		} else {
			for (int i=0; i<length; i++) {
				float ff = _clamp_(*pl ++);
				*p ++ = ff;
				*p ++ = ff;
			}
		}
	} else {
		if (channels == 2) {
			for (int i=0; i<length; i++) {
				*p ++ = _clamp_((*pl ++) * volume);
				*p ++ = _clamp_((*pr ++) * volume);
			}
		} else {
			for (int i=0; i<length; i++) {
				float ff = _clamp_((*pl ++) * volume);
				*p ++ = ff;
				*p ++ = ff;
			}
		}
	}
}

void AudioBuffer::deinterleave(const float *p, int source_channels) {
	float *pl = &c[0][0];
	float *pr = &c[1][0];
	if (source_channels == 1) {
		// mono -> mono
		memcpy(pl, p, length * sizeof(float));

		if (channels == 2) {
			// mono -> stereo
			memcpy(pr, p, length * sizeof(float));
		}
	} else if (source_channels == 2) {
		if (channels >= 2) {
			// stereo -> stereo
			for (int i=0; i<length; i++) {
				*pl ++ = *p ++;
				*pr ++ = *p ++;
			}
		} else if (channels == 1) {
			// stereo -> mono
			for (int i=0; i<length; i++) {
				*pl ++ = *p ++;
				p ++;
			}
		}
	} else {
		// complex...
		int nc = min(channels, source_channels);
		for (int i=0; i<length; i++) {
			for (int ci=0; ci<nc; ci++)
				c[ci][i] = *p ++;
			// too many input channels?
			for (int ci=nc; ci<source_channels; ci++)
				p ++;
		}

	}
	_data_was_changed();
}

Range AudioBuffer::range() const {
	return Range(offset, length);
}

Range AudioBuffer::range0() const {
	return Range(0, length);
}

bool AudioBuffer::has_compressed() const {
	return compressed.get();
}

void AudioBuffer::_data_was_changed() {
	// invalidate compressed
	compressed = nullptr;

	version = next_buffer_version(version);
}
