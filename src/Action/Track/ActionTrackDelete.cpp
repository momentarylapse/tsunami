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

ActionTrackDelete::ActionTrackDelete(Song *a, int index)
{
	assert(index >= 0 && index < a->tracks.num);

	Track *t = a->tracks[index];

	// delete buffers
	foreachi(TrackLevel &l, t->levels, li)
		for (int i=l.buffers.num-1;i>=0;i--)
			addSubAction(new ActionTrack__DeleteBufferBox(t, li, i), a);

	// delete samples
	for (int i=t->samples.num-1;i>=0;i--)
		addSubAction(new ActionTrackDeleteSample(t, i), a);

	// delete the track itself
	addSubAction(new ActionTrack__DeleteEmpty(index), a);
}

ActionTrackDelete::~ActionTrackDelete()
{
}
