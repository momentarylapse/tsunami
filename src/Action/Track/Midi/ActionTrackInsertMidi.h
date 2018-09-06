/*
 * ActionTrackInsertMidi.h
 *
 *  Created on: 23.02.2013
 *      Author: michi
 */

#ifndef ACTIONTRACKINSERTMIDI_H_
#define ACTIONTRACKINSERTMIDI_H_

#include "../../Action.h"
#include "../../../Data/Midi/MidiData.h"

class TrackLayer;

class ActionTrackInsertMidi : public Action
{
public:
	ActionTrackInsertMidi(TrackLayer *l, int offset, const MidiNoteBuffer &midi);
	virtual ~ActionTrackInsertMidi();

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	TrackLayer *layer;
	MidiNoteBuffer midi;
	int offset;
	Array<int> inserted_at;
	bool applied;
};

#endif /* ACTIONTRACKINSERTMIDI_H_ */
