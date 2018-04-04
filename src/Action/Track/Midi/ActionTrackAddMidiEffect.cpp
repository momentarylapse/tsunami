/*
 * ActionTrackAddMidiEffect.cpp
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#include "ActionTrackAddMidiEffect.h"
#include "../../../Data/Track.h"
#include "../../../Module/Midi/MidiEffect.h"

ActionTrackAddMidiEffect::ActionTrackAddMidiEffect(Track *t, MidiEffect *_effect)
{
	track_no = get_track_index(t);
	effect = _effect;
}

void *ActionTrackAddMidiEffect::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	Track *t = a->get_track(track_no);
	t->midi.fx.add(effect);
	t->notify(t->MESSAGE_ADD_MIDI_EFFECT);

	return NULL;
}

void ActionTrackAddMidiEffect::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	effect->Observable::notify(effect->MESSAGE_DELETE);
	Track *t = a->get_track(track_no);
	t->midi.fx.pop();
	t->notify(t->MESSAGE_DELETE_MIDI_EFFECT);
}

