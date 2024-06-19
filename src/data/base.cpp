/*
 * base.cpp
 *
 *  Created on: 25.07.2018
 *      Author: michi
 */

#include "base.h"
#include "../lib/hui/hui.h"
#include <math.h>


namespace tsunami {

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
		return SampleFormat::Int8;
	} else if (bits == 16) {
		if (big_endian)
			return SampleFormat::Int16Bigendian;
		return SampleFormat::Int16;
	} else if (bits == 24) {
		if (big_endian)
			return SampleFormat::Int24Bigendian;
		return SampleFormat::Int24;
	} else if (bits == 32) {
		if (prefer_float)
			return SampleFormat::Float32;
		else if (big_endian)
			return SampleFormat::Int32Bigendian;
		else
			return SampleFormat::Int32;
	}
	return SampleFormat::Unknown;
}

int format_get_bits(SampleFormat format) {
	if (format == SampleFormat::Int8)
		return 8;
	if ((format == SampleFormat::Int16) or (format == SampleFormat::Int16Bigendian))
		return 16;
	if ((format == SampleFormat::Int24) or (format == SampleFormat::Int24Bigendian))
		return 24;
	if ((format == SampleFormat::Int32) or (format == SampleFormat::Int32Bigendian) or (format == SampleFormat::Float32))
		return 32;
	return 0;
}

string format_name(SampleFormat format) {
	if (format == SampleFormat::Int8)
		return "8 bit";
	if (format == SampleFormat::Int16)
		return "16 bit";
	if (format == SampleFormat::Int16Bigendian)
		return "16 bit BigEndian";
	if (format == SampleFormat::Int24)
		return "24 bit";
	if (format == SampleFormat::Int24Bigendian)
		return "24 bit BigEndian";
	if (format == SampleFormat::Int32)
		return "32 bit";
	if (format == SampleFormat::Int32Bigendian)
		return "32 bit BigEndian";
	if (format == SampleFormat::Float32)
		return "32 bit float";
	return "???";
}

string format_code(SampleFormat format) {
	if (format == SampleFormat::Int8)
		return "i8";
	if (format == SampleFormat::Int16)
		return "i16";
	if (format == SampleFormat::Int16Bigendian)
		return "i16be";
	if (format == SampleFormat::Int24)
		return "i24";
	if (format == SampleFormat::Int24Bigendian)
		return "i24be";
	if (format == SampleFormat::Int32)
		return "i32";
	if (format == SampleFormat::Int32Bigendian)
		return "i32be";
	if (format == SampleFormat::Float32)
		return "f32";
	return "???";
}

SampleFormat format_from_code(const string &code) {
	if (code == "i8")
		return SampleFormat::Int8;
	if (code == "i16")
		return SampleFormat::Int16;
	if (code == "i16be")
		return SampleFormat::Int16Bigendian;
	if (code == "i24")
		return SampleFormat::Int24;
	if (code == "i24be")
		return SampleFormat::Int24Bigendian;
	if (code == "i32")
		return SampleFormat::Int32;
	if (code == "i32be")
		return SampleFormat::Int32Bigendian;
	if (code == "f32")
		return SampleFormat::Float32;
	return SampleFormat::Unknown;
}

string signal_type_name(SignalType type) {
	if (type == SignalType::Audio)
		return _("Audio");
	if (type == SignalType::Midi)
		return _("Midi");
	if (type == SignalType::Beats)
		return _("Metronome");
	if (type == SignalType::AudioMono)
		return _("Audio (mono)");
	if (type == SignalType::AudioStereo)
		return _("Audio (stereo)");
	return "???";
}

}


