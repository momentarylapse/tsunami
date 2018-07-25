/*
 * base.cpp
 *
 *  Created on: 25.07.2018
 *      Author: michi
 */

#include "base.h"
#include "../lib/hui/hui.h"
#include <math.h>


float amplitude2db(float amp)
{
	return log10(amp) * 20.0f;
}

float db2amplitude(float db)
{
	return pow(10, db * 0.05);
}


SampleFormat format_for_bits(int bits)
{
	if (bits == 8)
		return SampleFormat::SAMPLE_FORMAT_8;
	if (bits == 16)
		return SampleFormat::SAMPLE_FORMAT_16;
	if (bits == 24)
		return SampleFormat::SAMPLE_FORMAT_24;
	if (bits == 32)
		return SampleFormat::SAMPLE_FORMAT_32;
	return SampleFormat::SAMPLE_FORMAT_UNKNOWN;
}

int format_get_bits(SampleFormat format)
{
	if (format == SampleFormat::SAMPLE_FORMAT_8)
		return 8;
	if ((format == SampleFormat::SAMPLE_FORMAT_16) or (format == SampleFormat::SAMPLE_FORMAT_16_BIGENDIAN))
		return 16;
	if ((format == SampleFormat::SAMPLE_FORMAT_24) or (format == SampleFormat::SAMPLE_FORMAT_24_BIGENDIAN))
		return 24;
	if ((format == SampleFormat::SAMPLE_FORMAT_32) or (format == SampleFormat::SAMPLE_FORMAT_32_BIGENDIAN) or (format == SampleFormat::SAMPLE_FORMAT_32_FLOAT))
		return 32;
	return 0;
}

string format_name(SampleFormat format)
{
	if (format == SampleFormat::SAMPLE_FORMAT_8)
		return "8 bit";
	if (format == SampleFormat::SAMPLE_FORMAT_16)
		return "16 bit";
	if (format == SampleFormat::SAMPLE_FORMAT_16_BIGENDIAN)
		return "16 bit BigEndian";
	if (format == SampleFormat::SAMPLE_FORMAT_24)
		return "24 bit";
	if (format == SampleFormat::SAMPLE_FORMAT_24_BIGENDIAN)
		return "24 bit BigEndian";
	if (format == SampleFormat::SAMPLE_FORMAT_32)
		return "32 bit";
	if (format == SampleFormat::SAMPLE_FORMAT_32_BIGENDIAN)
		return "32 bit BigEndian";
	if (format == SampleFormat::SAMPLE_FORMAT_32_FLOAT)
		return "32 bit float";
	return "???";
}

string signal_type_name(SignalType type)
{
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




