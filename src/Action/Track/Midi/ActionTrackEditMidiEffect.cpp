/*
 * ActionTrackEditMidiEffect.cpp
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#include "ActionTrackEditMidiEffect.h"
#include "../../../Data/Track.h"
#include <assert.h>

#include "../../../Module/Midi/MidiEffect.h"

ActionTrackEditMidiEffect::ActionTrackEditMidiEffect(MidiEffect *_fx) {
	fx = _fx;
	old_value = fx->_config_latest_history;
	new_value = fx->config_to_string();
}

void *ActionTrackEditMidiEffect::execute(Data *d) {
	fx->_config_latest_history = new_value;
	fx->config_from_string(new_value);
	fx->notify();
	return nullptr;
}

void ActionTrackEditMidiEffect::redo(Data *d) {
	fx->_config_latest_history = new_value;
	fx->config_from_string(old_value);
	fx->notify();
}

void ActionTrackEditMidiEffect::undo(Data *d) {
	fx->_config_latest_history = old_value;
	fx->config_from_string(old_value);
	fx->notify();
}

bool ActionTrackEditMidiEffect::mergable(Action *a) {
	auto *aa = dynamic_cast<ActionTrackEditMidiEffect*>(a);
	if (!aa)
		return false;
	return (aa->fx == fx);
}

