/*
 * ActionTrackPasteAsSample.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "ActionTrackPasteAsSample.h"

#include "../../../Data/Song.h"
#include "../../Sample/ActionSampleAdd.h"
#include "ActionTrackAddSample.h"

ActionTrackPasteAsSample::ActionTrackPasteAsSample(Track *_t, int _pos, const AudioBuffer &_buf, bool _auto_delete)
{
	t = _t;
	pos = _pos;
	buf = &_buf;
	midi = NULL;
	sample = NULL;
	auto_delete = _auto_delete;
}

ActionTrackPasteAsSample::ActionTrackPasteAsSample(Track *_t, int _pos, const MidiData &_midi, bool _auto_delete)
{
	t = _t;
	pos = _pos;
	buf = NULL;
	midi = &_midi;
	sample = NULL;
	auto_delete = _auto_delete;
}

void ActionTrackPasteAsSample::build(Data *d)
{
	if (buf){
		sample = (Sample*)addSubAction(new ActionSampleAdd("-paste-", *buf, auto_delete), d);
		addSubAction(new ActionTrackAddSample(t, pos, sample), d);
	}else if (midi){
		sample = (Sample*)addSubAction(new ActionSampleAdd("-paste-", *midi, auto_delete), d);
		addSubAction(new ActionTrackAddSample(t, pos, sample), d);
	}
}

void *ActionTrackPasteAsSample::execute_return(Data *d)
{
	return sample;
}

