/*
 * ActionTrackEditMidiEffect.h
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#ifndef ACTIONTRACKEDITMIDIEFFECT_H_
#define ACTIONTRACKEDITMIDIEFFECT_H_

#include "../../ActionMergable.h"
class Track;
class MidiEffect;

class ActionTrackEditMidiEffect: public ActionMergable<string>
{
public:
	ActionTrackEditMidiEffect(MidiEffect *fx, const string &old_params);

	void *execute(Data *d) override;
	void undo(Data *d) override;

	bool mergable(Action *a) override;

private:
	MidiEffect *fx;
};

#endif /* ACTIONTRACKEDITMIDIEFFECT_H_ */
