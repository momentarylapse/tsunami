/*
 * ActionAudioDeleteTrack.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include "ActionAudioDeleteTrack.h"
#include "ActionTrack__DeleteBufferBox.h"
#include "ActionAudio__DeleteTrack.h"
#include <assert.h>

ActionAudioDeleteTrack::ActionAudioDeleteTrack(AudioFile *a, int index)
{
	assert(index >= 0 && index < a->track.num);

	Track &t = a->track[index];

	// delete buffers
	for (int i=t.buffer.num-1;i>=0;i--)
		AddSubAction(new ActionTrack__DeleteBufferBox(&t, i), a);

	// delete subs
	t.sub.clear();

	// delete the track itself
	AddSubAction(new ActionAudio__DeleteTrack(index), a);
}

ActionAudioDeleteTrack::~ActionAudioDeleteTrack()
{
}
