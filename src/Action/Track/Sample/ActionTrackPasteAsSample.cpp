/*
 * ActionTrackPasteAsSample.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "ActionTrackPasteAsSample.h"

#include "../../../Data/Track.h"
#include "../../../Data/SampleRef.h"
#include "../../Sample/ActionSampleAdd.h"
#include "ActionTrackAddSample.h"

ActionTrackPasteAsSample::ActionTrackPasteAsSample(TrackLayer *l, int _pos, const AudioBuffer &_buf, bool _auto_delete)
{
	layer = l;
	pos = _pos;
	buf = &_buf;
	midi = nullptr;
	sample = nullptr;
	auto_delete = _auto_delete;
}

ActionTrackPasteAsSample::ActionTrackPasteAsSample(TrackLayer *l, int _pos, const MidiNoteBuffer &_midi, bool _auto_delete)
{
	layer = l;
	pos = _pos;
	buf = nullptr;
	midi = &_midi;
	sample = nullptr;
	auto_delete = _auto_delete;
}

void ActionTrackPasteAsSample::build(Data *d)
{
	if (buf){
		sample = (Sample*)add_sub_action(new ActionSampleAdd("-paste-", *buf, auto_delete), d);
		add_sub_action(new ActionTrackAddSample(layer, pos, sample), d);
	}else if (midi){
		sample = (Sample*)add_sub_action(new ActionSampleAdd("-paste-", *midi, auto_delete), d);
		add_sub_action(new ActionTrackAddSample(layer, pos, sample), d);
	}
}

void *ActionTrackPasteAsSample::execute_return(Data *d)
{
	return sample;
}

