/*
 * ActionTrackEditEffect.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionTrackEditEffect.h"
#include "../../../Data/Track.h"
#include <assert.h>

ActionTrackEditEffect::ActionTrackEditEffect(Track *t, int _index, const string &_params)
{
	track_no = get_track_index(t);
	index = _index;
	params = _params;
	first_execution = true;
}

ActionTrackEditEffect::~ActionTrackEditEffect()
{
}

void *ActionTrackEditEffect::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	assert(index >= 0);

	// old_params... so don't execute on first run
	if (first_execution){
		first_execution = false;
		return NULL;
	}

	Effect *fx;

	if (track_no >= 0){
		Track *t = a->get_track(track_no);
		assert(t);
		assert(index < t->fx.num);

		fx = t->fx[index];
	}else{
		assert(index < a->fx.num);

		fx = a->fx[index];
	}

	fx->ConfigToString();
	string temp = params;
	params = fx->ConfigToString();
	fx->ConfigFromString(temp);

	return NULL;
}

void ActionTrackEditEffect::undo(Data *d)
{
	execute(d);
}

