/*
 * ActionTrackDeleteSample.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionTrackDeleteSample.h"

#include "../../../Data/Song.h"

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
	Song *a = dynamic_cast<Song*>(d);

	ref = a->tracks[track_no]->samples[index];
	ref->origin->unref();
	ref->owner = NULL;

	ref->notify(ref->MESSAGE_DELETE);
	a->tracks[track_no]->samples.erase(index);

	return NULL;
}

void ActionTrackDeleteSample::undo(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);

	a->tracks[track_no]->samples.insert(ref, index);
	ref->origin->ref();
	ref->owner = a;
	ref = NULL;
}


