/*
 * ActionTrackAddMidiEvent.h
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#ifndef ACTIONTRACKADDMIDIEVENT_H_
#define ACTIONTRACKADDMIDIEVENT_H_

#include "../../Action.h"
#include "../../../Data/Track.h"

class ActionTrackAddMidiEvent : public Action
{
public:
	ActionTrackAddMidiEvent(Track *t, const MidiEvent &e);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int track_no;
	MidiEvent event;
	int insert_index;
};

#endif /* ACTIONTRACKADDMIDIEVENT_H_ */
