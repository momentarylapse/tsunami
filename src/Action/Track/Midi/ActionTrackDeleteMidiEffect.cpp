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
	track = t;
	index = _index;
	effect = nullptr;
}

ActionTrackDeleteMidiEffect::~ActionTrackDeleteMidiEffect()
{
	if (effect)
		delete effect;
}

void *ActionTrackDeleteMidiEffect::execute(Data *d)
{
	assert(index >= 0);
	assert(index < track->midi_fx.num);

	effect = track->midi_fx[index];
	effect->fake_death();
	track->midi_fx.erase(index);
	track->notify(track->MESSAGE_DELETE_MIDI_EFFECT);

	return nullptr;
}

void ActionTrackDeleteMidiEffect::undo(Data *d)
{
	assert(index >= 0);
	assert(index <= track->midi_fx.num);

	track->midi_fx.insert(effect, index);
	track->notify(track->MESSAGE_ADD_MIDI_EFFECT);
	effect = nullptr;
}

