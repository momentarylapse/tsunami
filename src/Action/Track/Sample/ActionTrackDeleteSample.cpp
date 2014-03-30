/*
 * ActionTrackDeleteSample.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionTrackDeleteSample.h"
#include "../../../Data/AudioFile.h"

ActionTrackDeleteSample::ActionTrackDeleteSample(Track *t, int _index)
{
	track_no = get_track_index(t);
	index = _index;
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
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	ref = a->track[track_no]->sample[index];
	ref->origin->unref();
	ref->owner = NULL;

	a->track[track_no]->sample.erase(index);

	return NULL;
}

void ActionTrackDeleteSample::undo(Data* d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	a->track[track_no]->sample.insert(ref, index);
	ref->origin->ref();
	ref->owner = a;
	ref = NULL;
}


