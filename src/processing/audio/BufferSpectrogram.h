/*
 * BufferSpectrogram.h
 *
 *  Created on: 13 Aug 2023
 *      Author: michi
 */

#ifndef SRC_PROCESSING_AUDIO_BUFFERSPECTROGRAM_H_
#define SRC_PROCESSING_AUDIO_BUFFERSPECTROGRAM_H_


#include "../../lib/base/base.h"
class AudioBuffer;
class complex;

namespace BufferSpectrogram {

enum class WindowFunction {
	RECTANGLE,
	HANN
};


Array<float> spectrogram(AudioBuffer &b, int step_size, int window_size, WindowFunction wf);
bytes quantized_spectrogram(AudioBuffer &b, float sample_rate, int step_size, float f_min, float f_max, int f_count, WindowFunction wf);

}


#endif /* SRC_PROCESSING_AUDIO_BUFFERSPECTROGRAM_H_ */
