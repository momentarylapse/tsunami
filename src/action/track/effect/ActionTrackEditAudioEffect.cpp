/*
 * ActionTrackEditAudioEffect.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionTrackEditAudioEffect.h"
#include "../../../data/Track.h"
#include <assert.h>

#include "../../../module/audio/AudioEffect.h"

ActionTrackEditAudioEffect::ActionTrackEditAudioEffect(AudioEffect *_fx) {
	fx = _fx;
	old_value = fx->_config_latest_history;
	new_value = fx->config_to_string();
}

void *ActionTrackEditAudioEffect::execute(Data *d) {
	fx->_config_latest_history = new_value;
	fx->out_changed.notify();
	return nullptr;
}

void ActionTrackEditAudioEffect::redo(Data *d) {
	fx->_config_latest_history = new_value;
	fx->config_from_string(Module::VERSION_LATEST, new_value);
	fx->out_changed.notify();
}

void ActionTrackEditAudioEffect::undo(Data *d) {
	fx->_config_latest_history = old_value;
	fx->config_from_string(Module::VERSION_LATEST, old_value);
	fx->out_changed.notify();
}

bool ActionTrackEditAudioEffect::mergable(Action *a) {
	auto *aa = dynamic_cast<ActionTrackEditAudioEffect*>(a);
	if (!aa)
		return false;
	return (aa->fx == fx);
}

