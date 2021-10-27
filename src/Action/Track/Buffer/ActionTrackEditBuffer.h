/*
 * ActionTrackEditBuffer.h
 *
 *  Created on: 30.03.2012
 *      Author: michi
 */

#pragma once

#include "../../Action.h"
#include "../../../Data/Audio/AudioBuffer.h"

class TrackLayer;

class ActionTrackEditBuffer : public Action {
public:
	ActionTrackEditBuffer(TrackLayer *l, const Range &_range);

	string name() const override { return ":##:edit buffer"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;
	void redo(Data *d) override;

private:
	TrackLayer *layer;
	Range range;
	AudioBuffer box;
	int index;
};
