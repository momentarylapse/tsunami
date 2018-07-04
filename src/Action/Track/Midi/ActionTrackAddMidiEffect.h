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
	~ActionTrackAddMidiEffect();

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	MidiEffect *effect;
	Track *track;
};

#endif /* ACTIONTRACKADDMIDIEFFECT_H_ */
