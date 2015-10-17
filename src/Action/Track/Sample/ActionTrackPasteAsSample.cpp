/*
 * ActionTrackPasteAsSample.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "ActionTrackPasteAsSample.h"

#include "../../../Data/Song.h"
#include "../../Song/Sample/ActionSongAddSample.h"
#include "ActionTrackAddSample.h"

ActionTrackPasteAsSample::ActionTrackPasteAsSample(Track *t, int pos, BufferBox *buf)
{
	addSubAction(new ActionSongAddSample("-paste-", *buf), t->song);
	addSubAction(new ActionTrackAddSample(t, pos, t->song->samples.num - 1), t->song);
}

ActionTrackPasteAsSample::ActionTrackPasteAsSample(Track *t, int pos, MidiNoteData *midi)
{
	addSubAction(new ActionSongAddSample("-paste-", *midi), t->song);
	addSubAction(new ActionTrackAddSample(t, pos, t->song->samples.num - 1), t->song);
}

