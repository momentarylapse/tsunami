/*
 * ActionTrackToggleEffectEnabled.cpp
 *
 *  Created on: 30.03.2014
 *      Author: michi
 */

#include "ActionTrackToggleEffectEnabled.h"
#include "../../../module/audio/AudioEffect.h"
#include "../../../data/Song.h"

ActionTrackToggleEffectEnabled::ActionTrackToggleEffectEnabled(AudioEffect *_fx) {
	fx = _fx;
}

void *ActionTrackToggleEffectEnabled::execute(Data *d) {
	fx->enabled = !fx->enabled;
	fx->out_changed.notify();
	((Song*)d)->out_enable_fx.notify();

	return nullptr;
}

void ActionTrackToggleEffectEnabled::undo(Data *d) {
	execute(d);
}


