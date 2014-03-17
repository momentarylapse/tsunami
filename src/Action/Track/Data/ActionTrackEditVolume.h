/*
 * ActionTrackEditVolume.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKEDITVOLUME_H_
#define ACTIONTRACKEDITVOLUME_H_

#include "../../ActionMergable.h"
class Track;

class ActionTrackEditVolume : public ActionMergable<float>
{
public:
	ActionTrackEditVolume(Track *t, float volume);
	virtual ~ActionTrackEditVolume();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

	virtual bool mergable(Action *a);

private:
	int track_no;
};

#endif /* ACTIONTRACKEDITVOLUME_H_ */
