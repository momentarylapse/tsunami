/*
 * ActionTrackAddEffect.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionTrackAddEffect.h"
#include "../../../Data/Track.h"
#include "../../../Data/Song.h"
#include "../../../Module/Audio/AudioEffect.h"

ActionTrackAddEffect::ActionTrackAddEffect(Track *t, AudioEffect *_effect) {
	track = t;
	effect = _effect;
}

ActionTrackAddEffect::~ActionTrackAddEffect() {
	if (effect)
		delete effect;
}

void *ActionTrackAddEffect::execute(Data *d) {
	track->fx.add(effect);
	track->notify(track->MESSAGE_ADD_EFFECT);
	effect = nullptr;

	return nullptr;
}

void ActionTrackAddEffect::undo(Data *d) {
	effect = track->fx.back();
	effect->fake_death();
	track->fx.pop();
	track->notify(track->MESSAGE_DELETE_EFFECT);
}

