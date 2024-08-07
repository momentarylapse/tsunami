/*
 * ActionTrackPasteAsSample.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "ActionTrackPasteAsSample.h"

#include "../../../data/base.h"
#include "../../../data/Track.h"
#include "../../../data/Sample.h"
#include "../../../data/SampleRef.h"
#include "../../sample/ActionSampleAdd.h"
#include "ActionTrackAddSample.h"

namespace tsunami {

ActionTrackPasteAsSample::ActionTrackPasteAsSample(TrackLayer *l, int _pos, const AudioBuffer &_buf, bool _auto_delete) {
	layer = l;
	pos = _pos;
	buf = &_buf;
	midi = nullptr;
	sample = nullptr;
	auto_delete = _auto_delete;
}

ActionTrackPasteAsSample::ActionTrackPasteAsSample(TrackLayer *l, int _pos, const MidiNoteBuffer &_midi, bool _auto_delete) {
	layer = l;
	pos = _pos;
	buf = nullptr;
	midi = &_midi;
	sample = nullptr;
	auto_delete = _auto_delete;
}

void ActionTrackPasteAsSample::build(Data *d) {
	if (buf) {
		sample = new Sample("-paste-", *buf);
	} else {
		sample = new Sample("-paste-", *midi);
	}
	sample->auto_delete = auto_delete;
	add_sub_action(new ActionSampleAdd(sample), d);
	add_sub_action(new ActionTrackAddSample(layer, pos, sample), d);
}

void *ActionTrackPasteAsSample::execute_return(Data *d) {
	return sample;
}

}

