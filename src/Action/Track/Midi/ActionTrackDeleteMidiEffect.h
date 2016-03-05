/*
 * ActionTrackDeleteMidiEffect.h
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#ifndef ACTIONTRACKDELETEMIDIEFFECT_H_
#define ACTIONTRACKDELETEMIDIEFFECT_H_

#include "../../Action.h"
#include "../../../Plugins/MidiEffect.h"
class Track;

class ActionTrackDeleteMidiEffect: public Action
{
public:
	ActionTrackDeleteMidiEffect(Track *t, int index);

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	MidiEffect *effect;
	int track_no;
	int index;
};

#endif /* ACTIONTRACKDELETEEFFECT_H_ */
