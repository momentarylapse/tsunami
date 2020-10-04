/*
 * ActionTrackAddMidiEffect.h
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_MIDI_ACTIONTRACKADDMIDIEFFECT_H_
#define SRC_ACTION_TRACK_MIDI_ACTIONTRACKADDMIDIEFFECT_H_

#include "../../Action.h"
class Track;
class MidiEffect;

class ActionTrackAddMidiEffect: public Action {
public:
	ActionTrackAddMidiEffect(Track *t, MidiEffect *effect);

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	shared<MidiEffect> effect;
	Track *track;
};

#endif /* SRC_ACTION_TRACK_MIDI_ACTIONTRACKADDMIDIEFFECT_H_ */
