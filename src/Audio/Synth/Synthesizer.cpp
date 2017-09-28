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

Synthesizer::Output::Output(Synthesizer *s)
{
	synth = s;
	source = NULL;
}

void Synthesizer::Output::reset()
{
	synth->reset();
	//source->reset();
}

void Synthesizer::Output::setSource(MidiSource *_source)
{
	source = _source;
}

int Synthesizer::Output::getSampleRate()
{
	return synth->sample_rate;
}

int Synthesizer::Output::read(AudioBuffer &buf)
{
	if (!source)
		return 0;
	printf("synth read %d\n", buf.length);
	synth->source_run_out = false;
	// get from source...
	synth->events.samples = buf.length;
	int n = source->read(synth->events);
	printf("sr  %d", n);
	if (n == source->NOT_ENOUGH_DATA){
		printf(" no data\n");
		return NOT_ENOUGH_DATA;
	}
	if (n == source->END_OF_STREAM){
		synth->source_run_out = true;

		// if source end_of_stream but still active rendering
		//  => render full requested buf size
		n = buf.length;
	}

	//printf("  %d  %d\n", synth->active_pitch.num, n);
	if (synth->hasRunOutOfData()){
		printf(" eos\n");
		return END_OF_STREAM;
	}

	printf("...%d  %d  ok\n", n, buf.length);
	buf.scale(0);
	synth->render(buf);

	synth->events.clear();

	return buf.length;
}


Synthesizer::Synthesizer() :
	Configurable(TYPE_SYNTHESIZER)
{
	out = new Output(this);
	sample_rate = 0;
	keep_notes = 0;
	instrument = Instrument(Instrument::TYPE_PIANO);
	source_run_out = false;

	tuning.set_default();

	setSampleRate(DEFAULT_SAMPLE_RATE);
}

Synthesizer::~Synthesizer()
{
	delete out;
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

