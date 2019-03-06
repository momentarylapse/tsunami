/*
 * MidiEffect.cpp
 *
 *  Created on: 10.09.2014
 *      Author: michi
 */

#include "MidiEffect.h"
#include "../ModuleFactory.h"

#include "../../Session.h"
#include "../../lib/math/math.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/TrackLayer.h"
#include "../../Data/SongSelection.h"
#include "../../Action/Track/Buffer/ActionTrackEditBuffer.h"

MidiEffect::Output::Output(MidiEffect *_fx) : Port(SignalType::AUDIO, "out")
{
	fx = _fx;
}

int MidiEffect::Output::read_midi(MidiEventBuffer &buf)
{
	if (!fx->source)
		return buf.samples;
	return fx->source->read_midi(buf);
}

MidiEffect::MidiEffect() :
	Module(ModuleType::MIDI_EFFECT)
{
	port_out.add(new Output(this));
	port_in.add(InPortDescription(SignalType::MIDI, &source, "in"));
	source = nullptr;
	only_on_selection = false;
}

void MidiEffect::__init__()
{
	new(this) MidiEffect;
}

void MidiEffect::__delete__()
{
	this->MidiEffect::~MidiEffect();
}

void MidiEffect::prepare()
{
	reset_state();
	if (!usable)
		session->e(get_error());
}

void MidiEffect::apply(MidiNoteBuffer &midi, Track *t, bool log_error)
{
	// run
	process(&midi);

	if (!usable){
		msg_error("not usable... apply");
		if (log_error)
			session->e(_("While applying a midi effect: ") + get_error());
	}
}



void MidiEffect::process_layer(TrackLayer *l, const SongSelection &sel)
{
	MidiNoteBuffer midi = l->midi.get_notes_by_selection(sel);

	l->song()->begin_action_group();

	for (int i=l->midi.num-1; i>=0; i--)
		if (sel.has(l->midi[i]))
			l->delete_midi_note(l->midi[i]);

	process(&midi);
	for (MidiNote *n: midi)
		n->reset_meta();

	l->insert_midi_data(0, midi);
	l->song()->end_action_group();
}


MidiEffect *CreateMidiEffect(Session *session, const string &name)
{
	return (MidiEffect*)ModuleFactory::create(session, ModuleType::MIDI_EFFECT, name);
}

