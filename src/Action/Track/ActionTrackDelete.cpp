/*
 * ActionTrackDelete.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include "ActionTrackDelete.h"
#include "Sample/ActionTrackDeleteSample.h"
#include "ActionTrack__DeleteEmpty.h"
#include <assert.h>
#include "Buffer/ActionTrack__DeleteBuffer.h"

ActionTrackDelete::ActionTrackDelete(Track *_track)
{
	track = _track;
}

void ActionTrackDelete::build(Data *d)
{
	//Song *s = dynamic_cast<Song*>(d);

	// delete buffers
	for (TrackLayer *l: track->layers){
		for (int i=l->buffers.num-1; i>=0; i--)
			addSubAction(new ActionTrack__DeleteBuffer(l, i), d);

		// delete samples
		for (int i=l->samples.num-1; i>=0; i--)
			addSubAction(new ActionTrackDeleteSample(l->samples[i]), d);
	}

	// delete the track itself
	addSubAction(new ActionTrack__DeleteEmpty(track), d);
}
