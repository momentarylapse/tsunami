/*
 * ActionTrackEditMuted.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_DATA_ACTIONTRACKEDITMUTED_H_
#define SRC_ACTION_TRACK_DATA_ACTIONTRACKEDITMUTED_H_

#include "../../Action.h"
class Track;

class ActionTrackEditMuted : public Action {
public:
	ActionTrackEditMuted(Track *t, bool muted);

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	bool muted;
	Track *track;
};

#endif /* SRC_ACTION_TRACK_DATA_ACTIONTRACKEDITMUTED_H_ */
