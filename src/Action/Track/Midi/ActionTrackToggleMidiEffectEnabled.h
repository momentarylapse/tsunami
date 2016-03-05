/*
 * ActionTrackToggleMidiEffectEnabled.h
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#ifndef ACTIONTRACKTOGGLEMIDIEFFECTENABLED_H_
#define ACTIONTRACKTOGGLEMIDIEFFECTENABLED_H_

#include "../../Action.h"
class Track;
class MidiEffect;

class ActionTrackToggleMidiEffectEnabled: public Action
{
public:
	ActionTrackToggleMidiEffectEnabled(Track *t, int index);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int track_no;
	int index;
};

#endif /* ACTIONTRACKTOGGLEMIDIEFFECTENABLED_H_ */
