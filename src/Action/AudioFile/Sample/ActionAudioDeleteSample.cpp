/*
 * ActionAudioDeleteSample.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "ActionAudioDeleteSample.h"
#include "../../../Data/AudioFile.h"
#include <assert.h>

ActionAudioDeleteSample::ActionAudioDeleteSample(int _index)
{
	index = _index;
	sample = NULL;
}

ActionAudioDeleteSample::~ActionAudioDeleteSample()
{
	if (sample)
		if (!sample->owner)
			delete(sample);
}

void *ActionAudioDeleteSample::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	assert(index >= 0);
	assert(index < a->sample.num);
	sample = a->sample[index];
	assert(sample->ref_count == 0);
	a->sample.erase(index);
	sample->owner = NULL;
	return NULL;
}

void ActionAudioDeleteSample::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	sample->owner = a;
	a->sample.insert(sample, index);
}

