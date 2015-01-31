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


const int MANY_SAMPLES = 0x60000000;

Synthesizer::Synthesizer() :
	Configurable("Synthesizer", TYPE_SYNTHESIZER)
{
	sample_rate = DEFAULT_SAMPLE_RATE;
	keep_notes = 0;
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

void Synthesizer::reset()
{
}

void Synthesizer::renderMetronomeClick(BufferBox &buf, int pos, int level, float volume)
{
	if (level == 0)
		renderNote(buf, Range(pos, 0), 81, volume);
	else
		renderNote(buf, Range(pos, 0), 74, volume * 0.5f);
}



void Synthesizer::add(const MidiEvent &e)
{
	msg_write("add");
	int p = e.pitch;
	for (int i=events[p].num-1; i>=0; i--)
		if (events[p][i].pos <= e.pos){
			events[p].insert(e, i+1);
			return;
		}
	events[p].add(e);
}

void Synthesizer::stopAll()
{
	foreach(MidiNote &n, cur_notes){
		if (n.range.end() > 0){
			add(MidiEvent(0, n.pitch, 0));
		}
	}
}

void Synthesizer::iterate(int samples)
{
	for (int i=0;i<cur_notes.num;i++){
		cur_notes[i].range.offset -= samples;
		if (cur_notes[i].range.end() + keep_notes < 0){
			cur_notes.erase(i);
			i --;
		}
	}
}

void Synthesizer::createNotes()
{
	for (int p=0; p<128; p++){
		foreach(MidiEvent &e, events[p]){
			if (e.volume > 0){
				// start new note
				MidiNote n = MidiNote(Range(e.pos, MANY_SAMPLES), e.pitch, e.volume);
				cur_notes.add(n);
			}else{
				// stop note
				foreach(MidiNote &n, cur_notes)
					if ((e.pitch == n.pitch) and (n.range.is_inside(e.pos)))
						n.range.set_end(e.pos);
			}
		}

		//events[p].clear();
	}
}

void Synthesizer::render(BufferBox &buf)
{
	foreach(MidiNote &n, cur_notes)
		renderNote(buf, n.range, n.pitch, n.volume);
}

int Synthesizer::read(BufferBox &buf)
{
	// get from source...
	buf.scale(0);

	createNotes();

	if ((auto_stop) and (cur_notes.num == 0))
		return 0;

	render(buf);

	iterate(buf.num);


	for (int p=0; p<128; p++)
		events[p].clear();

	return buf.num;
}

void Synthesizer::resetMidiData()
{
	for (int p=0; p<128; p++)
		events[p].clear();
	cur_notes.clear();
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
		msg_write("resetConfig");
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
