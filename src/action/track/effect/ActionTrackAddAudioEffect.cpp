/*
 * ActionTrackAddAudioEffect.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionTrackAddAudioEffect.h"
#include "../../../data/Track.h"
#include "../../../data/Song.h"
#include "../../../module/audio/AudioEffect.h"

ActionTrackAddAudioEffect::ActionTrackAddAudioEffect(Track *t, shared<AudioEffect> _effect) {
	track = t;
	effect = _effect;
}

void *ActionTrackAddAudioEffect::execute(Data *d) {
	track->fx.add(effect);
	track->out_effect_list_changed.notify();

	return nullptr;
}

void ActionTrackAddAudioEffect::undo(Data *d) {
	effect->fake_death();
	track->fx.pop();
	track->out_effect_list_changed.notify();
}

