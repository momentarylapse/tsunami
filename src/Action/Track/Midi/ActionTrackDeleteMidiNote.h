/*
 * ActionTrackDeleteMidiNote.h
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#ifndef ACTIONTRACKDELETEMIDINOTE_H_
#define ACTIONTRACKDELETEMIDINOTE_H_

#include "../../Action.h"

class Track;
class MidiNote;

class ActionTrackDeleteMidiNote: public Action
{
public:
	ActionTrackDeleteMidiNote(Track *t, int index);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int track_no;
	MidiNote *note;
	int index;
};

#endif /* ACTIONTRACKDELETEMIDINOTE_H_ */
