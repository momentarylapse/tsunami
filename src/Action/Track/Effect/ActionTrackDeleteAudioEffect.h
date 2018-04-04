/*
 * ActionTrackDeleteEffect.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKDELETEEFFECT_H_
#define ACTIONTRACKDELETEEFFECT_H_

#include "../../../Module/Audio/AudioEffect.h"
#include "../../Action.h"
class Track;

class ActionTrackDeleteEffect: public Action
{
public:
	ActionTrackDeleteEffect(Track *t, int index);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	AudioEffect *effect;
	int track_no;
	int index;
};

#endif /* ACTIONTRACKDELETEEFFECT_H_ */
