/*
 * ActionTrackAddMidiEffect.h
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#ifndef ACTIONTRACKADDMIDIEFFECT_H_
#define ACTIONTRACKADDMIDIEFFECT_H_

#include "../../Action.h"
class Track;
class MidiEffect;

class ActionTrackAddMidiEffect: public Action
{
public:
	ActionTrackAddMidiEffect(Track *t, MidiEffect *effect);
	virtual ~ActionTrackAddMidiEffect();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	MidiEffect *effect;
	int track_no;
};

#endif /* ACTIONTRACKADDMIDIEFFECT_H_ */
