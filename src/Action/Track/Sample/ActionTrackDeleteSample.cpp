/*
 * ActionTrackDeleteSample.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionTrackDeleteSample.h"

#include "../../../Data/Song.h"

ActionTrackDeleteSample::ActionTrackDeleteSample(SampleRef *_ref)
{
	layer = _ref->layer;
	index = _ref->get_index();
	ref = NULL;
}

ActionTrackDeleteSample::~ActionTrackDeleteSample()
{
	if (ref)
		if (!ref->owner)
			delete(ref);
}

void* ActionTrackDeleteSample::execute(Data* d)
{

	ref = layer->samples[index];
	ref->origin->unref();
	ref->owner = NULL;

	ref->notify(ref->MESSAGE_DELETE);
	layer->samples.erase(index);

	return NULL;
}

void ActionTrackDeleteSample::undo(Data* d)
{
	layer->samples.insert(ref, index);
	ref->origin->ref();
	ref->owner = layer->track->song;
	ref = NULL;
}


