/*
 * ActionTrackEditSample.cpp
 *
 *  Created on: 28.03.2014
 *      Author: michi
 */

#include "ActionTrackEditSample.h"

#include "../../../data/Track.h"
#include "../../../data/SampleRef.h"

ActionTrackEditSample::ActionTrackEditSample(shared<SampleRef> _ref, float volume, bool mute) {
	ref = _ref;
	old_value.volume = ref->volume;
	old_value.mute = ref->muted;
	new_value.volume = volume;
	new_value.mute = mute;
}

void *ActionTrackEditSample::execute(Data *d) {
	//Song *a = dynamic_cast<Song*>(d);

	ref->volume = new_value.volume;
	ref->muted = new_value.mute;
	ref->out_changed_by_action.notify();

	return nullptr;
}

void ActionTrackEditSample::undo(Data *d) {
	//Song *a = dynamic_cast<Song*>(d);

	ref->volume = old_value.volume;
	ref->muted = old_value.mute;
	ref->out_changed_by_action.notify();
}


bool ActionTrackEditSample::mergable(Action *a) {
	auto aa = dynamic_cast<ActionTrackEditSample*>(a);
	if (!aa)
		return false;
	return (aa->ref == ref.get());
}

