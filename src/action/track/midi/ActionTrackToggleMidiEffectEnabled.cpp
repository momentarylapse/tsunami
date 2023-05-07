/*
 * ActionTrackToggleMidiEffectEnabled.cpp
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#include "ActionTrackToggleMidiEffectEnabled.h"
#include "../../../data/Song.h"
#include "../../../module/midi/MidiEffect.h"

ActionTrackToggleMidiEffectEnabled::ActionTrackToggleMidiEffectEnabled(MidiEffect *_fx) {
	fx = _fx;
}

void *ActionTrackToggleMidiEffectEnabled::execute(Data *d) {
	fx->enabled = !fx->enabled;
	fx->out_changed.notify();
	((Song*)d)->out_enable_fx.notify();

	return nullptr;
}

void ActionTrackToggleMidiEffectEnabled::undo(Data *d) {
	execute(d);
}


