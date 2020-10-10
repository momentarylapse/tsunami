/*
 * ActionTrackEditMidiNote.h
 *
 *  Created on: 16.03.2018
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_MIDI_ACTIONTRACKEDITMIDINOTE_H_
#define SRC_ACTION_TRACK_MIDI_ACTIONTRACKEDITMIDINOTE_H_

#include "../../Action.h"

class Track;
class MidiNote;
class Range;
enum class NoteModifier;

class ActionTrackEditMidiNote : public Action {
public:
	ActionTrackEditMidiNote(shared<MidiNote> n, const Range &range, float pitch, float volume, int stringno, int flags);

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	shared<MidiNote> note;
	shared<MidiNote> note2;
};

#endif /* SRC_ACTION_TRACK_MIDI_ACTIONTRACKEDITMIDINOTE_H_ */
