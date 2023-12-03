/*
 * AudioBuffer.h
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#ifndef SRC_DATA_AUDIO_AUDIOBUFFER_H_
#define SRC_DATA_AUDIO_AUDIOBUFFER_H_

#include "../../lib/base/pointer.h"
#include "../Range.h"
#include <shared_mutex>


enum class SampleFormat;

class AudioBuffer {
public:
	AudioBuffer();
	AudioBuffer(const AudioBuffer &b);
	AudioBuffer(AudioBuffer &&b) noexcept;
	AudioBuffer(int length, int channels);
	~AudioBuffer() = default;

	void _cdecl __init__();
	void _cdecl __delete__();

	void operator=(const AudioBuffer &b);
	void operator=(AudioBuffer &&b) noexcept;
	void __assign__(const AudioBuffer &other) { *this = other; }

	int offset, length;
	int channels;
	Array<Array<float>> c;

	int uid;
	int version;

	std::shared_timed_mutex mtx;

	Range _cdecl range() const;
	Range _cdecl range0() const;

	void _cdecl clear();
	void _cdecl set_zero();
	void _cdecl scale(float factor);
	void _cdecl set_channels(int channels);
	void _cdecl resize(int length);
	bool _cdecl is_ref() const;
	void _cdecl make_own();
	void _cdecl mix_stereo(float volume, float panning = 0);
	void _cdecl swap_ref(AudioBuffer &b);
	void _cdecl swap_value(AudioBuffer &b);
	void _cdecl append(const AudioBuffer &b);
	void _cdecl set(const AudioBuffer &source, int target_offset, float volume = 1.0f);
	void _cdecl set_x(const AudioBuffer &source, int source_offset, int length, int target_offset, float volume = 1.0f);
	void _cdecl add(const AudioBuffer &source, int offset, float volume = 1.0f);
	void _cdecl set_as_ref(const AudioBuffer &source, int offset, int length);

	AudioBuffer ref(int start, int end);
	AudioBuffer cref(int start, int end) const;

	void _cdecl import(void *data, int channels, SampleFormat format, int samples);
	bool _cdecl _export(void *data, int channels, SampleFormat format, bool align32) const;
	bool _cdecl exports(bytes &data, int channels, SampleFormat format) const;
	void _cdecl interleave(float *p, float volume) const;
	void _cdecl deinterleave(const float *p, int num_channels);

	struct Compressed : Sharable<base::Empty> {
		bytes data;
		string codec;
	};
	shared<Compressed> compressed;
	bool has_compressed() const;
	void _data_was_changed();
};

#endif /* SRC_AUDIO_AUDIOBUFFER_H_ */
