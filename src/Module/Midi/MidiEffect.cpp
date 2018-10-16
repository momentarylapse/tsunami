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

MidiEffect::Output::Output(MidiEffect *_fx) : MidiPort("out")
{
	fx = _fx;
}

int MidiEffect::Output::read(MidiEventBuffer &buf)
{
	if (!fx->source)
		return buf.samples;
	return fx->source->read(buf);
}

void MidiEffect::Output::reset()
{
	if (fx->source)
		fx->source->reset();
}

MidiEffect::MidiEffect() :
	Module(ModuleType::MIDI_EFFECT)
{
	out = new Output(this);
	port_out.add(out);
	port_in.add(InPortDescription(SignalType::MIDI, (Port**)&source, "in"));
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

	l->song()->beginActionGroup();

	for (int i=l->midi.num-1; i>=0; i--)
		if (sel.has(l->midi[i]))
			l->deleteMidiNote(l->midi[i]);

	process(&midi);
	for (MidiNote *n: midi)
		n->reset_meta();

	l->insertMidiData(0, midi);
	l->song()->endActionGroup();
}


MidiEffect *CreateMidiEffect(Session *session, const string &name)
{
	return (MidiEffect*)ModuleFactory::create(session, ModuleType::MIDI_EFFECT, name);
}

