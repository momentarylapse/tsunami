/*
 * ActionAudioAddSample.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "ActionAudioAddSample.h"
#include "../../../Data/AudioFile.h"
#include <assert.h>

ActionAudioAddSample::ActionAudioAddSample(const string &name, BufferBox &buf)
{
	sample = new Sample;
	sample->buf = buf;
	sample->name = name;
}

ActionAudioAddSample::~ActionAudioAddSample()
{
	if (!sample->owner)
		delete(sample);
}

void *ActionAudioAddSample::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	sample->owner = a;
	a->sample.add(sample);
	return (void*)(long)(a->sample.num - 1);
}

void ActionAudioAddSample::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	assert(sample->ref_count == 0);
	a->sample.pop();
	sample->owner = NULL;
}

