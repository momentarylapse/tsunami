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
#include "../../Tsunami.h"
#include "../../Stuff/Log.h"
#include "../../Plugins/PluginManager.h"
#include "../../lib/math/math.h"


Synthesizer::Synthesizer() :
	Configurable("Synthesizer", TYPE_SYNTHESIZER)
{
	sample_rate = 0;
	keep_notes = 0;
	auto_stop = true;
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
	events.clear();
	active_pitch.clear();
	delete_me.clear();
	this->Configurable::~Configurable();
}

void Synthesizer::Tuning::set_default()
{
	for (int p=0; p<MAX_PITCH; p++)
		freq[p] = pitch_to_freq(p);
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
	while (locked){}
	locked = true;
}

void Synthesizer::unlock()
{
	locked = false;
}


// _events should be sorted...
void Synthesizer::feed(const MidiRawData &_events)
{
	events.append(_events);
}

/*void Synthesizer::add(const MidiEvent &e)
{
	for (int i=events.num-1; i>=0; i--)
		if (events[i].pos <= e.pos){
			events.insert(e, i+1);
			return;
		}
	events.add(e);
}*/

void Synthesizer::endAllNotes()
{
	MidiRawData midi;
	foreach(int p, active_pitch)
		midi.add(MidiEvent(0, p, 0));
	if (midi.num > 0)
		feed(midi);
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

void Synthesizer::iterateEvents(int samples)
{
	for (int i=events.num-1; i>=0; i--)
		if (events[i].pos >= samples)
			events[i].pos -= samples;
		else
			events.erase(i);
	events.samples = max(events.samples - samples, 0);
}

int Synthesizer::read(BufferBox &buf)
{
	// get from source...
	buf.scale(0);

	if (auto_stop and hasEnded())
		return 0;

	render(buf);

	iterateEvents(buf.length);

	return buf.length;
}

bool Synthesizer::hasEnded()
{
	return (events.num == 0) and (events.samples == 0) and (active_pitch.num == 0);
}

void Synthesizer::resetMidiData()
{
	events.clear();
	events.samples = 0;
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
	tsunami->log->error(_("unbekannter Synthesizer: ") + name);
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
