/*
 * base.cpp
 *
 *  Created on: 25.07.2018
 *      Author: michi
 */

#include "base.h"
#include "../lib/hui/hui.h"
#include <math.h>

const Array<int> POSSIBLE_SAMPLE_RATES = {
		22050,
		DEFAULT_SAMPLE_RATE,
		48000,
		96000
};

float amplitude2db(float amp) {
	return log10(amp) * 20.0f;
}

float db2amplitude(float db) {
	return pow(10, db * 0.05f);
}

float power2db(float power) {
	return log10(power) * 10.0f;
}

float db2power(float db) {
	return pow(10, db * 0.1f);
}


SampleFormat format_for_bits(int bits, bool prefer_float, bool big_endian) {
	if (bits == 8) {
		return SampleFormat::INT_8;
	} else if (bits == 16) {
		if (big_endian)
			return SampleFormat::INT_16_BIGENDIAN;
		return SampleFormat::INT_16;
	} else if (bits == 24) {
		if (big_endian)
			return SampleFormat::INT_24_BIGENDIAN;
		return SampleFormat::INT_24;
	} else if (bits == 32) {
		if (prefer_float)
			return SampleFormat::FLOAT_32;
		else if (big_endian)
			return SampleFormat::INT_32_BIGENDIAN;
		else
			return SampleFormat::INT_32;
	}
	return SampleFormat::UNKNOWN;
}

int format_get_bits(SampleFormat format) {
	if (format == SampleFormat::INT_8)
		return 8;
	if ((format == SampleFormat::INT_16) or (format == SampleFormat::INT_16_BIGENDIAN))
		return 16;
	if ((format == SampleFormat::INT_24) or (format == SampleFormat::INT_24_BIGENDIAN))
		return 24;
	if ((format == SampleFormat::INT_32) or (format == SampleFormat::INT_32_BIGENDIAN) or (format == SampleFormat::FLOAT_32))
		return 32;
	return 0;
}

string format_name(SampleFormat format) {
	if (format == SampleFormat::INT_8)
		return "8 bit";
	if (format == SampleFormat::INT_16)
		return "16 bit";
	if (format == SampleFormat::INT_16_BIGENDIAN)
		return "16 bit BigEndian";
	if (format == SampleFormat::INT_24)
		return "24 bit";
	if (format == SampleFormat::INT_24_BIGENDIAN)
		return "24 bit BigEndian";
	if (format == SampleFormat::INT_32)
		return "32 bit";
	if (format == SampleFormat::INT_32_BIGENDIAN)
		return "32 bit BigEndian";
	if (format == SampleFormat::FLOAT_32)
		return "32 bit float";
	return "???";
}

string format_code(SampleFormat format) {
	if (format == SampleFormat::INT_8)
		return "i8";
	if (format == SampleFormat::INT_16)
		return "i16";
	if (format == SampleFormat::INT_16_BIGENDIAN)
		return "i16be";
	if (format == SampleFormat::INT_24)
		return "i24";
	if (format == SampleFormat::INT_24_BIGENDIAN)
		return "i24be";
	if (format == SampleFormat::INT_32)
		return "i32";
	if (format == SampleFormat::INT_32_BIGENDIAN)
		return "i32be";
	if (format == SampleFormat::FLOAT_32)
		return "f32";
	return "???";
}

SampleFormat format_from_code(const string &code) {
	if (code == "i8")
		return SampleFormat::INT_8;
	if (code == "i16")
		return SampleFormat::INT_16;
	if (code == "i16be")
		return SampleFormat::INT_16_BIGENDIAN;
	if (code == "i24")
		return SampleFormat::INT_24;
	if (code == "i24be")
		return SampleFormat::INT_24_BIGENDIAN;
	if (code == "i32")
		return SampleFormat::INT_32;
	if (code == "i32be")
		return SampleFormat::INT_32_BIGENDIAN;
	if (code == "f32")
		return SampleFormat::FLOAT_32;
	return SampleFormat::UNKNOWN;
}

string signal_type_name(SignalType type) {
	if (type == SignalType::AUDIO)
		return _("Audio");
	if (type == SignalType::MIDI)
		return _("Midi");
	if (type == SignalType::BEATS)
		return _("Metronome");
	if (type == SignalType::AUDIO_MONO)
		return _("Audio (mono)");
	if (type == SignalType::AUDIO_STEREO)
		return _("Audio (stereo)");
	return "???";
}




