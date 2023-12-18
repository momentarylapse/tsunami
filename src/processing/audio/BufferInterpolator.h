/*
 * BufferInterpolator.h
 *
 *  Created on: 21.04.2017
 *      Author: michi
 */

#ifndef SRC_PROCESSING_AUDIO_BUFFERINTERPOLATOR_H_
#define SRC_PROCESSING_AUDIO_BUFFERINTERPOLATOR_H_

#include "../../lib/base/base.h"
class AudioBuffer;
//class RingBuffer;

namespace BufferInterpolator {

enum class Method {
	LINEAR,
	CUBIC,
	SINC,
	FOURIER,
};

double cubic_inter(double A, double B, double C, double D, float t);

void interpolate_channel_linear(const Array<float> &in, Array<float> &out);
void interpolate_channel_cubic(const Array<float> &in, Array<float> &out);
void interpolate_channel_fourier(const Array<float> &in, Array<float> &out);

void interpolate_channel(const Array<float> &in, Array<float> &out, Method method);
void interpolate(const AudioBuffer &in, AudioBuffer &out, Method method);

struct Operator {
	Method method = Method::LINEAR;
	float factor = 1.0f;
	int64 consumed = 0;
	int64 produced = 0;

	void reset(float factor);
	AudioBuffer process(const AudioBuffer &buf);
};

}

#endif /* SRC_PROCESSING_AUDIO_BUFFERINTERPOLATOR_H_ */
