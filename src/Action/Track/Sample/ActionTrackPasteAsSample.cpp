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

ActionTrackPasteAsSample::ActionTrackPasteAsSample(Track *_t, int _pos, const BufferBox &_buf)
{
	t = _t;
	pos = _pos;
	buf = &_buf;
	midi = NULL;
}

ActionTrackPasteAsSample::ActionTrackPasteAsSample(Track *_t, int _pos, const MidiData &_midi)
{
	t = _t;
	pos = _pos;
	buf = NULL;
	midi = &_midi;
}

void ActionTrackPasteAsSample::build(Data *d)
{
	if (buf){
		Sample* sample = (Sample*)addSubAction(new ActionSongAddSample("-paste-", *buf), d);
		addSubAction(new ActionTrackAddSample(t, pos, sample), d);
	}else if (midi){
		Sample* sample = (Sample*)addSubAction(new ActionSongAddSample("-paste-", *midi), d);
		addSubAction(new ActionTrackAddSample(t, pos, sample), d);
	}
}

