/*
 * ActionTrackMoveAudioEffect.h
 *
 *  Created on: 20.11.2018
 *      Author: michi
 */

#pragma once

#include "../../Action.h"
class AudioEffect;
class Track;

class ActionTrackMoveAudioEffect: public Action {
public:
	ActionTrackMoveAudioEffect(Track *track, int source, int target);

	string name() const override { return ":##:move fx"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	Track *track;
	int source;
	int target;
};
