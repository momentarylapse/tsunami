/*
 * ActionTrackDeleteSample.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionTrackDeleteSample.h"
#include "../../../Data/AudioFile.h"

ActionTrackDeleteSample::ActionTrackDeleteSample(int _track_no, int _index)
{
	track_no = _track_no;
	index = _index;
}

ActionTrackDeleteSample::~ActionTrackDeleteSample()
{
}

void* ActionTrackDeleteSample::execute(Data* d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	ref = a->track[track_no]->sample[index];

	a->track[track_no]->sample.erase(index);

	return NULL;
}

void ActionTrackDeleteSample::undo(Data* d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	a->track[track_no]->sample.insert(ref, index);
}


