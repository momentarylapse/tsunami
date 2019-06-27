/*
 * ActionTrackDeleteSample.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionTrackDeleteSample.h"

#include "../../../Data/TrackLayer.h"
#include "../../../Data/SampleRef.h"
#include "../../../Data/Sample.h"

ActionTrackDeleteSample::ActionTrackDeleteSample(SampleRef *_ref)
{
	layer = _ref->layer;
	index = _ref->get_index();
	ref = nullptr;
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
	ref->owner = nullptr;

	ref->fake_death();
	layer->samples.erase(index);

	return nullptr;
}

void ActionTrackDeleteSample::undo(Data* d)
{
	layer->samples.insert(ref, index);
	ref->origin->ref();
	ref->owner = layer->song();
	ref = nullptr;
}


