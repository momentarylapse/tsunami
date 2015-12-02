/*
 * ActionTrackSetInstrument.h
 *
 *  Created on: Dec 2, 2015
 *      Author: ankele
 */

#ifndef SRC_ACTION_TRACK_DATA_ACTIONTRACKSETINSTRUMENT_H_
#define SRC_ACTION_TRACK_DATA_ACTIONTRACKSETINSTRUMENT_H_

#include "../../Action.h"
class Track;

class ActionTrackSetInstrument: public Action
{
public:
	ActionTrackSetInstrument(Track *t, const string &instrument, const Array<int> &tuning);
	virtual ~ActionTrackSetInstrument(){}

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int track_no;
	string instrument;
	Array<int> tuning;
};

#endif /* SRC_ACTION_TRACK_DATA_ACTIONTRACKSETINSTRUMENT_H_ */
