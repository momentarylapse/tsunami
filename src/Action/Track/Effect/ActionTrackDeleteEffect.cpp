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
	Song *a = dynamic_cast<Song*>(d);
	assert(index >= 0);

	if (track_no >= 0){
		Track *t = a->get_track(track_no);
		assert(index < t->fx.num);

		effect = t->fx[index];
		effect->Observable::notify(effect->MESSAGE_DELETE);
		t->fx.erase(index);
		t->notify(t->MESSAGE_DELETE_EFFECT);
	}else{
		assert(index < a->fx.num);

		effect = a->fx[index];
		effect->Observable::notify(effect->MESSAGE_DELETE);
		a->fx.erase(index);
		a->notify(a->MESSAGE_DELETE_EFFECT);
	}

	return NULL;
}

void ActionTrackDeleteEffect::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	assert(index >= 0);

	if (track_no >= 0){
		Track *t = a->get_track(track_no);
		assert(t);
		assert(index <= t->fx.num);

		t->fx.insert(effect, index);
		t->notify(t->MESSAGE_ADD_EFFECT);
	}else{
		assert(index < a->fx.num);

		a->fx.insert(effect, index);
		a->notify(a->MESSAGE_ADD_EFFECT);
	}
}

