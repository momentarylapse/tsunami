/*
 * Synthesizer.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "Synthesizer.h"
#include "DummySynthesizer.h"
#include "SampleSynthesizer.h"
#include "../../Data/AudioFile.h"
#include "../../Tsunami.h"
#include "../../Stuff/Log.h"
#include "../../Plugins/PluginManager.h"
#include <math.h>

const int MANY_SAMPLES = 0x7fffffff;

float pitch_to_freq(float pitch)
{
	return 440.0f * pow(2, (pitch - 69.0f) / 12.0f);
}


// "scientific" notation
//   naive MIDI octave is off by 1
int pitch_get_octave(int pitch)
{
	return (pitch / 12) - 1;
}

int pitch_from_octave_and_rel(int rel, int octave)
{
	return rel + octave * 12 + 12;
}

int pitch_to_rel(int pitch)
{
	return pitch % 12;
}

string rel_pitch_name(int pitch_rel)
{
	if (pitch_rel == 0)
		return "C";
	if (pitch_rel == 1)
		return "C#";
	if (pitch_rel == 2)
		return "D";
	if (pitch_rel == 3)
		return "D#";
	if (pitch_rel == 4)
		return "E";
	if (pitch_rel == 5)
		return "F";
	if (pitch_rel == 6)
		return "F#";
	if (pitch_rel == 7)
		return "G";
	if (pitch_rel == 8)
		return "G#";
	if (pitch_rel == 9)
		return "A";
	if (pitch_rel == 10)
		return "A#";
	if (pitch_rel == 11)
		return "H";
	return "???";
}

string pitch_name(int pitch)
{
	return rel_pitch_name(pitch_to_rel(pitch)) + " " + i2s(pitch_get_octave(pitch));
}

Synthesizer::Synthesizer() :
	Configurable("Synthesizer", TYPE_SYNTHESIZER)
{
	sample_rate = DEFAULT_SAMPLE_RATE;
	keep_notes = 0;
	_reset();
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
	this->Configurable::~Configurable();
}

void Synthesizer::_reset()
{
	notes.clear();
	keep_notes = 0;
}

void Synthesizer::reset()
{
	_reset();
}

void Synthesizer::set(float pitch, float volume, int offset)
{
	// end active notes
	foreach(MidiNote &n, notes)
		if (n.pitch == pitch)
			if (n.range.is_inside(offset))
				n.range.num = offset - n.range.offset;

	// start a new note
	if (volume > 0){
		MidiNote n = MidiNote(Range(offset, MANY_SAMPLES), pitch, volume);
	}
}

void Synthesizer::iterate(int samples)
{
	for (int i=0;i<notes.num;i++){
		notes[i].range.offset -= samples;
		if (notes[i].range.end() + keep_notes < 0){
			notes.erase(i);
			i --;
		}
	}
}

int Synthesizer::read(BufferBox &buf)
{
	// get from source...

	foreach(MidiNote &n, notes)
		renderNote(buf, n.range, n.pitch, n.volume);

	iterate(buf.num);
	return buf.num;
}

void Synthesizer::renderMetronomeClick(BufferBox &buf, int pos, int level, float volume)
{
	if (level == 0)
		renderNote(buf, Range(pos, 0), 81, volume);
	else
		renderNote(buf, Range(pos, 0), 74, volume * 0.5f);
}


// factory
Synthesizer *CreateSynthesizer(const string &name)
{
	if ((name == "Dummy") || (name == ""))
		return new DummySynthesizer;
	/*if (name == "Sample")
		return new SampleSynthesizer;*/
	Synthesizer *s = tsunami->plugin_manager->LoadSynthesizer(name);
	if (s){
		s->resetConfig();
		s->name = name;
		return s;
	}
	tsunami->log->error(_("unbekannter Synthesizer: ") + name);
	s = new DummySynthesizer;
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
