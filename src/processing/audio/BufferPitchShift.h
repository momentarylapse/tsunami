/*
 * BufferPitchShift.h
 *
 *  Created on: 23.10.2022
 *      Author: michi
 */

#ifndef SRC_PROCESSING_AUDIO_BUFFERPITCHSHIFT_H_
#define SRC_PROCESSING_AUDIO_BUFFERPITCHSHIFT_H_

#include "../../lib/base/base.h"
#include "../../data/audio/RingBuffer.h"
#include "BufferInterpolator.h"

namespace tsunami {

class AudioBuffer;

namespace BufferInterpolator {
	enum class Method;
}

namespace BufferPitchShift {

void pitch_shift(AudioBuffer &buf, float factor);

AudioBuffer scale_and_pitch_shift(const AudioBuffer &buf, int new_size, BufferInterpolator::Method scaling_method, float pitch_factor);


struct DummyStretchOperator {
	void reset(float length_factor);

	float factor = 1.0f;

	int64 offset_in = 0;
	int64 produced = 0;

	RingBuffer ring_buffer{65536};
	AudioBuffer lin_buffer;

	AudioBuffer process(const AudioBuffer &buf);
};

struct Operator {
	Operator();
	void reset(float length_factor, BufferInterpolator::Method scaling_method, float pitch_factor);

	DummyStretchOperator op_stretch;
	RingBuffer buf_between;
	BufferInterpolator::Operator op_inter;

	int64 consumed = 0;
	int64 produced = 0;

	AudioBuffer process(const AudioBuffer &buf);
};
}

}

#endif /* SRC_PROCESSING_AUDIO_BUFFERPITCHSHIFT_H_ */
