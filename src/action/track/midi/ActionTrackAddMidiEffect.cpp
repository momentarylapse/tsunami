/*
 * ActionTrackAddMidiEffect.cpp
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#include "ActionTrackAddMidiEffect.h"
#include "../../../data/Track.h"
#include "../../../module/midi/MidiEffect.h"
#include <cassert>

ActionTrackAddMidiEffect::ActionTrackAddMidiEffect(Track *t, shared<MidiEffect> _effect) {
	track = t;
	effect = _effect;
}

void *ActionTrackAddMidiEffect::execute(Data *d) {
	track->midi_fx.add(effect);
	track->out_add_midi_effect.notify();

	return nullptr;
}

void ActionTrackAddMidiEffect::undo(Data *d) {
	assert(track->midi_fx.num > 0);
	track->midi_fx.pop();
	effect->fake_death();
	track->out_delete_midi_effect.notify();
}

