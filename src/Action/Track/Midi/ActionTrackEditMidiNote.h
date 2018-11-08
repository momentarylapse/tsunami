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

class ActionTrackEditMidiNote : public Action
{
public:
	ActionTrackEditMidiNote(MidiNote *n, const Range &range, float pitch, float volume, int stringno);

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	MidiNote *note;
	int offset, length;
	float pitch, volume;
	int stringno;
};

#endif /* SRC_ACTION_TRACK_MIDI_ACTIONTRACKEDITMIDINOTE_H_ */
