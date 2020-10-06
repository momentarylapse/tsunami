/*
 * ActionTrackDeleteMidiNote.h
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#ifndef ACTIONTRACKDELETEMIDINOTE_H_
#define ACTIONTRACKDELETEMIDINOTE_H_

#include "../../Action.h"

class TrackLayer;
class MidiNote;

class ActionTrackDeleteMidiNote: public Action {
public:
	ActionTrackDeleteMidiNote(TrackLayer *l, int index);

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	TrackLayer *layer;
	shared<MidiNote> note;
	int index;
};

#endif /* ACTIONTRACKDELETEMIDINOTE_H_ */
