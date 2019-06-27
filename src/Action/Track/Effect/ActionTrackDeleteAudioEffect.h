/*
 * ActionTrackDeleteEffect.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKDELETEEFFECT_H_
#define ACTIONTRACKDELETEEFFECT_H_

#include "../../Action.h"
class Track;
class AudioEffect;

class ActionTrackDeleteEffect: public Action {
public:
	ActionTrackDeleteEffect(Track *t, int index);
	~ActionTrackDeleteEffect();

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	AudioEffect *effect;
	Track *track;
	int index;
};

#endif /* ACTIONTRACKDELETEEFFECT_H_ */
