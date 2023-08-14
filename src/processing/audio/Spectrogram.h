/*
 * Spectrogram.h
 *
 *  Created on: 13 Aug 2023
 *      Author: michi
 */

#ifndef SRC_PROCESSING_AUDIO_SPECTROGRAM_H_
#define SRC_PROCESSING_AUDIO_SPECTROGRAM_H_

#include "common.h"

class AudioBuffer;

namespace Spectrogram {

int spectrum_size(int window_size);

Array<float> spectrogram(AudioBuffer &b, int step_size, int window_size, WindowFunction wf);

Array<float> log_spectrogram(AudioBuffer &b, float sample_rate, int step_size, float f_min, float f_max, int f_count, WindowFunction wf);

Array<float> to_db(const Array<float> &data, float db_range, float db_boost);

bytes quantize(const Array<float>& data);
float dequantize(unsigned char q);

}


#endif /* SRC_PROCESSING_AUDIO_SPECTROGRAM_H_ */
