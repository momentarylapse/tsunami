/*
 * ActionTrackEditEffect.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionTrackEditEffect.h"
#include "../../../Data/Track.h"
#include <assert.h>

ActionTrackEditEffect::ActionTrackEditEffect(Track *t, int _index, Array<EffectParam> &_params)
{
	track_no = get_track_index(t);
	index = _index;
	params = _params;
}

ActionTrackEditEffect::~ActionTrackEditEffect()
{
}

void *ActionTrackEditEffect::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	assert(index >= 0);

	if (track_no >= 0){
		Track *t = a->get_track(track_no);
		assert(t);
		assert(index < t->fx.num);

		Array<EffectParam> temp = params;
		params = t->fx[index]->param;
		t->fx[index]->param = temp;
	}else{
		assert(index < a->fx.num);

		Array<EffectParam> temp = params;
		params = a->fx[index]->param;
		a->fx[index]->param = temp;
	}

	return NULL;
}

void ActionTrackEditEffect::undo(Data *d)
{
	execute(d);
}

