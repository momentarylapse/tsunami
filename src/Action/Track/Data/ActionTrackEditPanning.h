/*
 * ActionTrackEditPanning.h
 *
 *  Created on: 13.03.2013
 *      Author: michi
 */

#ifndef ACTIONTRACKEDITPANNING_H_
#define ACTIONTRACKEDITPANNING_H_

#include "../../ActionMergable.h"
class Track;

class ActionTrackEditPanning : public ActionMergable<float>
{
public:
	ActionTrackEditPanning(Track *t, float panning);
	virtual ~ActionTrackEditPanning();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

	virtual bool mergable(Action *a);

private:
	int track_no;
};

#endif /* ACTIONTRACKEDITPANNING_H_ */
