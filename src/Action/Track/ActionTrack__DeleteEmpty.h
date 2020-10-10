/*
 * ActionTrack__DeleteEmpty.h
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_ACTIONTRACK__DELETEEMPTY_H_
#define SRC_ACTION_TRACK_ACTIONTRACK__DELETEEMPTY_H_

#include "../Action.h"

class Track;

class ActionTrack__DeleteEmpty: public Action {
public:
	ActionTrack__DeleteEmpty(shared<Track> track);

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int index;
	shared<Track> track;
};

#endif /* SRC_ACTION_TRACK_ACTIONTRACK__DELETEEMPTY_H_ */
