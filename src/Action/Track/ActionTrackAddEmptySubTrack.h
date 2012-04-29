/*
 * ActionTrackAddEmptySubTrack.h
 *
 *  Created on: 25.03.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKADDEMPTYSUBTRACK_H_
#define ACTIONTRACKADDEMPTYSUBTRACK_H_

#include "../Action.h"
#include "../../Data/Range.h"

class ActionTrackAddEmptySubTrack : public Action
{
public:
	ActionTrackAddEmptySubTrack(int _track_no, const Range &_range, const string &_name);
	virtual ~ActionTrackAddEmptySubTrack();

	void *execute(Data *d);
	void undo(Data *d);

private:
	int track_no;
	int index;
	Range range;
	string name;
};

#endif /* ACTIONTRACKADDEMPTYSUBTRACK_H_ */
