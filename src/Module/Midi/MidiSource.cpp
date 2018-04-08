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


MidiSource::Output::Output(MidiSource *s)
{
	source = s;
}

int MidiSource::Output::read(MidiEventBuffer &midi)
{
	return source->read(midi);
}

void  MidiSource::Output::reset()
{
	source->reset();
}

MidiSource::MidiSource() :
	Module(Type::MIDI_SOURCE)
{
	beat_source = BeatSource::dummy->out;
	out = new Output(this);
	port_out.add(PortDescription(SignalType::MIDI, (Port**)&out, "out"));
}

MidiSource::~MidiSource()
{
	delete out;
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
	return (MidiSource*)ModuleFactory::create(session, Module::Type::MIDI_SOURCE, name);
}

