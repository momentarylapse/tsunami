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
#include "../../Tsunami.h"
#include "../../Stuff/Log.h"
#include "../../Plugins/PluginManager.h"
#include "../../lib/math/math.h"


Synthesizer::Synthesizer() :
	Configurable("Synthesizer", TYPE_SYNTHESIZER)
{
	sample_rate = 0;
	keep_notes = 0;
	locked = false;
	instrument = Instrument(Instrument::TYPE_PIANO);

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

void Synthesizer::lock()
{
	// really unsafe locking mechanism
	printf("syn.lock\n");
	while (locked){}
	printf("...\n");
	locked = true;
}

void Synthesizer::unlock()
{
	printf("syn.unlock\n");
	locked = false;
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

int Synthesizer::read(BufferBox &buf, MidiSource *source)
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

	return buf.length;
}

bool Synthesizer::hasRunOutOfData()
{
	return (events.num == 0) and (active_pitch.num == 0) and source_run_out;
}

void Synthesizer::reset()
{
	resetState();
	active_pitch.clear();
	//resetMidiData();
}

bool Synthesizer::isDefault()
{
	return (name == "Dummy") and (tuning.is_default());
}



// factory
Synthesizer *CreateSynthesizer(const string &name, Song *song)
{
	if ((name == "Dummy") or (name == ""))
		return new DummySynthesizer;
	/*if (name == "Sample")
		return new SampleSynthesizer;*/
	Synthesizer *s = tsunami->plugin_manager->LoadSynthesizer(name, song);
	if (s){
		s->resetConfig();
		s->name = name;
		return s;
	}
	tsunami->log->error(_("unknown synthesizer: ") + name);
	s = new DummySynthesizer;
	s->song = song;
	s->name = name;
	return s;
}

Array<string> FindSynthesizers()
{
	Array<string> names = tsunami->plugin_manager->FindSynthesizers();
	names.add("Dummy");
	//names.add("Sample");
	return names;
}
