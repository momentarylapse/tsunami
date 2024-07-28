/*
 * ActionTrackEditSynthesizer.cpp
 *
 *  Created on: 29.12.2013
 *      Author: michi
 */

#include "ActionTrackEditSynthesizer.h"
#include "../../../data/Track.h"
#include "../../../module/synthesizer/Synthesizer.h"
#include <assert.h>

namespace tsunami {

ActionTrackEditSynthesizer::ActionTrackEditSynthesizer(Track *t) {
	track = t;
	old_value = t->synth->_config_latest_history;
	new_value = t->synth->config_to_string();
}

void *ActionTrackEditSynthesizer::execute(Data *d) {
	track->synth->_config_latest_history = new_value;
	track->synth->out_changed.notify();
	return nullptr;
}

void ActionTrackEditSynthesizer::redo(Data *d) {
	track->synth->_config_latest_history = new_value;
	track->synth->config_from_string(Module::VersionNumber::Latest, new_value);
	track->synth->out_changed.notify();
}

void ActionTrackEditSynthesizer::undo(Data *d) {
	track->synth->_config_latest_history = old_value;
	track->synth->config_from_string(Module::VersionNumber::Latest, old_value);
	track->synth->out_changed.notify();
}

bool ActionTrackEditSynthesizer::mergable(Action *a) {
	auto *aa = dynamic_cast<ActionTrackEditSynthesizer*>(a);
	if (!aa)
		return false;
	return (aa->track == track);
}

}

