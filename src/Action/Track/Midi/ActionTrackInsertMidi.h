/*
 * ActionTrackInsertMidi.h
 *
 *  Created on: 23.02.2013
 *      Author: michi
 */

#ifndef ACTIONTRACKINSERTMIDI_H_
#define ACTIONTRACKINSERTMIDI_H_

#include "../../Action.h"
#include "../../../Data/Track.h"

class ActionTrackInsertMidi : public Action
{
public:
	ActionTrackInsertMidi(Track *t, int offset, MidiData &midi);
	virtual ~ActionTrackInsertMidi();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int track_no;
	MidiData midi;
	int offset;
	Array<int> inserted_at;
};

#endif /* ACTIONTRACKINSERTMIDI_H_ */
