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

ActionTrackDelete::ActionTrackDelete(int _index)
{
	index = _index;
}

void ActionTrackDelete::build(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);
	assert(index >= 0 and index < s->tracks.num);

	Track *t = s->tracks[index];

	// delete buffers
	foreachi(TrackLevel &l, t->levels, li)
		for (int i=l.buffers.num-1;i>=0;i--)
			addSubAction(new ActionTrack__DeleteBufferBox(t, li, i), d);

	// delete samples
	for (int i=t->samples.num-1;i>=0;i--)
		addSubAction(new ActionTrackDeleteSample(t, i), d);

	// delete the track itself
	addSubAction(new ActionTrack__DeleteEmpty(index), d);
}
