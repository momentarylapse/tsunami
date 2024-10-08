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
#include "../../lib/os/msg.h"
#include "../../data/base.h"
#include "../../data/Song.h"
#include "../../data/TrackLayer.h"
#include "../../data/SongSelection.h"

namespace tsunami {

int MidiEffect::read_midi(int port, MidiEventBuffer &buf) {
	if (!in.source)
		return Return::NoSource;
	int r = in.source->read_midi(buf);
	process(buf);
	return r;
}

MidiEffect::MidiEffect() :
	Module(ModuleCategory::MidiEffect, "")
{
}

void MidiEffect::__init__() {
	new(this) MidiEffect;
}

void MidiEffect::__delete__() {
	this->MidiEffect::~MidiEffect();
}


void MidiEffect::process_layer(TrackLayer *l, const SongSelection &sel) {
	msg_error("TODO: MidiEffect.process_layer()");
	/*MidiNoteBuffer midi = l->midi.get_notes_by_selection(sel);

	l->song()->begin_action_group("midi fx process layer");

	for (int i=l->midi.num-1; i>=0; i--)
		if (sel.has(l->midi[i].get()))
			l->delete_midi_note(l->midi[i].get());

	auto ref = l->midi.get_notes(Range::ALL);

	process(&midi);

	l->insert_midi_data(0, midi);
	l->song()->end_action_group();

	// select new notes
	for (auto *n: weak(l->midi))
		if (!ref.has(n))
			sel.set(n, true);*/
}


MidiEffect *CreateMidiEffect(Session *session, const string &name) {
	return (MidiEffect*)ModuleFactory::create(session, ModuleCategory::MidiEffect, name);
}

}

