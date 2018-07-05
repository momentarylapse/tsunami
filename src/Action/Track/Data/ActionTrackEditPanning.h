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

	void *execute(Data *d) override;
	void undo(Data *d) override;

	bool mergable(Action *a) override;

private:
	Track *track;
};

#endif /* ACTIONTRACKEDITPANNING_H_ */
