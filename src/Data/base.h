/*
 * base.h
 *
 *  Created on: 25.07.2018
 *      Author: michi
 */

#ifndef SRC_DATA_BASE_H_
#define SRC_DATA_BASE_H_

#include "../lib/base/base.h"


float amplitude2db(float amp);
float db2amplitude(float db);

const int DEFAULT_SAMPLE_RATE = 44100;


enum class SampleFormat {
	SAMPLE_FORMAT_UNKNOWN,
	SAMPLE_FORMAT_8,
	SAMPLE_FORMAT_16,
	SAMPLE_FORMAT_16_BIGENDIAN,
	SAMPLE_FORMAT_24,
	SAMPLE_FORMAT_24_BIGENDIAN,
	SAMPLE_FORMAT_32,
	SAMPLE_FORMAT_32_BIGENDIAN,
	SAMPLE_FORMAT_32_FLOAT,
	NUM_SAMPLE_FORMATS
};


SampleFormat format_for_bits(int bits);
int format_get_bits(SampleFormat);
string format_name(SampleFormat format);


enum class SignalType {
	AUDIO,
	BEATS,
	MIDI,

	// special
	AUDIO_MONO,
	AUDIO_STEREO,
	GROUP
};

string signal_type_name(SignalType type);

#endif /* SRC_DATA_BASE_H_ */
