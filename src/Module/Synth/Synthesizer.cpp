/*
 * Synthesizer.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "Synthesizer.h"
#include "../ModuleFactory.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Audio/AudioBuffer.h"
#include "DummySynthesizer.h"
#include "SampleSynthesizer.h"
#include "../../lib/math/math.h"
#include "../Port/MidiPort.h"

Synthesizer::Output::Output(Synthesizer *s)
{
	synth = s;
}

int Synthesizer::Output::sample_rate()
{
	return synth->sample_rate;
}

void Synthesizer::Output::reset()
{
	synth->reset();
	if (synth->source)
		synth->source->reset();
}

int Synthesizer::Output::read(AudioBuffer &buf)
{
	if (!synth->source)
		return 0;
//	printf("synth read %d\n", buf.length);
	synth->source_run_out = false;
	// get from source...
	synth->events.samples = buf.length;
	int n = synth->source->read(synth->events);
//	printf("sr  %d", n);
	if (n == synth->source->NOT_ENOUGH_DATA){
//		printf(" no data\n");
		return NOT_ENOUGH_DATA;
	}
	if (n == synth->source->END_OF_STREAM){
		synth->source_run_out = true;

		// if source end_of_stream but still active rendering
		//  => render full requested buf size
		n = buf.length;
	}

	//printf("  %d  %d\n", synth->active_pitch.num, n);
	if (synth->hasRunOutOfData()){
//		printf(" eos\n");
		return END_OF_STREAM;
	}

//	printf("...%d  %d  ok\n", n, buf.length);
	buf.scale(0);
	synth->render(buf);

	synth->events.clear();

	return buf.length;
}


Synthesizer::Synthesizer() :
	Module(ModuleType::SYNTHESIZER)
{
	out = new Output(this);
	port_out.add(PortDescription(SignalType::AUDIO, (Port**)&out, "out"));
	port_in.add(PortDescription(SignalType::MIDI, (Port**)&source, "in"));
	sample_rate = DEFAULT_SAMPLE_RATE;
	keep_notes = 0;
	instrument = Instrument(Instrument::Type::PIANO);
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

void Synthesizer::set_source(MidiPort *_source)
{
	source = _source;
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
	on_config();
}

void Synthesizer::update_delta_phi()
{
	for (int p=0; p<MAX_PITCH; p++)
		delta_phi[p] = tuning.freq[p] * 2.0f * pi / sample_rate;
}

void Synthesizer::setInstrument(Instrument &i)
{
	instrument = i;
	on_config();
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
	reset_state();
	active_pitch.clear();
	source_run_out = false;
}

bool Synthesizer::isDefault()
{
	return (module_subtype == "Dummy") and (tuning.is_default());
}

Synthesizer* CreateSynthesizer(Session *session, const string &name)
{
	return (Synthesizer*)ModuleFactory::create(session, ModuleType::SYNTHESIZER, name);
}

