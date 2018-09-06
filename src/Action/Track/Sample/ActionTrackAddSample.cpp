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

ActionTrackAddSample::ActionTrackAddSample(TrackLayer *l, int _pos, Sample *_sample)
{
	layer = l;
	pos = _pos;

	sample = _sample;
	sample->_pointer_ref();

	ref = new SampleRef(sample);

	// "unlink" until execute()
	sample->unref();

	ref->layer = l;
	ref->pos = pos;
	ref->owner = l->song();
}

ActionTrackAddSample::~ActionTrackAddSample()
{
	if (ref){
		sample->ref();
		delete ref;
	}

	sample->_pointer_unref();
	sample = nullptr;
}

void ActionTrackAddSample::undo(Data *d)
{
	ref = layer->samples.pop();
	ref->notify(ref->MESSAGE_DELETE);

	sample->unref();
}



void *ActionTrackAddSample::execute(Data *d)
{
	sample->ref();

	layer->samples.add(ref);
	ref = nullptr;
	return layer->samples.back();
}

