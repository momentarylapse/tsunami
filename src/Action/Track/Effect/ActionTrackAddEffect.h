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
class AudioEffect;

class ActionTrackAddEffect: public Action
{
public:
	ActionTrackAddEffect(Track *t, AudioEffect *effect);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	AudioEffect *effect;
	int track_no;
};

#endif /* ACTIONTRACKADDEFFECT_H_ */
