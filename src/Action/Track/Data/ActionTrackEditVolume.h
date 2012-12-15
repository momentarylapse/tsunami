/*
 * ActionTrackEditVolume.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKEDITVOLUME_H_
#define ACTIONTRACKEDITVOLUME_H_

#include "../../Action.h"
class Track;

class ActionTrackEditVolume : public Action
{
public:
	ActionTrackEditVolume(Track *t, float volume);
	virtual ~ActionTrackEditVolume();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	float volume;
	int track_no;
};

#endif /* ACTIONTRACKEDITVOLUME_H_ */
