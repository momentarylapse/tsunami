/*
 * ActionTrackAddSample.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "ActionTrackAddSample.h"

#include "../../../Data/Song.h"

ActionTrackAddSample::ActionTrackAddSample(TrackLayer *l, int _pos, Sample *_sample)
{
	layer = l;
	pos = _pos;
	sample = _sample;
	ref = new SampleRef(sample);
	ref->origin->unref(); // cancel ref() until execute()
	ref->layer = l;
	ref->pos = pos;
	ref->owner = l->track->song;
}

ActionTrackAddSample::~ActionTrackAddSample()
{
	if (ref)
		delete ref;
}

void ActionTrackAddSample::undo(Data *d)
{
	ref = layer->samples.pop();
	ref->notify(ref->MESSAGE_DELETE);
	ref->origin->unref();
}



void *ActionTrackAddSample::execute(Data *d)
{
	ref->origin->ref();
	layer->samples.add(ref);
	ref = NULL;
	return layer->samples.back();
}

