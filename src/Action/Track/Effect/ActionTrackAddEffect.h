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

class ActionTrackAddEffect: public Action {
public:
	ActionTrackAddEffect(Track *t, AudioEffect *effect);
	~ActionTrackAddEffect();

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	AudioEffect *effect;
	Track *track;
};

#endif /* ACTIONTRACKADDEFFECT_H_ */
