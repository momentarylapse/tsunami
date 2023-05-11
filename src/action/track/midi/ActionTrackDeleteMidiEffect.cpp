/*
 * ActionTrackDeleteMidiEffect.cpp
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#include "ActionTrackDeleteMidiEffect.h"
#include "../../../data/Track.h"
#include <assert.h>

ActionTrackDeleteMidiEffect::ActionTrackDeleteMidiEffect(Track *t, int _index) {
	track = t;
	index = _index;
	effect = t->midi_fx[index];
}

void *ActionTrackDeleteMidiEffect::execute(Data *d) {
	assert(index >= 0);
	assert(index < track->midi_fx.num);

	effect->fake_death();
	track->midi_fx.erase(index);
	track->out_midi_effect_list_changed.notify();

	return nullptr;
}

void ActionTrackDeleteMidiEffect::undo(Data *d) {
	assert(index >= 0);
	assert(index <= track->midi_fx.num);

	track->midi_fx.insert(effect, index);
	track->out_midi_effect_list_changed.notify();
}

