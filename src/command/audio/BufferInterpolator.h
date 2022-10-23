/*
 * BufferInterpolator.h
 *
 *  Created on: 21.04.2017
 *      Author: michi
 */

#ifndef SRC_COMMAND_AUDIO_BUFFERINTERPOLATOR_H_
#define SRC_COMMAND_AUDIO_BUFFERINTERPOLATOR_H_

#include "../../lib/base/base.h"
class AudioBuffer;

namespace BufferInterpolator {

enum class Method{
	LINEAR,
	CUBIC,
	SINC,
	FOURIER,
};

double cubic_inter(double A, double B, double C, double D, float t);

void interpolate_channel_linear(Array<float> &in, Array<float> &out);
void interpolate_channel_cubic(Array<float> &in, Array<float> &out);
void interpolate_channel_fourier(Array<float> &in, Array<float> &out);

void interpolate_channel(Array<float> &in, Array<float> &out, Method method);
void interpolate(AudioBuffer &in, AudioBuffer &out, Method method);

}

#endif /* SRC_COMMAND_AUDIO_BUFFERINTERPOLATOR_H_ */
