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

class ActionTrackSetInstrument: public ActionMergable<Instrument>
{
public:
	ActionTrackSetInstrument(Track *t, const Instrument &instrument);
	virtual ~ActionTrackSetInstrument(){}

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

	virtual bool mergable(Action *a);
	virtual bool absorb(ActionMergableBase *a);

private:
	int track_no;
};

#endif /* SRC_ACTION_TRACK_DATA_ACTIONTRACKSETINSTRUMENT_H_ */
