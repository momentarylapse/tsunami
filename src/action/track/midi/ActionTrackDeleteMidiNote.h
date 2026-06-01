/*
 * ActionTrackDeleteMidiNote.h
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>

namespace tsunami {

class TrackLayer;
class MidiNote;

class ActionTrackDeleteMidiNote: public history::Action {
public:
	ActionTrackDeleteMidiNote(TrackLayer *l, int index);

	string name() const override { return ":##:delete midi"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	TrackLayer *layer;
	shared<MidiNote> note;
	int index;
};

}
