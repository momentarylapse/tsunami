/*
 * ActionTrackMoveAudioEffect.h
 *
 *  Created on: 20.11.2018
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_EFFECT_ACTIONTRACKMOVEAUDIOEFFECT_H_
#define SRC_ACTION_TRACK_EFFECT_ACTIONTRACKMOVEAUDIOEFFECT_H_

#include "../../Action.h"
class AudioEffect;
class Track;

class ActionTrackMoveAudioEffect: public Action {
public:
	ActionTrackMoveAudioEffect(Track *track, int source, int target);

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	Track *track;
	int source;
	int target;
};

#endif /* SRC_ACTION_TRACK_EFFECT_ACTIONTRACKMOVEAUDIOEFFECT_H_ */
