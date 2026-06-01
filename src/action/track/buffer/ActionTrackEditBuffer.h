/*
 * ActionTrackEditBuffer.h
 *
 *  Created on: 30.03.2012
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>
#include "../../../data/audio/AudioBuffer.h"

namespace tsunami {

class TrackLayer;

class ActionTrackEditBuffer : public history::Action {
public:
	ActionTrackEditBuffer(TrackLayer *l, const Range &_range);

	string name() const override { return ":##:edit buffer"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;
	void redo(history::Data *d) override;

private:
	TrackLayer *layer;
	Range range;
	AudioBuffer box;
	int index;
};

}
