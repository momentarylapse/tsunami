/*
 * ActionTrackDeleteEffect.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include <assert.h>
#include "ActionTrackDeleteAudioEffect.h"
#include "../../../data/Track.h"
#include "../../../data/Song.h"
#include "../../../module/audio/AudioEffect.h"

ActionTrackDeleteEffect::ActionTrackDeleteEffect(Track *t, int _index) {
	track = t;
	index = _index;
}

void *ActionTrackDeleteEffect::execute(Data *d) {
	assert(index >= 0);

	assert(index < track->fx.num);

	effect = track->fx[index];
	effect->fake_death();
	track->fx.erase(index);
	track->out_delete_effect.notify();

	return nullptr;
}

void ActionTrackDeleteEffect::undo(Data *d) {
	assert(index >= 0);

	assert(index <= track->fx.num);

	track->fx.insert(effect, index);
	track->out_add_effect.notify();
}

