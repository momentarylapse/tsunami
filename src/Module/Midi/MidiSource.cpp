/*
 * MidiSource.cpp
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#include "MidiSource.h"
#include "../../Session.h"
#include "../ModuleFactory.h"
#include "../Beats/BeatSource.h"
#include "../../Data/base.h"


MidiSource::Output::Output(MidiSource *s) : MidiPort("out")
{
	source = s;
}

int MidiSource::Output::read(MidiEventBuffer &midi)
{
	int r = source->read(midi);
	return r;
}

void  MidiSource::Output::reset()
{
	source->reset();
}

MidiSource::MidiSource() :
	Module(ModuleType::MIDI_SOURCE)
{
	beat_source = BeatSource::dummy->out;
	out = new Output(this);
	port_out.add(out);
}

void MidiSource::__init__()
{
	new(this) MidiSource;
}

void MidiSource::__delete__()
{
	this->MidiSource::~MidiSource();
}

void MidiSource::set_beat_source(BeatPort *s)
{
	beat_source = s;
}




MidiSource *CreateMidiSource(Session *session, const string &name)
{
	return (MidiSource*)ModuleFactory::create(session, ModuleType::MIDI_SOURCE, name);
}

