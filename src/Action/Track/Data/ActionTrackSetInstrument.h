/*
 * ActionTrackSetInstrument.h
 *
 *  Created on: Dec 2, 2015
 *      Author: ankele
 */

#ifndef SRC_ACTION_TRACK_DATA_ACTIONTRACKSETINSTRUMENT_H_
#define SRC_ACTION_TRACK_DATA_ACTIONTRACKSETINSTRUMENT_H_

#include "../../ActionMergable.h"
class Track;
class Instrument;

class ActionTrackSetInstrument: public ActionMergable<int>
{
public:
	ActionTrackSetInstrument(Track *t, const Instrument &instrument, const Array<int> &tuning);
	virtual ~ActionTrackSetInstrument(){}

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

	virtual bool mergable(Action *a);
	virtual bool absorb(ActionMergableBase *a);

private:
	int track_no;
	Array<int> new_tuning;
	Array<int> old_tuning;
};

#endif /* SRC_ACTION_TRACK_DATA_ACTIONTRACKSETINSTRUMENT_H_ */
