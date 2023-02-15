/*
 * PitchDetector.cpp
 *
 *  Created on: 13.09.2017
 *      Author: michi
 */

#include "PitchDetector.h"
#include "../port/Port.h"
#include "../../data/base.h"
#include "../../data/audio/AudioBuffer.h"
#include "../../data/midi/MidiData.h"
#include "../../lib/fft/fft.h"
#include "../../lib/math/complex.h"



const int F_MIN = 50;
const int F_MAX = 2000;
const float THRESHOLD = 0.1f;
const int BUFFER_SIZE = 32768;

float get_freq(float i) {
	return i / BUFFER_SIZE * DEFAULT_SAMPLE_RATE;
}

int freq_to_index(float f) {
	return f*BUFFER_SIZE / DEFAULT_SAMPLE_RATE;
}

PitchDetector::PitchDetector() {
	module_category = ModuleCategory::PITCH_DETECTOR;
	port_in.add(InPortDescription(SignalType::AUDIO, &source, "in"));
	source = nullptr;
	loud_enough = false;
	volume = 0;
	frequency = 0;
	pitch = 0;
}

void PitchDetector::__init__() {
	new(this) PitchDetector;
}

int PitchDetector::read(MidiEventBuffer& midi) {
	AudioBuffer buf;
	buf.resize(midi.samples);
	int l = source->read_audio(buf);
	if (l <= 0)
		return l;

	process(midi, buf);
	if (loud_enough) {
		midi.add(MidiEvent(0, pitch, 1));
		midi.add(MidiEvent(midi.samples-1, pitch, 0));
	}

	return l;
}


DummyPitchDetector::DummyPitchDetector() {
	module_class = "Dummy";
}

void DummyPitchDetector::__init__() {
	new(this) DummyPitchDetector;
}

void DummyPitchDetector::process(MidiEventBuffer &midi, AudioBuffer &buf) {
	Array<float> temp;
	temp = buf.c[0];// + buf.c[1];

	// fft
	Array<complex> bufc;
	fft::r2c(temp, bufc);

	int i0 = freq_to_index(F_MIN);
	int i1 = freq_to_index(F_MAX);

	float max = 0;
	int imax = 0;
	[[maybe_unused]]  int imax2 = 0;
	for (int i=0; i<bufc.num; i++)
		if ((i > i0) and (i < i1)) {
			float amp = abs(bufc[i].x) * (1.0 - (float)i / bufc.num);
			//floatout f
			//if (amp > max * 2.5) or (amp > max and i < imax2)
			if (amp > max){
				max = amp;
				imax = i;
				imax2 = i * 1.5f;
			}
		}
	max /= sqrt(buf.length) * 2 * pi;
	volume = clamp(max / THRESHOLD / 2, 0.0f, 1.0f);
	loud_enough = (max > THRESHOLD);
	float fmax = get_freq(imax);

	// update values with some inertia
	if (loud_enough) {
		if (abs(log(fmax / frequency)) < 0.1f)
			frequency *= 1 + log(fmax / frequency) * 0.1;
		else
			frequency = fmax;
		//frequency = fmax
		pitch = freq_to_pitch(frequency);
	}
}
