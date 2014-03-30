/*
 * ActionTrackToggleEffectEnabled.h
 *
 *  Created on: 30.03.2014
 *      Author: michi
 */

#ifndef ACTIONTRACKTOGGLEEFFECTENABLED_H_
#define ACTIONTRACKTOGGLEEFFECTENABLED_H_

#include "../../Action.h"
class Track;
class Effect;

class ActionTrackToggleEffectEnabled: public Action
{
public:
	ActionTrackToggleEffectEnabled(Track *t, int index);
	virtual ~ActionTrackToggleEffectEnabled();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int track_no;
	int index;
};

#endif /* ACTIONTRACKTOGGLEEFFECTENABLED_H_ */
