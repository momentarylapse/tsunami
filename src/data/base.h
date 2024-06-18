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
float power2db(float power);
float db2power(float db);

const int DEFAULT_SAMPLE_RATE = 44100;

extern const Array<int> POSSIBLE_SAMPLE_RATES;


enum class SampleFormat {
	Unknown,
	Int8,
	Int16,
	Int16Bigendian,
	Int24,
	Int24Bigendian,
	Int32,
	Int32Bigendian,
	Float32,
	Count
};


SampleFormat format_for_bits(int bits, bool prefer_float, bool big_endian=false);
int format_get_bits(SampleFormat);
string format_name(SampleFormat format);
string format_code(SampleFormat format);
SampleFormat format_from_code(const string &code);


enum class SignalType {
	Audio,
	Beats,
	Midi,

	// special
	AudioMono,
	AudioStereo,
	Group
};

string signal_type_name(SignalType type);

#endif /* SRC_DATA_BASE_H_ */
