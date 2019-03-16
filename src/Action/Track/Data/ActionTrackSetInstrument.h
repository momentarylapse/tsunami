/*
 * ActionTrackSetInstrument.h
 *
 *  Created on: Dec 2, 2015
 *      Author: ankele
 */

#ifndef SRC_ACTION_TRACK_DATA_ACTIONTRACKSETINSTRUMENT_H_
#define SRC_ACTION_TRACK_DATA_ACTIONTRACKSETINSTRUMENT_H_

#include "../../ActionMergable.h"
#include "../../../Data/Midi/Instrument.h"
class Track;
class MidiNote;

class ActionTrackSetInstrument: public ActionMergable<Instrument>
{
public:
	ActionTrackSetInstrument(Track *t, const Instrument &instrument);
	virtual ~ActionTrackSetInstrument(){}

	void *execute(Data *d) override;
	void undo(Data *d) override;

	bool mergable(Action *a) override;
	bool absorb(ActionMergableBase *a) override;

private:
	Track *track;

	struct StringChange
	{
		MidiNote* note;
		int from, to;
	};
	Array<StringChange> string_change;
};

#endif /* SRC_ACTION_TRACK_DATA_ACTIONTRACKSETINSTRUMENT_H_ */
