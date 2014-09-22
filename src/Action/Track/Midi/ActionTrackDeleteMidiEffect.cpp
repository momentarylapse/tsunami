/*
 * ActionTrackDeleteMidiEffect.cpp
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#include "ActionTrackDeleteMidiEffect.h"
#include "../../../Data/Track.h"
#include <assert.h>

ActionTrackDeleteMidiEffect::ActionTrackDeleteMidiEffect(Track *t, int _index)
{
	track_no = get_track_index(t);
	index = _index;
}

ActionTrackDeleteMidiEffect::~ActionTrackDeleteMidiEffect()
{
}

void *ActionTrackDeleteMidiEffect::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	assert(index >= 0);

	Track *t = a->get_track(track_no);
	assert(index < t->midi.fx.num);

	effect = t->midi.fx[index];
	effect->Notify(effect->MESSAGE_DELETE);
	t->midi.fx.erase(index);
	t->Notify(t->MESSAGE_DELETE_MIDI_EFFECT);

	return NULL;
}

void ActionTrackDeleteMidiEffect::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	assert(index >= 0);

	Track *t = a->get_track(track_no);
	assert(t);
	assert(index <= t->midi.fx.num);

	t->midi.fx.insert(effect, index);
	t->Notify(t->MESSAGE_ADD_MIDI_EFFECT);
}

