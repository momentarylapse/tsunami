/*
 * ActionTrackInsertSample.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionTrackInsertSample.h"
#include "ActionTrackDeleteSample.h"
#include "../Buffer/ActionTrackCreateBuffers.h"
#include "../Buffer/ActionTrackEditBuffer.h"
#include "../../../Data/AudioFile.h"

ActionTrackInsertSample::ActionTrackInsertSample(AudioFile *a, int track_no, int index, int level_no)
{
	SampleRef *sub = a->track[track_no]->sample[index];

	// get target buffer
	Range r = sub->GetRange();
	AddSubAction(new ActionTrackCreateBuffers(a->track[track_no], level_no, r), a);
	BufferBox buf = a->track[track_no]->ReadBuffers(level_no, r);

	// insert sub (ignore muted)
	ActionTrackEditBuffer *action = new ActionTrackEditBuffer(a->track[track_no], level_no, r);
	buf.set(sub->buf, 0, sub->volume);
	AddSubAction(action, a);

	// delete sub
	AddSubAction(new ActionTrackDeleteSample(track_no, index), a);
}

ActionTrackInsertSample::~ActionTrackInsertSample()
{
}

