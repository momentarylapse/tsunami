/*
 * ActionTrackToggleEffectEnabled.cpp
 *
 *  Created on: 30.03.2014
 *      Author: michi
 */

#include "ActionTrackToggleEffectEnabled.h"
#include "../../../Module/Audio/AudioEffect.h"
#include "../../../Data/Song.h"

ActionTrackToggleEffectEnabled::ActionTrackToggleEffectEnabled(AudioEffect *_fx) {
	fx = _fx;
}

void *ActionTrackToggleEffectEnabled::execute(Data *d) {
	fx->enabled = !fx->enabled;
	fx->notify();
	d->notify(Song::MESSAGE_ENABLE_FX);

	return nullptr;
}

void ActionTrackToggleEffectEnabled::undo(Data *d) {
	execute(d);
}


