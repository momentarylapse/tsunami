/*
 * ActionTrackAddSample.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "ActionTrackAddSample.h"

#include "../../../Data/TrackLayer.h"
#include "../../../Data/SampleRef.h"
#include "../../../Data/Sample.h"
#include <cassert>

ActionTrackAddSample::ActionTrackAddSample(TrackLayer *l, int _pos, shared<Sample> _sample) {
	layer = l;
	pos = _pos;

	sample = _sample;

	ref = new SampleRef(_sample);

	// "unlink" until execute()
	sample->unref();

	ref->layer = l;
	ref->pos = pos;
	ref->owner = l->song();
}

void ActionTrackAddSample::undo(Data *d) {
	assert(layer->samples.num > 0);
	layer->samples.pop();
	ref->fake_death();

	sample->unref();
}



void *ActionTrackAddSample::execute(Data *d) {
	sample->ref();

	layer->samples.add(ref);
	return ref.get();
}

