/*
 * ActionTrackEditMuted.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKEDITMUTED_H_
#define ACTIONTRACKEDITMUTED_H_

#include "../Action.h"
class Track;

class ActionTrackEditMuted : public Action
{
public:
	ActionTrackEditMuted(Track *t, bool muted);
	virtual ~ActionTrackEditMuted();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	bool muted;
	int track_no;
};

#endif /* ACTIONTRACKEDITMUTED_H_ */
