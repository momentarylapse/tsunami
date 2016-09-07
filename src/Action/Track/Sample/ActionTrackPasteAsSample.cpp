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

ActionTrackPasteAsSample::ActionTrackPasteAsSample(Track *t, int pos, const BufferBox &buf)
{
	Sample* sample = (Sample*)addSubAction(new ActionSongAddSample("-paste-", buf), t->song);
	addSubAction(new ActionTrackAddSample(t, pos, sample), t->song);
}

ActionTrackPasteAsSample::ActionTrackPasteAsSample(Track *t, int pos, const MidiData &midi)
{
	msg_write("a");
	Sample* sample = (Sample*)addSubAction(new ActionSongAddSample("-paste-", midi), t->song);
	msg_write("b");
	addSubAction(new ActionTrackAddSample(t, pos, sample), t->song);
	msg_write("c");
}

