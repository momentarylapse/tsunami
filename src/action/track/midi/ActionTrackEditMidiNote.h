/*
 * ActionTrackEditMidiNote.h
 *
 *  Created on: 16.03.2018
 *      Author: michi
 */

#pragma once

#include "../../Action.h"

namespace tsunami {

class TrackLayer;
class MidiNote;
class Range;
enum class NoteModifier;

class ActionTrackEditMidiNote : public Action {
public:
	ActionTrackEditMidiNote(TrackLayer *l, shared<MidiNote> n, const Range &range, float pitch, float volume, int stringno, int flags);

	string name() const override { return ":##:edit midi"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	TrackLayer *layer;
	shared<MidiNote> note;
	shared<MidiNote> note2;
};

}
