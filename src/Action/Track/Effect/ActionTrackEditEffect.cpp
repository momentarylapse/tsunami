/*
 * ActionTrackEditEffect.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionTrackEditEffect.h"
#include "../../../Data/Track.h"
#include <assert.h>

ActionTrackEditEffect::ActionTrackEditEffect(Track *t, int _index, Effect &_effect)
{
	track_no = get_track_index(t);
	index = _index;
	effect = _effect;
}

ActionTrackEditEffect::~ActionTrackEditEffect()
{
}

void *ActionTrackEditEffect::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	assert(index >= 0);

	if (track_no >= 0){
		Track *t = a->get_track(track_no, -1);
		assert(t);
		assert(index < t->fx.num);

		Effect temp = effect;
		effect = t->fx[index];
		t->fx[index] = temp;
	}else{
		assert(index < a->fx.num);

		Effect temp = effect;
		effect = a->fx[index];
		a->fx[index] = temp;
	}

	return NULL;
}

void ActionTrackEditEffect::undo(Data *d)
{
	execute(d);
}

