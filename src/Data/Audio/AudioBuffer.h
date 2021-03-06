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


#define NUM_PEAK_LEVELS		24
#define PEAK_FACTOR			2

enum class SampleFormat;

class AudioBuffer {
public:
	AudioBuffer();
	AudioBuffer(const AudioBuffer &b);
	AudioBuffer(AudioBuffer &&b);
	AudioBuffer(int length, int channels);
	~AudioBuffer();

	void _cdecl __init__();
	void _cdecl __delete__();

	void operator=(const AudioBuffer &b);
	void operator=(AudioBuffer &&b);
	void __assign__(const AudioBuffer &other) { *this = other; }

	int offset, length;
	int channels;
	Array<Array<float>> c;

	Array<bytes> peaks;

	std::shared_timed_mutex mtx;

	Range _cdecl range() const;
	Range _cdecl range0() const;

	void _cdecl clear();
	void _cdecl set_zero();
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

	AudioBuffer _cdecl ref(int start, int end);

	void _cdecl import(void *data, int channels, SampleFormat format, int samples);
	bool _cdecl _export(void *data, int channels, SampleFormat format, bool align32) const;
	bool _cdecl exports(bytes &data, int channels, SampleFormat format) const;
	void _cdecl interleave(float *p, float volume) const;
	void _cdecl deinterleave(const float *p, int num_channels);


	static const int PEAK_CHUNK_EXP;
	static const int PEAK_CHUNK_SIZE;
	static const int PEAK_OFFSET_EXP;
	static const int PEAK_FINEST_SIZE;
	static const int PEAK_MAGIC_LEVEL2;

	void _cdecl invalidate_peaks(const Range &r);

	void _ensure_peak_size(int level4, int n, bool set_invalid = false);
	int _update_peaks_prepare();
	void _update_peaks_chunk(int index);
	bool _peaks_chunk_needs_update(int index);
	void _truncate_peaks(int length);


	struct Compressed : Sharable<Empty> {
		bytes data;
		string codec;
	};
	shared<Compressed> compressed;
	bool has_compressed() const;
	void invalidate_compressed();
};

#endif /* SRC_AUDIO_AUDIOBUFFER_H_ */
