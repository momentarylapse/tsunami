/*
 * ActionTrackEditEffect.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionTrackEditEffect.h"
#include "../../../Data/Track.h"
#include <assert.h>

#include "../../../Module/Audio/AudioEffect.h"

ActionTrackEditEffect::ActionTrackEditEffect(AudioEffect *_fx) {
	fx = _fx;
	old_value = fx->_config_latest_history;
	new_value = fx->config_to_string();
}

void *ActionTrackEditEffect::execute(Data *d) {
	fx->_config_latest_history = new_value;
	fx->notify();
	return nullptr;
}

void ActionTrackEditEffect::redo(Data *d) {
	fx->_config_latest_history = new_value;
	fx->config_from_string(new_value);
	fx->notify();
}

void ActionTrackEditEffect::undo(Data *d) {
	fx->_config_latest_history = old_value;
	fx->config_from_string(old_value);
	fx->notify();
}

bool ActionTrackEditEffect::mergable(Action *a) {
	auto *aa = dynamic_cast<ActionTrackEditEffect*>(a);
	if (!aa)
		return false;
	return (aa->fx == fx);
}

