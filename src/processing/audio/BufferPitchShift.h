/*
 * BufferPitchShift.h
 *
 *  Created on: 23.10.2022
 *      Author: michi
 */

#ifndef SRC_PROCESSING_AUDIO_BUFFERPITCHSHIFT_H_
#define SRC_PROCESSING_AUDIO_BUFFERPITCHSHIFT_H_

#include "../../lib/base/base.h"
class AudioBuffer;

namespace BufferInterpolator {
	enum class Method;
}

namespace BufferPitchShift {

void pitch_shift(AudioBuffer &buf, float factor);

AudioBuffer scale_and_pitch_shift(const AudioBuffer &buf, int new_size, BufferInterpolator::Method scaling_method, float pitch_factor);

}

#endif /* SRC_PROCESSING_AUDIO_BUFFERPITCHSHIFT_H_ */
