/*
 * Synthesizer.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "Synthesizer.h"

#include "../../Data/Song.h"
#include "DummySynthesizer.h"
#include "SampleSynthesizer.h"
#include "../../Midi/MidiSource.h"
#include "../../lib/math/math.h"


Synthesizer::Synthesizer() :
	Configurable(TYPE_SYNTHESIZER)
{
	sample_rate = 0;
	keep_notes = 0;
	instrument = Instrument(Instrument::TYPE_PIANO);
	source_run_out = false;

	tuning.set_default();

	setSampleRate(DEFAULT_SAMPLE_RATE);
}

Synthesizer::~Synthesizer()
{
}

void Synthesizer::__init__()
{
	new(this) Synthesizer;
}

void Synthesizer::__delete__()
{
	this->Synthesizer::~Synthesizer();
}

void Synthesizer::Tuning::set_default()
{
	for (int p=0; p<MAX_PITCH; p++)
		freq[p] = pitch_to_freq((float)p);
}

bool Synthesizer::Tuning::is_default()
{
	for (int p=0; p<MAX_PITCH; p++)
		if (fabs(freq[p] - pitch_to_freq(p)) > 0.01f)
			return false;
	return true;
}

void Synthesizer::setSampleRate(int _sample_rate)
{
	//if (_sample_rate == sample_rate)
	//	return;
	sample_rate = _sample_rate;

	update_delta_phi();
	onConfig();
}

void Synthesizer::update_delta_phi()
{
	for (int p=0; p<MAX_PITCH; p++)
		delta_phi[p] = tuning.freq[p] * 2.0f * pi / sample_rate;
}

void Synthesizer::setInstrument(Instrument &i)
{
	instrument = i;
	onConfig();
}

void Synthesizer::enablePitch(int pitch, bool enable)
{
	if (enable)
		active_pitch.add(pitch);
	else
		active_pitch.erase(pitch);
		// delayed deletion (makes things easier in the render() function)
		//delete_me.add(pitch);
}

int Synthesizer::read(AudioBuffer &buf, MidiSource *source)
{
	// get from source...
	events.samples = buf.length;
	int n = source->read(events);
	if (n < buf.length)
		source_run_out = true;

	buf.scale(0);

	if (hasRunOutOfData())
		return 0;

	render(buf);

	events.clear();

	return n;
}

bool Synthesizer::hasRunOutOfData()
{
	return (active_pitch.num == 0) and source_run_out;
}

void Synthesizer::reset()
{
	resetState();
	active_pitch.clear();
	source_run_out = false;
	//resetMidiData();
}

bool Synthesizer::isDefault()
{
	return (name == "Dummy") and (tuning.is_default());
}

