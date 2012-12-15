/*
 * ActionTrackEditEffect.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKEDITEFFECT_H_
#define ACTIONTRACKEDITEFFECT_H_

#include "../../Action.h"
#include "../../../Plugins/Effect.h"
class Track;

class ActionTrackEditEffect: public Action
{
public:
	ActionTrackEditEffect(Track *t, int index, Effect &effect);
	virtual ~ActionTrackEditEffect();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	Effect effect;
	int track_no;
	int index;
};

#endif /* ACTIONTRACKEDITEFFECT_H_ */
