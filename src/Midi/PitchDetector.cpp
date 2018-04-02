/*
 * PitchDetector.cpp
 *
 *  Created on: 13.09.2017
 *      Author: michi
 */

#include "PitchDetector.h"
#include "../Audio/AudioBuffer.h"
#include "../Audio/Source/AudioPort.h"
#include "../Plugins/FastFourierTransform.h"



const int F_MIN = 50;
const int F_MAX = 2000;
const float THRESHOLD = 0.1f;
const int BUFFER_SIZE = 32768;

float get_freq(float i)
{
	return i / BUFFER_SIZE * DEFAULT_SAMPLE_RATE;
}

int freq_to_index(float f)
{
	return f*BUFFER_SIZE / DEFAULT_SAMPLE_RATE;
}

PitchDetector::PitchDetector(AudioPort *_source)
{
	source = _source;
	loud_enough = false;
	volume = 0;
	frequency = 0;
}

PitchDetector::~PitchDetector()
{
}

void PitchDetector::__init__(AudioPort *source)
{
	new(this) PitchDetector(source);
}

void PitchDetector::__delete__()
{
	this->~PitchDetector();
}

void PitchDetector::process(MidiEventBuffer &midi, AudioBuffer &buf)
{
	Array<float> temp;
	temp = buf.c[0] + buf.c[1];

	// fft
	Array<complex> bufc;
	bufc.resize(buf.length / 2 + 1);
	FastFourierTransform::fft_r2c(temp, bufc);

	int i0 = freq_to_index(F_MIN);
	int i1 = freq_to_index(F_MAX);

	float max = 0;
	int imax = 0, imax2 = 0;
	for (int i=0; i<bufc.num; i++)
		if ((i > i0) and (i < i1)){
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
	volume = clampf(max / THRESHOLD / 2, 0, 1);
	loud_enough = (max > THRESHOLD);
	float fmax = get_freq(imax);

	// update values with some inertia
	if (loud_enough){
		if (abs(log(fmax / frequency)) < 0.1f)
			frequency *= 1 + log(fmax / frequency) * 0.1;
		else
			frequency = fmax;
		//frequency = fmax
	}
}

int PitchDetector::read(MidiEventBuffer& midi)
{
	AudioBuffer buf;
	buf.resize(midi.samples);
	int l = source->read(buf);

	process(midi, buf);

	return l;
}
