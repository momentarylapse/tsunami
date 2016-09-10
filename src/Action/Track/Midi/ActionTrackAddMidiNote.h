/*
 * ActionTrackAddMidiNote.h
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#ifndef ACTIONTRACKADDMIDINOTE_H_
#define ACTIONTRACKADDMIDINOTE_H_

#include "../../Action.h"

class Track;
class MidiNote;

class ActionTrackAddMidiNote : public Action
{
public:
	ActionTrackAddMidiNote(Track *t, const MidiNote &n);
	~ActionTrackAddMidiNote();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int track_no;
	MidiNote *note;
	int insert_index;
};

#endif /* ACTIONTRACKADDMIDINOTE_H_ */
