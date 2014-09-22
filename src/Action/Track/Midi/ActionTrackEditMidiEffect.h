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
class AudioFile;

class ActionTrackEditMidiEffect: public ActionMergable<string>
{
public:
	ActionTrackEditMidiEffect(Track *t, int index, const string &old_params, MidiEffect *fx);
	virtual ~ActionTrackEditMidiEffect();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

	virtual bool mergable(Action *a);

private:
	int track_no;
	int index;
};

#endif /* ACTIONTRACKEDITMIDIEFFECT_H_ */
