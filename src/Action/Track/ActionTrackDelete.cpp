/*
 * ActionTrackDelete.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include "ActionTrackDelete.h"
#include "Buffer/ActionTrack__DeleteBufferBox.h"
#include "Sample/ActionTrackDeleteSample.h"
#include "ActionTrack__DeleteEmpty.h"
#include <assert.h>

ActionTrackDelete::ActionTrackDelete(AudioFile *a, int index)
{
	assert(index >= 0 && index < a->track.num);

	Track *t = a->track[index];

	// delete buffers
	foreachi(TrackLevel &l, t->level, li)
		for (int i=l.buffer.num-1;i>=0;i--)
			AddSubAction(new ActionTrack__DeleteBufferBox(t, li, i), a);

	// delete samples
	for (int i=t->sample.num-1;i>=0;i--)
		AddSubAction(new ActionTrackDeleteSample(t, i), a);

	// delete the track itself
	AddSubAction(new ActionTrack__DeleteEmpty(index), a);
}

ActionTrackDelete::~ActionTrackDelete()
{
}
