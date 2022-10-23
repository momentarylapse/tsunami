/*
 * BufferPitchShift.h
 *
 *  Created on: 23.10.2022
 *      Author: michi
 */

#ifndef SRC_DATA_AUDIO_BUFFERPITCHSHIFT_H_
#define SRC_DATA_AUDIO_BUFFERPITCHSHIFT_H_

#include "../../lib/base/base.h"
class AudioBuffer;

namespace BufferPitchShift {

void pitch_shift(AudioBuffer &buf, float factor);

}

#endif /* SRC_DATA_AUDIO_BUFFERPITCHSHIFT_H_ */
