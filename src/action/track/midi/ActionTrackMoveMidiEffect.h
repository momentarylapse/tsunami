/*
 * ActionTrackMoveMidiEffect.h
 *
 *  Created on: 26.05.2019
 *      Author: michi
 */

#pragma once

#include "../../Action.h"
class AudioEffect;
class Track;

class ActionTrackMoveMidiEffect: public Action {
public:
	ActionTrackMoveMidiEffect(Track *track, int source, int target);

	string name() const override { return ":##:move midi fx"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	Track *track;
	int source;
	int target;
};
