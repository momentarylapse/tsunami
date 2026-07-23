/*
 * ActionTrackAddMidiNote.h
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>

namespace tsunami {

struct TrackLayer;
struct MidiNote;

class ActionTrackAddMidiNote : public history::Action {
public:
	ActionTrackAddMidiNote(TrackLayer *l, shared<MidiNote> n);

	string name() const override { return ":##:add midi"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	TrackLayer *layer;
	shared<MidiNote> note;
	int insert_index;
};

}
