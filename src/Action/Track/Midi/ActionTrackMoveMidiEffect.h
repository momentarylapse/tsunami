/*
 * ActionTrackMoveMidiEffect.h
 *
 *  Created on: 26.05.2019
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_MIDI_ACTIONTRACKMOVEMIDIEFFECT_H_
#define SRC_ACTION_TRACK_MIDI_ACTIONTRACKMOVEMIDIEFFECT_H_

#include "../../Action.h"
class AudioEffect;
class Track;

class ActionTrackMoveMidiEffect: public Action
{
public:
	ActionTrackMoveMidiEffect(Track *track, int source, int target);

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	Track *track;
	int source;
	int target;
};

#endif /* SRC_ACTION_TRACK_MIDI_ACTIONTRACKMOVEMIDIEFFECT_H_ */
