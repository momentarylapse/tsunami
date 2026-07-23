/*
 * ActionTrackInsertMidi.h
 *
 *  Created on: 23.02.2013
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>
#include "../../../data/midi/MidiData.h"

namespace tsunami {

struct TrackLayer;

class ActionTrackInsertMidi : public history::Action {
public:
	ActionTrackInsertMidi(TrackLayer *l, int offset, const MidiNoteBuffer &midi);

	string name() const override { return ":##:add midi"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	TrackLayer *layer;
	MidiNoteBuffer midi;
	int offset;
	Array<int> inserted_at;
	bool applied;
};

}
