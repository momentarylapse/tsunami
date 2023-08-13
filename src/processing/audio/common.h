/*
 * common.h
 *
 *  Created on: 13 Aug 2023
 *      Author: michi
 */

#ifndef SRC_PROCESSING_AUDIO_COMMON_H_
#define SRC_PROCESSING_AUDIO_COMMON_H_


#include "../../lib/base/base.h"
class complex;

float sum(const Array<float> &a);
float xmax(const Array<float> &a);
float sum_abs(const Array<complex> &z);
float max_abs(const Array<complex> &z);

enum class WindowFunction {
	RECTANGLE,
	HANN
};

void apply_window_function(Array<float> &data, WindowFunction wf);

#endif /* SRC_PROCESSING_AUDIO_COMMON_H_ */
