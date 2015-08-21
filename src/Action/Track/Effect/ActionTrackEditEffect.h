/*
 * ActionTrackEditEffect.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKEDITEFFECT_H_
#define ACTIONTRACKEDITEFFECT_H_

#include "../../ActionMergable.h"
class Track;
class Effect;
class Song;

class ActionTrackEditEffect: public ActionMergable<string>
{
public:
	ActionTrackEditEffect(Track *t, int index, const string &old_params, Effect *fx);
	virtual ~ActionTrackEditEffect();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

	virtual bool mergable(Action *a);

private:
	int track_no;
	int index;
};

#endif /* ACTIONTRACKEDITEFFECT_H_ */
