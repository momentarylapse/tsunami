/*
 * ActionSubTrackDelete.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionSubTrackDelete.h"
#include "../../Data/AudioFile.h"

ActionSubTrackDelete::ActionSubTrackDelete(int _track_no, int _index)
{
	track_no = _track_no;
	index = _index;
}

ActionSubTrackDelete::~ActionSubTrackDelete()
{
}

void* ActionSubTrackDelete::execute(Data* d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	sub = a->track[track_no]->sample[index];

	a->track[track_no]->sample.erase(index);

	return NULL;
}

void ActionSubTrackDelete::undo(Data* d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	a->track[track_no]->sample.insert(sub, index);
}


