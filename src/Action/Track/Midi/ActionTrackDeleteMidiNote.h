/*
 * ActionTrackDeleteMidiNote.h
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#pragma once

#include "../../Action.h"

class TrackLayer;
class MidiNote;

class ActionTrackDeleteMidiNote: public Action {
public:
	ActionTrackDeleteMidiNote(TrackLayer *l, int index);

	string name() const override { return ":##:delete midi"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	TrackLayer *layer;
	shared<MidiNote> note;
	int index;
};
