/*
 * ActionTrackAddMidiNote.h
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#ifndef ACTIONTRACKADDMIDINOTE_H_
#define ACTIONTRACKADDMIDINOTE_H_

#include "../../Action.h"

class TrackLayer;
class MidiNote;

class ActionTrackAddMidiNote : public Action
{
public:
	ActionTrackAddMidiNote(TrackLayer *l, MidiNote *n);
	~ActionTrackAddMidiNote();

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	TrackLayer *layer;
	MidiNote *note;
	int insert_index;
};

#endif /* ACTIONTRACKADDMIDINOTE_H_ */
