/*
 * ActionTrackDeleteEffect.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionTrackDeleteEffect.h"
#include "../../../Data/Track.h"
#include <assert.h>

ActionTrackDeleteEffect::ActionTrackDeleteEffect(Track *t, int _index)
{
	track_no = get_track_index(t);
	index = _index;
}

ActionTrackDeleteEffect::~ActionTrackDeleteEffect()
{
}

void *ActionTrackDeleteEffect::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	assert(index >= 0);

	if (track_no >= 0){
		Track *t = a->get_track(track_no);
		assert(index < t->fx.num);

		effect = t->fx[index];
		effect->Notify("Delete");
		t->fx.erase(index);
		t->Notify("DeleteEffect");
	}else{
		assert(index < a->fx.num);

		effect = a->fx[index];
		effect->Notify("Delete");
		a->fx.erase(index);
		a->Notify("DeleteEffect");
	}

	return NULL;
}

void ActionTrackDeleteEffect::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	assert(index >= 0);

	if (track_no >= 0){
		Track *t = a->get_track(track_no);
		assert(t);
		assert(index <= t->fx.num);

		t->fx.insert(effect, index);
		t->Notify("AddEffect");
	}else{
		assert(index < a->fx.num);

		a->fx.insert(effect, index);
		a->Notify("AddEffect");
	}
}

