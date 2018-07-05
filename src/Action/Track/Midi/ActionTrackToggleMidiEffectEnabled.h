/*
 * ActionTrackToggleMidiEffectEnabled.h
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#ifndef ACTIONTRACKTOGGLEMIDIEFFECTENABLED_H_
#define ACTIONTRACKTOGGLEMIDIEFFECTENABLED_H_

#include "../../Action.h"
class MidiEffect;

class ActionTrackToggleMidiEffectEnabled: public Action
{
public:
	ActionTrackToggleMidiEffectEnabled(MidiEffect *fx);

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	MidiEffect *fx;
};

#endif /* ACTIONTRACKTOGGLEMIDIEFFECTENABLED_H_ */
