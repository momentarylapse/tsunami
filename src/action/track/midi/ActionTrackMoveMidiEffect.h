/*
 * ActionTrackMoveMidiEffect.h
 *
 *  Created on: 26.05.2019
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>

namespace tsunami {

class AudioEffect;
struct Track;

class ActionTrackMoveMidiEffect: public history::Action {
public:
	ActionTrackMoveMidiEffect(Track *track, int source, int target);

	string name() const override { return ":##:move midi fx"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	Track *track;
	int source;
	int target;
};

}
