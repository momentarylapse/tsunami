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

namespace tsunami {

enum class SampleFormat;

class AudioBuffer {
public:
	AudioBuffer();
	AudioBuffer(const AudioBuffer &b);
	AudioBuffer(AudioBuffer &&b) noexcept;
	AudioBuffer(int length, int channels);
	~AudioBuffer() = default;

	void __init__();
	void __delete__();

	void operator=(const AudioBuffer &b);
	void operator=(AudioBuffer &&b) noexcept;
	void __assign__(const AudioBuffer &other) { *this = other; }

	int offset, length;
	int channels;
	Array<Array<float>> c;

	int uid;
	int version;

	std::shared_timed_mutex mtx;

	Range range() const;
	Range range0() const;

	void clear();
	void set_zero();
	void scale(float factor);
	void set_channels(int channels);
	void resize(int length);
	bool is_ref() const;
	void make_own();
	void mix_stereo(float volume, float panning = 0);
	void swap_ref(AudioBuffer &b);
	void swap_value(AudioBuffer &b);
	void append(const AudioBuffer &b);
	void set(const AudioBuffer &source, int target_offset, float volume = 1.0f);
	void set_x(const AudioBuffer &source, int source_offset, int length, int target_offset, float volume = 1.0f);
	void add(const AudioBuffer &source, int offset, float volume = 1.0f);
	void set_as_ref(const AudioBuffer &source, int offset, int length);

	AudioBuffer ref(int start, int end);
	AudioBuffer cref(int start, int end) const;

	void import(void *data, int channels, SampleFormat format, int samples);
	bool _export(void *data, int channels, SampleFormat format, bool align32) const;
	bool exports(bytes &data, int channels, SampleFormat format) const;
	void interleave(float *p, float volume) const;
	void deinterleave(const float *p, int num_channels);

	struct Compressed : Sharable<base::Empty> {
		bytes data;
		string codec;
	};
	shared<Compressed> compressed;
	bool has_compressed() const;
	void _data_was_changed();
};

}

#endif /* SRC_AUDIO_AUDIOBUFFER_H_ */
