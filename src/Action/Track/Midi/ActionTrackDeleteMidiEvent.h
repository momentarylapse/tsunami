/*
 * ActionTrackDeleteMidiEvent.h
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#ifndef ACTIONTRACKDELETEMIDIEVENT_H_
#define ACTIONTRACKDELETEMIDIEVENT_H_

#include "../../Action.h"
#include "../../../Data/Track.h"

class ActionTrackDeleteMidiEvent : public Action
{
public:
	ActionTrackDeleteMidiEvent(Track *t, int index);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int track_no;
	MidiEvent event;
	int index;
};

#endif /* ACTIONTRACKDELETEMIDIEVENT_H_ */
