/*
 * ActionTrackAddMidiNote.h
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#pragma once

#include "../../Action.h"

namespace tsunami {

class TrackLayer;
class MidiNote;

class ActionTrackAddMidiNote : public Action {
public:
	ActionTrackAddMidiNote(TrackLayer *l, shared<MidiNote> n);

	string name() const override { return ":##:add midi"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	TrackLayer *layer;
	shared<MidiNote> note;
	int insert_index;
};

}
