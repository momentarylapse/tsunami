/*
 * ActionTrackAddEmptySubTrack.h
 *
 *  Created on: 25.03.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKADDEMPTYSUBTRACK_H_
#define ACTIONTRACKADDEMPTYSUBTRACK_H_

#include "Action.h"

class ActionTrackAddEmptySubTrack : public Action
{
public:
	ActionTrackAddEmptySubTrack(int _track_no, int _pos, int _length, const string &_name);
	virtual ~ActionTrackAddEmptySubTrack();

	void *execute(Data *d);
	void undo(Data *d);

private:
	int track_no;
	int pos, length, index;
	string name;
};

#endif /* ACTIONTRACKADDEMPTYSUBTRACK_H_ */
