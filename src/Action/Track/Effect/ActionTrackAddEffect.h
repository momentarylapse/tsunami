/*
 * ActionTrackAddEffect.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKADDEFFECT_H_
#define ACTIONTRACKADDEFFECT_H_

#include "../../Action.h"
class Track;
class Effect;

class ActionTrackAddEffect: public Action
{
public:
	ActionTrackAddEffect(Track *t, Effect *effect);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	Effect *effect;
	int track_no;
};

#endif /* ACTIONTRACKADDEFFECT_H_ */
