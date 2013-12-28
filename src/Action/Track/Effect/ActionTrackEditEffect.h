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
class EffectParam;

class ActionTrackEditEffect: public Action
{
public:
	ActionTrackEditEffect(Track *t, int index, Array<EffectParam> &params);
	virtual ~ActionTrackEditEffect();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	Array<EffectParam> params;
	int track_no;
	int index;
	bool first_execution;
};

#endif /* ACTIONTRACKEDITEFFECT_H_ */
