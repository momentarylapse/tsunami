/*
 * ActionTrackAddEffect.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionTrackAddEffect.h"
#include "../../../data/Track.h"
#include "../../../data/Song.h"
#include "../../../module/audio/AudioEffect.h"

ActionTrackAddEffect::ActionTrackAddEffect(Track *t, shared<AudioEffect> _effect) {
	track = t;
	effect = _effect;
}

void *ActionTrackAddEffect::execute(Data *d) {
	track->fx.add(effect);
	track->notify(track->MESSAGE_ADD_EFFECT);

	return nullptr;
}

void ActionTrackAddEffect::undo(Data *d) {
	effect->fake_death();
	track->fx.pop();
	track->notify(track->MESSAGE_DELETE_EFFECT);
}
